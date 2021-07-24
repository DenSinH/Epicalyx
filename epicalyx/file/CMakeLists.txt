add_library(File STATIC
        File.h File.cpp SString.h SString.cpp)

set_target_properties(File PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(File PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})