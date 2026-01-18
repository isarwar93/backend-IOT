#ifndef EVENT_LOOP_MANAGER_HPP
#define EVENT_LOOP_MANAGER_HPP

#include <string>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <gio/gio.h>

/// Manages GMainLoop workers for handling asynchronous BLE operations.
/// Each worker runs a GMainLoop on its own thread.
class EventLoopManager {
public:
    EventLoopManager() = default;
    ~EventLoopManager();

    // Start a dedicated GMainLoop on its own thread for a given key.
    // Returns false if a loop for this key already exists or creation fails.
    bool startLoopFor(const std::string& key);

    // Stop + join + free the loop for this key, if present.
    void stopLoopFor(const std::string& key);

    // Stop all loops (used in shutdown).
    void stopAllLoops();

    // Helper functions
    bool hasLoop(const std::string& key) const;
    size_t loopCount() const;

private:
    struct LoopWorker {
        GMainContext* ctx = nullptr; // created with g_main_context_new(); unref when done
        GMainLoop*    loop = nullptr; // created with g_main_loop_new(ctx, FALSE); unref when done
        std::thread   th;             // runs g_main_loop_run(loop)
    };

    // IMPORTANT: one GMainContext per thread for multiple loops in parallel
    // (the default context should not be run from multiple threads simultaneously).
    std::unordered_map<std::string, LoopWorker> m_loops;
    mutable std::mutex m_mx;

    static void destroyWorker(LoopWorker& w);
};

#endif // EVENT_LOOP_MANAGER_HPP
