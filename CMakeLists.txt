cmake_minimum_required(VERSION 3.20)
project(nysor LANGUAGES CXX)

include(FetchContent)
option(NYSOR_BUILD_EDITOR "Build the Dear ImGui and imnodes editor" ON)

set(TF_BUILD_TESTS OFF CACHE BOOL "Disable Taskflow tests" FORCE)
set(TF_BUILD_EXAMPLES OFF CACHE BOOL "Disable Taskflow examples" FORCE)

FetchContent_Declare(
    taskflow
    GIT_REPOSITORY https://github.com/taskflow/taskflow.git
    GIT_TAG v3.8.0
)
FetchContent_MakeAvailable(taskflow)

FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(json)

add_library(nysor INTERFACE)
target_include_directories(nysor INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(nysor INTERFACE cxx_std_20)
target_link_libraries(nysor INTERFACE Taskflow)

add_executable(nysor_example examples/basic_graph.cpp)
target_link_libraries(nysor_example PRIVATE nysor)

add_executable(nysor_core_test tests/core_pipeline.cpp)
target_link_libraries(nysor_core_test PRIVATE nysor)

add_library(nysor_loader STATIC src/block_loader.cpp)
target_include_directories(nysor_loader PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(nysor_loader PUBLIC cxx_std_20)
target_link_libraries(nysor_loader PUBLIC nysor nlohmann_json::nlohmann_json)

add_executable(nysor_block_loader_test tests/block_loader.cpp)
target_link_libraries(nysor_block_loader_test PRIVATE nysor_loader)

add_executable(nysor_block_validation_test tests/block_validation.cpp)
target_link_libraries(nysor_block_validation_test PRIVATE nysor_loader)

add_executable(nysor_external_block_pipeline_test tests/external_block_pipeline.cpp)
target_link_libraries(nysor_external_block_pipeline_test PRIVATE nysor_loader)

add_executable(nysor_graph_composition_test tests/graph_composition.cpp)
target_link_libraries(nysor_graph_composition_test PRIVATE nysor_loader)

add_executable(nysor_port_specification_test tests/port_specification.cpp)
target_link_libraries(nysor_port_specification_test PRIVATE nysor_loader)

add_executable(nysor_06_test tests/0.6/port_system.cpp)
target_link_libraries(nysor_06_test PRIVATE nysor_loader)

enable_testing()
add_test(NAME nysor_example COMMAND nysor_example)
add_test(NAME nysor_core_test COMMAND nysor_core_test)
add_test(NAME nysor_block_loader_test COMMAND nysor_block_loader_test)
add_test(NAME nysor_block_validation_test COMMAND nysor_block_validation_test)
add_test(NAME nysor_external_block_pipeline_test COMMAND nysor_external_block_pipeline_test)
add_test(NAME nysor_graph_composition_test COMMAND nysor_graph_composition_test)
add_test(NAME nysor_port_specification_test COMMAND nysor_port_specification_test)
add_test(NAME nysor_06_test COMMAND nysor_06_test)

if(NYSOR_BUILD_EDITOR)
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "Disable GLFW docs" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "Disable GLFW tests" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "Disable GLFW examples" FORCE)

    FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.4
    )
    FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG v1.91.9b
    )
    FetchContent_Declare(
        imnodes
        GIT_REPOSITORY https://github.com/Nelarius/imnodes.git
        GIT_TAG v0.5
    )
    FetchContent_MakeAvailable(glfw imgui imnodes)

    add_library(nysor_imgui STATIC
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_demo.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
        ${imnodes_SOURCE_DIR}/imnodes.cpp
    )
    target_include_directories(nysor_imgui PUBLIC
        ${imgui_SOURCE_DIR}
        ${imgui_SOURCE_DIR}/backends
        ${imnodes_SOURCE_DIR}
    )
    target_compile_definitions(nysor_imgui PUBLIC IMGUI_DEFINE_MATH_OPERATORS)
    target_link_libraries(nysor_imgui PUBLIC glfw)
    target_compile_features(nysor_imgui PUBLIC cxx_std_20)

    find_package(OpenGL REQUIRED)
    add_executable(nysor_editor editor/main.cpp)
    target_link_libraries(nysor_editor PRIVATE nysor nysor_imgui OpenGL::GL)
endif()