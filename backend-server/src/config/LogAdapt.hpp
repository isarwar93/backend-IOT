#ifndef LOG_ADAPT_HPP
#define LOG_ADAPT_HPP

#include "oatpp/Environment.hpp"
#include "oatpp/base/Log.hpp"


constexpr const char* extract_filename(const char* path) {
  const char* filename = path;
  for (const char* p = path; *p; ++p) {
    if (*p == '/' || *p == '\\') {  // Support both Linux and Windows
      filename = p + 1;
    }
  }
  return filename;
}


// Helper to isolate __VA_ARGS__
#define LOG_IMPL(LEVEL,FMT, ...) \
  oatpp::base::Log::stream( \
    LEVEL, \
    "", \
    "[{}:{} {}] " FMT, \
    extract_filename(__FILE__), __LINE__, __func__, ##__VA_ARGS__)


/**
 * Specific log level macros (INFO, DEBUG, WARN, ERROR)
 */
#define LOGV(FMT, ...) LOG_IMPL(oatpp::Logger::PRIORITY_V, FMT, ##__VA_ARGS__)
#define LOGI(FMT, ...) LOG_IMPL(oatpp::Logger::PRIORITY_I, FMT, ##__VA_ARGS__)
#define LOGD(FMT, ...) LOG_IMPL(oatpp::Logger::PRIORITY_D, FMT, ##__VA_ARGS__)
#define LOGW(FMT, ...) LOG_IMPL(oatpp::Logger::PRIORITY_W, FMT, ##__VA_ARGS__)
#define LOGE(FMT, ...) LOG_IMPL(oatpp::Logger::PRIORITY_E, FMT, ##__VA_ARGS__)


#endif // LOG_ADAPT_HPP