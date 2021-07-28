#include "Log.h"

#ifdef _WIN32
#include <Windows.h>

#define FOREGROUND_YELLOW (FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN)
#define FOREGROUND_PINK (FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_BLUE)
#define FOREGROUND_WHITE (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)

#define CONSOLE_BLUE() { SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE | FOREGROUND_INTENSITY); }
#define CONSOLE_GREEN() { SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_INTENSITY); }
#define CONSOLE_DARK_GREEN() { SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN); }
#define CONSOLE_YELLOW() { SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_YELLOW); }
#define CONSOLE_RED() { SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED); }
#define CONSOLE_PINK() { SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_PINK); }
#define CONSOLE_RESTORE() { SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_WHITE); }

#else

// linux / macOS
#define CONSOLE_BLUE() { fprintf(stdout, "\033[0;36m"); }
#define CONSOLE_GREEN() { fprintf(stdout, "\033[0;32m"); }
#define CONSOLE_DARK_GREEN() { fprintf(stdout, "\033[1;32m"); }
#define CONSOLE_YELLOW() { fprintf(stdout, "\033[0;33m"); }
#define CONSOLE_RED() { fprintf(stderr, "\033[1;31m"); }
#define CONSOLE_PINK() { fprintf(stderr, "\033[0;35m"); }
#define CONSOLE_RESTORE() { fprintf(stdout, "\033[0m"); }

#endif

namespace Log {

template<> void ConsoleColor<Color::Blue>() { CONSOLE_BLUE(); }
template<> void ConsoleColor<Color::Green>() { CONSOLE_GREEN(); }
template<> void ConsoleColor<Color::DarkGreen>() { CONSOLE_DARK_GREEN(); }
template<> void ConsoleColor<Color::Yellow>() { CONSOLE_YELLOW(); }
template<> void ConsoleColor<Color::Red>() { CONSOLE_RED(); }
template<> void ConsoleColor<Color::Pink>() { CONSOLE_PINK(); }
template<> void ConsoleColor<Color::White>() { CONSOLE_RESTORE(); }

}