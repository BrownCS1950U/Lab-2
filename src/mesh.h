#pragma once

#include "debug.h"

#include <glm/glm.hpp>
#include <vector>
#include <unordered_map>

struct texture_names {
    std::string ambient_texname;             // map_Ka
    std::string diffuse_texname;             // map_Kd
    std::string specular_texname;            // map_Ks
    std::string specular_highlight_texname;  // map_Ns
};

struct DrawObject {
    GLuint vao = 0;
    GLuint vbo = 0;
    size_t numTriangles = 0;
    size_t material_id = -1;

    glm::vec3 bmin; // Boundary Min
    glm::vec3 bmax; // Boundary Max

    // Material properties
    glm::vec3 ambient;
    float shininess;
    int material_size;
    texture_names texNames;
};

class DataTex {

public:

    std::unordered_map<std::string, GLuint> textures;
    std::vector<DrawObject> m_draw_objects;

    void cleanup() {
        // Delete all textures
        for (auto& [name, texId] : textures) {
            if (texId != 0) {
                glDeleteTextures(1, &texId);
            }
        }
        textures.clear();

        // Delete all VAOs and VBOs
        for (auto& obj : m_draw_objects) {
            if (obj.vbo != 0) {
                glDeleteBuffers(1, &obj.vbo);
                obj.vbo = 0;
            }
            if (obj.vao != 0) {
                glDeleteVertexArrays(1, &obj.vao);
                obj.vao = 0;
            }
        }
        m_draw_objects.clear();
    }
};

class Mesh{

public:

    static DataTex load_obj(const std::string &filename);
    static void draw(GLenum face, GLenum type, GLuint programID, DataTex& data);
    static void check_errors(const std::string& desc);

private:
    static std::string get_base_dir(std::string_view filepath);
    static void fix_path(std::string &path);
    static void load_texture (std::string filename, const std::string& texname, DataTex& data);
    static void bind_material_textures(const texture_names& mat, GLuint programId, DataTex& data);

};