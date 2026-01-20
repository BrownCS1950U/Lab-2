cmake_minimum_required (VERSION 3.16)

project (viewer)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable the required dependencies
set(DEP_GLFW ON "Build GLFW")
set(DEP_GLM ON "Build GLM")
set(DEP_GLEW ON "Build GLEW")
set(DEP_FREETYPE ON "Build FreeType")
set(DEP_STB ON "Include STB headers")
set(DEP_IMGUI ON "Build Dear ImGui")

# ImGui options
set(DEP_IMGUI_BACKEND_GLFW ON "ImGui GLFW backend")
set(DEP_IMGUI_BACKEND_OPENGL3 ON "ImGui OpenGL3 backend")
set(DEP_IMGUI_FREETYPE ON "ImGui FreeType support")

# Add dependencies
add_subdirectory(dependencies)

# Set up executable
set(SOURCE_DIR "${PROJECT_SOURCE_DIR}/src")
file (GLOB SOURCE_FILES "${SOURCE_DIR}/*.cpp")
add_executable (${PROJECT_NAME} ${SOURCE_FILES})

target_include_directories(
        ${PROJECT_NAME} PRIVATE
        ${GLFW_SOURCE_DIR}/include
        ${GLEW_SOURCE_DIR}/include
        ${IMGUI_SOURCE_DIR}
        ${STB_SOURCE_DIR}
)

# OpenGL (system)
find_package(OpenGL REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE OpenGL::GL)

# Project dependencies
target_link_libraries(${PROJECT_NAME} PRIVATE
        # glfw  ## GLFW is linked implicitly via imgui
        glm
        glew_static
        stb
        imgui
)

if (WIN32)
    add_compile_definitions(GLEW_STATIC)
    target_link_libraries(${PROJECT_NAME} opengl32 glu32)
endif()

if (UNIX AND NOT APPLE)
    target_link_libraries(${PROJECT_NAME} GL)
endif()