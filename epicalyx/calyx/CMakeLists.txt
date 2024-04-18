add_library(CalyxHeaders STATIC
        Types.h Directive.h Calyx.h
        ${PROJECT_SOURCE_DIR}/epicalyx/config/Fake.cpp 
)

target_precompile_headers(CalyxHeaders PUBLIC 
    ${PROJECT_SOURCE_DIR}/epicalyx/config/PCH.h 
    Types.h
    Directive.h
    Calyx.h
)

add_library(Calyx STATIC
        Directive.cpp
        Calyx.cpp
        Utils.h
        Utils.cpp
)

target_precompile_headers(Calyx REUSE_FROM CalyxHeaders)

add_subdirectory(backend)

target_link_libraries(Calyx CalyxHeaders)
target_link_libraries(Calyx CalyxBackend)
