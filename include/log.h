#include "spdlog/async.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/spdlog.h"

class Logger {
public:
  Logger(const Logger &) = delete;
  Logger(Logger &&) = delete;
  Logger &operator=(const Logger &) = delete;
  Logger &operator=(Logger &&) = delete;
  inline static std::shared_ptr<spdlog::logger> async_log =
      spdlog::basic_logger_mt<spdlog::async_factory>("async_file_logger",
                                                     "logs/log");
  ;
};

