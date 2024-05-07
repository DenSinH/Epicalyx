#include "Preprocessor.h"
#include "Escape.h"
#include "SStream.h"

#include <filesystem>
#include <ctime>
#include <cctype>


namespace epi {

const cotyl::unordered_map<cotyl::CString, cotyl::CString (Preprocessor::*)() const> Preprocessor::StandardDefinitions = {
  {cotyl::CString{"__FILE__"}, &Preprocessor::FILE}, 
  {cotyl::CString{"__LINE__"}, &Preprocessor::LINE}, 
  {cotyl::CString{"__DATE__"}, &Preprocessor::DATE},
  {cotyl::CString{"__TIME__"}, &Preprocessor::TIME},
  {cotyl::CString{"__STDC__"}, &Preprocessor::STDC},
  {cotyl::CString{"__STDC_HOSTED__"}, &Preprocessor::STDC_HOSTED},
  {cotyl::CString{"__STDC_VERSION__"}, &Preprocessor::STDC_VERSION},
};

cotyl::CString Preprocessor::FILE() const {
  auto full_path = std::filesystem::canonical(file_stack.Top().name).string();
  return cotyl::QuotedEscape(full_path.c_str()).cfinalize();
}

cotyl::CString Preprocessor::LINE() const {
  return cotyl::CString{std::to_string(file_stack.Top().line)};
}

// localtime is deprecated
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
cotyl::CString Preprocessor::DATE() const {
  auto result = cotyl::CString{"\"??? ?? ????\""};
  auto datetime = std::time({});
  // account for quotes
  std::strftime(result.c_str() + 1, result.size() - 2, "%b %d %Y", std::localtime(&datetime));
  return std::move(result);
}

cotyl::CString Preprocessor::TIME() const {
  auto result = cotyl::CString{"\"??:??:??\""};
  auto datetime = std::time({});
  // account for quotes
  std::strftime(result.c_str() + 1, result.size() - 2, "%H:%M:%S", std::localtime(&datetime));
  return std::move(result);
}
#pragma GCC diagnostic pop

cotyl::CString Preprocessor::STDC() const {
  // In normal operation, this macro expands to the constant 1,
  // to signify that this compiler conforms to ISO Standard C. 
  return cotyl::CString{'1'};
}

cotyl::CString Preprocessor::STDC_HOSTED() const {
  // This macro is defined, with value 1, if the compiler’s target 
  // is a hosted environment. A hosted environment has the complete 
  // facilities of the standard C library available.

  // we don't, but we'll just say we do
  return cotyl::CString{'1'};
}

cotyl::CString Preprocessor::STDC_VERSION() const {
  // This macro expands to the C Standard’s version number, 
  // a long integer constant of the form yyyymmL where yyyy 
  // and mm are the year and month of the Standard version.
  // ...
  // the value 201112L signifies the 2011 revision of the 
  // C standard
  return cotyl::CString{"201112L"};
}

}