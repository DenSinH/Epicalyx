add_library(Cycle STATIC
        Cycle.h
        Cycle.cpp
        Graph.h
)

find_package(Graphviz)
if (NOT ${GRAPHVIZ_CGRAPH_FOUND})
  add_compile_definitions(NO_GRAPHVIZ)
  message(WARNING "Graphviz CGraph not found, please set GRAPHVIZ_ROOT environment variable as a hint.")
else()
  message(STATUS "Found Graphviz at")
  message(STATUS "${GRAPHVIZ_CGRAPH_LIBRARY}")
  message(STATUS "${GRAPHVIZ_GVC_LIBRARY}")
  target_include_directories(Cycle PRIVATE
    # to make sure we can include as <graphviz/header.h>
    ${GRAPHVIZ_INCLUDE_DIR}/../
  )
  target_link_libraries(Cycle PUBLIC ${GRAPHVIZ_CGRAPH_LIBRARY} ${GRAPHVIZ_GVC_LIBRARY})
endif()

target_include_directories(Cycle PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_precompile_headers(Cycle REUSE_FROM epicalyx-config)
