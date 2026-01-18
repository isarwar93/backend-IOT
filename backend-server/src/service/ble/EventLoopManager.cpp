// File: src/service/ble/EventLoopManager.cpp
#include "EventLoopManager.hpp"
#include "config/LogAdapt.hpp"

EventLoopManager::~EventLoopManager() {
    stopAllLoops();
}

bool EventLoopManager::startLoopFor(const std::string& key) {
    // Prepare GLib objects first
    GMainContext* ctx = g_main_context_new();
    if (!ctx) return false;

    GMainLoop* loop = g_main_loop_new(ctx, FALSE);
    if (!loop) {
        g_main_context_unref(ctx);
        return false;
    }

    // Start thread now so we know it's good before inserting
    std::thread th;
    try {
        LOGI("Starting event loop for key: {}", key);
        th = std::thread([ctx, loop, key]() {
            g_main_context_push_thread_default(ctx);
            LOGI("Running GMainLoop for key: {}", key);
            g_main_loop_run(loop);
            g_main_context_pop_thread_default(ctx);
            LOGI("GMainLoop stopped for key: {}", key);
        });
    } catch (...) {
        g_main_context_unref(ctx);
        return false; // If thread creation fails, clean up and return false
    }

    // Try to publish into the map (move the thread in)
    {
        std::lock_guard<std::mutex> lk(m_mx);
        auto [it, inserted] = m_loops.try_emplace(
            key, LoopWorker{ctx, loop, std::move(th)}
        );
        if (!inserted) {
            // Key already exists → roll back the new worker
            g_main_loop_quit(loop);
            if (th.joinable()) th.join();
            g_main_loop_unref(loop);
            g_main_context_unref(ctx);
            LOGW("Event loop for key '{}' already exists", key);
            return false;
        }
    }
    LOGI("Event loop started successfully for key: {}", key);
    return true;
}

void EventLoopManager::stopLoopFor(const std::string& key) {
    LOGI("Stopping event loop for key: {}", key);
    LoopWorker toStop; // take ownership out of the map to stop outside the lock
    {
        std::lock_guard<std::mutex> lk(m_mx);
        auto it = m_loops.find(key);
        if (it == m_loops.end()) {
            LOGW("Event loop for key '{}' not found", key);
            return;
        }

        // Signal loop to quit
        if (it->second.loop) g_main_loop_quit(it->second.loop);

        // Move the worker out so we can join without holding the lock
        toStop = std::move(it->second);
        m_loops.erase(it);
    }
    LOGI("Event loop for key '{}' is signaled to stop", key);
    
    // Join the thread
    if (toStop.th.joinable()) toStop.th.join();
    LOGI("Event loop for key '{}' has stopped", key);
    
    // Free GLib resources
    destroyWorker(toStop);
}

void EventLoopManager::stopAllLoops() {
    LOGI("Stopping all event loops");
    // Move everything out to join without holding the lock
    std::unordered_map<std::string, LoopWorker> toStopAll;
    {
        std::lock_guard<std::mutex> lk(m_mx);
        for (auto& [k, w] : m_loops) {
            if (w.loop) g_main_loop_quit(w.loop);
        }
        toStopAll = std::move(m_loops);
        m_loops.clear();
    }

    // Join and free
    for (auto& [k, w] : toStopAll) {
        if (w.th.joinable()) w.th.join();
        destroyWorker(w);
    }
    LOGI("All event loops have stopped");
}

bool EventLoopManager::hasLoop(const std::string& key) const {
    std::lock_guard<std::mutex> lk(m_mx);
    return m_loops.find(key) != m_loops.end();
}

size_t EventLoopManager::loopCount() const {
    std::lock_guard<std::mutex> lk(m_mx);
    return m_loops.size();
}

void EventLoopManager::destroyWorker(LoopWorker& w) {
    // Called after thread is already joined or never started.
    if (w.loop)  { g_main_loop_unref(w.loop);   w.loop = nullptr; }
    if (w.ctx)   { g_main_context_unref(w.ctx); w.ctx  = nullptr; }
}
