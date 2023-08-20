#include <Arduino.h>
#include "Log.h"

config_t config[] =
{
    {"TEST",    "\033[1;35m"},
    {"DEBUG",   "\033[1;36m"},
    {"INFO",    "\033[1;32m"},
    {"WARNING", "\033[1;33m"},
    {"ERROR",   "\033[1;31m"},
};

Log::Log(HardwareSerial *serial) {
  _serial = serial;
  _serial->begin(115200);
}

void Log::test(const char *tag, const char *format, ...) {
  va_list args;

  va_start(args, format);
  printLog("TEST", tag, format, args);
  va_end(args);
}

void Log::d(const char *tag, const char *format, ...) {
  va_list args;

  va_start(args, format);
  printLog("DEBUG", tag, format, args);
  va_end(args);
}

void Log::i(const char *tag, const char *format, ...) {
  va_list args;

  va_start(args, format);
  printLog("INFO", tag, format, args);
  va_end(args);
}

void Log::w(const char *tag, const char *format, ...) {
  va_list args;

  va_start(args, format);
  printLog("WARNING", tag, format, args);
  va_end(args);
}

void Log::e(const char *tag, const char *format, ...) {
  va_list args;

  va_start(args, format);
  printLog("ERROR", tag, format, args);
  va_end(args);
}

void Log::printLog(const char *level, const char *tag, const char *format, va_list args) {
  // Get the color associated with the level
  const char *color = getColor(level);

  // Print color, level, tag, and message
  _serial->printf("%s[%s] [%s] ", color, level, tag);
  _serial->vprintf(format, args);
  _serial->println();

  // Reset color
  _serial->printf("\033[0m");
}

const char* Log::getColor(const char *level) {
  size_t index;

  // Find the corresponding color based on the level
  for (index = 0; index < sizeof(config) / sizeof(config[0]); index++) {
    if (strcmp(config[index].level, level) == 0) {
      return config[index].color;
    }
  }

  // Reset color by default
  return "\033[0m";
}