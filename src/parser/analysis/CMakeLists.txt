add_library(Analysis STATIC
        visitor.h
        visitor.cpp)

include_directories(
        ${PROJECT_SOURCE_DIR}/include
)

set_target_properties(Analysis PROPERTIES LINKER_LANGUAGE CXX)


target_include_directories(Analysis PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/src/include)