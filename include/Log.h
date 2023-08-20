#ifndef LOG_H
#define LOG_H

#include <Arduino.h>
#include <stdarg.h>

typedef struct {
  const char *level;
  const char *color;
} config_t;

class Log {
public:
  Log(HardwareSerial *serial);
  void test(const char *tag, const char *format, ...);
  void d(const char *tag, const char *format, ...);
  void i(const char *tag, const char *format, ...);
  void w(const char *tag, const char *format, ...);
  void e(const char *tag, const char *format, ...);

private:
  HardwareSerial *_serial;
  void printLog(const char *level, const char *tag, const char *format, va_list args);
  const char *getColor(const char *level);
};

#endif // LOG_H
