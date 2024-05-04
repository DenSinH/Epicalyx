#include "Preprocessor.h"
#include "Escape.h"

#include <filesystem>
#include <ctime>
#include <cctype>


namespace epi {

const cotyl::unordered_map<std::string, std::string (Preprocessor::*)() const> Preprocessor::StandardDefinitions = {
  {"__FILE__", &Preprocessor::FILE}, 
  {"__LINE__", &Preprocessor::LINE}, 
  {"__DATE__", &Preprocessor::DATE},
  {"__TIME__", &Preprocessor::TIME},
  {"__STDC__", &Preprocessor::STDC},
  {"__STDC_HOSTED__", &Preprocessor::STDC_HOSTED},
  {"__STDC_VERSION__", &Preprocessor::STDC_VERSION},
};

std::string Preprocessor::FILE() const {
  auto full_path = std::filesystem::canonical(file_stack.back().name).string();
  auto escaped = cotyl::Escape(full_path.c_str());
  return cotyl::Format("\"%s\"", escaped.c_str());
}

std::string Preprocessor::LINE() const {
  return std::to_string(file_stack.back().line);
}

// localtime is deprecated
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
std::string Preprocessor::DATE() const {
  std::string result = "??? ?? ????";
  auto datetime = std::time({});
  std::strftime(result.data(), result.size(), "%b %d %Y", std::localtime(&datetime));
  return cotyl::Format("\"%s\"", result.c_str());
}

std::string Preprocessor::TIME() const {
  std::string result = "??:??:??";
  auto datetime = std::time({});
  std::strftime(result.data(), result.size(), "%H:%M:%S", std::localtime(&datetime));
  return cotyl::Format("\"%s\"", result.c_str());
}
#pragma GCC diagnostic pop

std::string Preprocessor::STDC() const {
  // In normal operation, this macro expands to the constant 1,
  // to signify that this compiler conforms to ISO Standard C. 
  return "1";
}

std::string Preprocessor::STDC_HOSTED() const {
  // This macro is defined, with value 1, if the compiler’s target 
  // is a hosted environment. A hosted environment has the complete 
  // facilities of the standard C library available.

  // we don't, but we'll just say we do
  return "1";
}

std::string Preprocessor::STDC_VERSION() const {
  // This macro expands to the C Standard’s version number, 
  // a long integer constant of the form yyyymmL where yyyy 
  // and mm are the year and month of the Standard version.
  // ...
  // the value 201112L signifies the 2011 revision of the 
  // C standard
  return "201112L";
}

}