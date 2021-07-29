#pragma once

#include <cstdlib>
#include <cstdarg>
#include <cstdio>


namespace Log {
enum Color {
  Blue, Green, DarkGreen, Yellow, Red, Pink, White
};

template<Color c = Color::White>
void ConsoleColor();

template<> void ConsoleColor<Color::Blue>();
template<> void ConsoleColor<Color::Green>();
template<> void ConsoleColor<Color::DarkGreen>();
template<> void ConsoleColor<Color::Yellow>();
template<> void ConsoleColor<Color::Red>();
template<> void ConsoleColor<Color::Pink>();
template<> void ConsoleColor<Color::White>();

struct Note {
  Note([[maybe_unused]] const char* format, ...) {
    ConsoleColor<Color::Blue>();
    va_list args;
    va_start(args, format);
    std::printf("[NOTE] ");
    std::vprintf(format, args);
    std::printf("\n");
    va_end(args);
    ConsoleColor();
  }

  template<typename T>
  void operator<<(const T& callable) {
    ConsoleColor<Color::Blue>();
    callable();
    ConsoleColor();
  }
};

struct Warn {
  Warn(const char* format, ...) {
    ConsoleColor<Color::Yellow>();
    va_list args;
    va_start(args, format);
    std::printf("[WARN] ");
    std::vprintf(format, args);
    std::printf("\n");
    va_end(args);
    ConsoleColor();
  }

  template<typename T>
  void operator<<(const T& callable) {
    ConsoleColor<Color::Yellow>();
    callable();
    ConsoleColor();
  }
};

}