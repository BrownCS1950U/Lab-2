#pragma once

#include "mesh.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <array>
#include <vector>

class Window {
public:

    ~Window();
    static void resize_window(GLFWwindow* window, int width, int height);
    static void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void scroll(GLFWwindow * window, double xoffset, double yoffset);
    static void cursor_enter_callback(GLFWwindow* window, int entered);
    static void mouse(GLFWwindow * window, double xpos, double ypos);
    static void mouseButton(GLFWwindow* window, int button, int action, int mods);
    static void drag_drop(GLFWwindow * window, int count, const char** paths);
    static int initialize(const std::string& filename);
    static void display();
    static void update();
    static bool isActive();
    static void cleanup();
    static void updateMVP(const DataTex& data);
    static void applyTextureFiltering();

private:
    // Variables to hold state
    static float sense;
    static bool active_cursor;
    static bool cursorInsideWindow;
    static bool firstMouseAfterToggle;

    static GLuint shaderProgram;
    static int render_mode;
    static int filter_mode;  // 0 = Bilinear, 1 = Trilinear, 2 = Anisotropic
    static bool vsync_enabled;
    static bool keys[1024];
    static int window_width, window_height;
    static int current_vp_height, current_vp_width;
    static float aspect_ratio;

    static const int num_lights = 5;

    static std::array<glm::vec4, num_lights> m_lightPosn;
    static std::array<glm::vec4, num_lights> m_lightCol;

    static GLFWwindow* glfwWindow;
    static std::vector<DataTex> m_data;
};