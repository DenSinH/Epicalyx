find_package(SDL2 REQUIRED)

# compatibility for linux
include_directories(${SDL2_INCLUDE_DIRS})

add_library(CycleFrontend STATIC
        imgui_impl_opengl3.cpp
        imgui_impl_opengl3.h
        imgui_impl_sdl.cpp
        imgui_impl_sdl.h
        imnodes/ImNodes.h
        imnodes/ImNodes.cpp
        imnodes/ImNodesEz.h
        imnodes/ImNodesEz.cpp
        KHR/khrplatform.h
        glad/glad.c
        glad/glad.h)

add_subdirectory(imgui)

target_link_libraries(CycleFrontend PRIVATE ${SDL2_LIBRARY})
if (WIN32)
    target_link_libraries(CycleFrontend PRIVATE imgui)
else()
    target_link_libraries(CycleFrontend PRIVATE imgui ${CMAKE_DL_LIBS})
endif()

target_include_directories(CycleFrontend PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${SDL2_INCLUDE_DIR})

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    # copy over SDL2.dll, does not happen automatically for me for some reason
    message("Copying SDL2.dll to ${CMAKE_BINARY_DIR}")
    file(COPY ${CMAKE_SOURCE_DIR}/files/SDL2.dll
            DESTINATION ${CMAKE_BINARY_DIR})
endif()