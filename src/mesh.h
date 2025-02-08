#pragma once

#include <glm/glm.hpp>

#include <vector>
#include <iostream>
#include <unordered_map>
#include "debug.h"

struct texture_names {
    std::string ambient_texname;   // map_Ka. For ambient or ambient occlusion.
    std::string diffuse_texname;   // map_Kd
    std::string specular_texname;  // map_Ks
    std::string specular_highlight_texname;  // map_Ns
    std::string bump_texname;                // map_bump, map_Bump, bump
    std::string alpha_texname;               // map_d
    std::string reflection_texname;
};

struct DrawObject {
    GLuint vao = 0;
    GLuint vbo = 0; // vertex buffer id
    size_t numTriangles = 0;
    size_t material_id = -1;

    glm::vec3 bmin; // Boundary Min
    glm::vec3 bmax; // Boundary Max

    // Texture related
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    glm::vec3 transmittance;
    glm::vec3 emission;
    float shininess;
    float ior;
    float dissolve;
    int illum;
    int material_size;
    texture_names texNames;
};

namespace gl {
class DataTex {

public:

    std::unordered_map<std::string, GLuint> textures;
    std::vector<DrawObject> m_draw_objects;
};

class Mesh{

public:

    static DataTex load_obj(const std::string &filename);
    static void draw(GLenum face, GLenum type, GLuint programID, gl::DataTex& data);
    static void check_errors(const std::string& desc);

private:
    static std::string get_base_dir(std::string_view filepath);
    static void fix_path(std::string &path);
    static void load_texture (std::string filename, const std::string& texname, gl::DataTex& data);
    static void bind_material_textures(const texture_names& mat, GLuint programId, gl::DataTex& data);

};
}