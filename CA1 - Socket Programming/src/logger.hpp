#ifndef LOGGER_HPP_INCLUDE
#define LOGGER_HPP_INCLUDE

#include <fstream>
#include <ostream>
#include <unordered_map>

class Logger {
public:
    enum class Level {
        Info,
        Warning,
        Error
    };

    Logger(Level level);
    Logger(Level level, std::ofstream& file);

    void info(const std::string& message,
              const std::string& messageCode = "",
              const std::string& action = "",
              const std::unordered_map<std::string, std::string>& details = {});
    void warn(const std::string& message,
              const std::string& messageCode = "",
              const std::string& action = "",
              const std::unordered_map<std::string, std::string>& details = {});
    void error(const std::string& message,
               const std::string& messageCode = "",
               const std::string& action = "",
               const std::unordered_map<std::string, std::string>& details = {});

private:
    Level level_;
    std::ostream& stream_;
    bool isaTTY_;

    void log(Level level,
             const std::string& message,
             const std::string& messageCode,
             const std::string& action,
             const std::unordered_map<std::string, std::string>& details);
    void logJson(Level level,
                 const std::string& message,
                 const std::string& messageCode,
                 const std::string& action,
                 const std::unordered_map<std::string, std::string>& details);
    std::string levelToString(Level level, bool isJson = false);
};

#endif // LOGGER_HPP_INCLUDE
