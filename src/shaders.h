#pragma once

#include <iostream>
#include <string>

namespace gl {
class Shader{
public:
    
    static GLuint init_shaders (GLenum type, const char * filename);
    static GLuint init_program (GLuint vertexshader, GLuint fragmentshader);

private:
    static std::string read_text_file(const char * filename);

    static void program_errors (GLint program);
    static void shader_errors (GLint shader);
};
}