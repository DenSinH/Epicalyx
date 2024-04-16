add_library(Calyx STATIC
        Types.h
        Directive.h
        Directive.cpp
        Calyx.h
        Calyx.cpp)

add_subdirectory(backend)

target_link_libraries(Calyx CalyxBackend)

target_precompile_headers(Calyx PUBLIC 
    ${PROJECT_SOURCE_DIR}/epicalyx/config/PCH.h 
    Types.h Directive.h Calyx.h
)