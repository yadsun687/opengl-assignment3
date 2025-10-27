cmake_minimum_required(VERSION 3.10)
project(OPENGL_APP)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # for zed editor
set(CMAKE_CONFIGURATION_TYPES "Release" CACHE STRING "" FORCE)
set(CMAKE_BUILD_TYPE "Release")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# add libralies
find_package(glm CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
# find_package(OpenMP REQUIRED)
# find_package(Freetype CONFIG REQUIRED)
# find_package(imgui CONFIG REQUIRED)
find_package(stb REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(soil2 CONFIG REQUIRED)


configure_file(configuration/root_directory.h.in configuration/root_directory.h)
include_directories(${CMAKE_BINARY_DIR}/configuration)

file(GLOB_RECURSE OPENGLApp_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.vs"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.fs"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.inl"
)

file(GLOB_RECURSE EXTRA_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/includes/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/includes/*.h"
)

add_executable(OPENGL_APP ${OPENGLApp_SOURCES} ${EXTRA_SOURCES})

# Add this after add_executable()
if(WIN32)
    # Copy DLLs to executable directory after build
    add_custom_command(TARGET OPENGL_APP POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_RUNTIME_DLLS:OPENGL_APP>
        $<TARGET_FILE_DIR:OPENGL_APP>
        COMMAND_EXPAND_LISTS
    )
endif()

# move executable into /src
# set_target_properties(OPENGL_APP PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../src)

target_include_directories(OPENGL_APP PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/includes
)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    message("-- This is a Release build.")
    # Add Release-specific configurations here
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message("-- This is a Debug build.")
    # Add Debug-specific configurations here
else()
    message("-- Unknown or undefined CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
endif()

# if(OpenMP_CXX_FOUND)
#     target_link_libraries(OPENGL_APP PUBLIC OpenMP::OpenMP_CXX)
# endif()
target_link_libraries(OPENGL_APP PUBLIC
    glfw
    glm::glm
    glad::glad
    assimp::assimp
    soil2
#     Freetype::Freetype
#     imgui::imgui
)

target_compile_options(OPENGL_APP PUBLIC
    -Wall
    -Wextra
    -Wpedantic
)
