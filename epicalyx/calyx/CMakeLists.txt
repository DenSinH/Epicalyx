add_library(Calyx STATIC
        Calyx.h
        Calyx.cpp)

add_subdirectory(backend)

target_link_libraries(Calyx CalyxBackend)