add_library(Cycle STATIC
        Cycle.h
        Cycle.cpp
        Graph.h
)

add_subdirectory(frontend)

target_include_directories(Cycle PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/frontend
        $<TARGET_PROPERTY:CycleFrontend,INCLUDE_DIRECTORIES>
)


target_link_libraries(Cycle CycleFrontend)