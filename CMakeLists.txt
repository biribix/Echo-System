cmake_minimum_required(VERSION 3.20)
project(EchoEngine VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ==============================================================================
# Dependencies via FetchContent (descarga automática)
# ==============================================================================
include(FetchContent)

# --- Raylib 5.x ---
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG        5.0
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(raylib)

# --- EnTT (header-only ECS) ---
FetchContent_Declare(
    entt
    GIT_REPOSITORY https://github.com/skypjack/entt.git
    GIT_TAG        v3.13.2
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(entt)

# --- nlohmann/json (header-only) ---
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.3
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(json)

# --- Dear ImGui (no tiene CMake nativo, lo compilamos manualmente) ---
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        v1.90.4
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(imgui)

# --- rlImGui (binding Raylib <-> ImGui) ---
FetchContent_Declare(
    rlimgui
    GIT_REPOSITORY https://github.com/raylib-extras/rlImGui.git
    GIT_TAG        main
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(rlimgui)

# ==============================================================================
# Librería ImGui (compilada como static lib)
# ==============================================================================
add_library(imgui_lib STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
)
target_include_directories(imgui_lib PUBLIC ${imgui_SOURCE_DIR})

# ==============================================================================
# Librería rlImGui (compilada como static lib)
# ==============================================================================
add_library(rlimgui_lib STATIC
    ${rlimgui_SOURCE_DIR}/rlImGui.cpp
)
target_include_directories(rlimgui_lib PUBLIC ${rlimgui_SOURCE_DIR})
target_link_libraries(rlimgui_lib PUBLIC imgui_lib raylib)

# ==============================================================================
# Ejecutable principal: EchoEngine
# ==============================================================================
file(GLOB_RECURSE ENGINE_SOURCES
    ${CMAKE_SOURCE_DIR}/src/*.cpp
)

add_executable(EchoEngine ${ENGINE_SOURCES})

target_include_directories(EchoEngine PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(EchoEngine PRIVATE
    raylib
    imgui_lib
    rlimgui_lib
    EnTT::EnTT
    nlohmann_json::nlohmann_json
)

# Copiar assets al directorio de build
add_custom_command(TARGET EchoEngine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:EchoEngine>/assets
)
