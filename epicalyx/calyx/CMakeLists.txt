add_library(Calyx STATIC
        Types.h
        Directive.h
        Directive.cpp
        Calyx.h
        Calyx.cpp)

add_subdirectory(backend)

target_link_libraries(Calyx CalyxBackend)