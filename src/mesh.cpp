#include "mesh.h"
#include "transform.h"

#define TINYOBJLOADER_IMPLEMENTATION
#define TINYOBJLOADER_USE_MAPBOX_EARCUT
#include "tiny_obj_loader.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>


std::vector<tinyobj::material_t> materials;

    std::string Mesh::get_base_dir(std::string_view filepath) {
        size_t pos = filepath.find_last_of("/\\");
        return (pos != std::string::npos) ? std::string(filepath.substr(0, pos)) : std::string{};
    }

    void Mesh::fix_path(std::string &path) {
        if (path.empty()) return;
#ifdef _WIN32
        std::ranges::replace(path, '/', '\\');
#else
        std::ranges::replace(path, '\\', '/');
#endif
    }

    void Mesh::check_errors(const std::string& desc) {
        GLenum error;
        while ((error = glGetError()) != GL_NO_ERROR) {
            std::cerr << std::format("OpenGL error in \"{}\": {} (0x{:X})\n", desc, error, error);
        }
        if (error != GL_NO_ERROR) {
            std::exit(20);
        }
    }

    void Mesh::load_texture (std::string filename, const std::string& texname, DataTex& data) {
        fix_path(filename);
        std::filesystem::path texName = texname;

        if (texName.empty() || data.textures.contains(texName)) return;

        std::filesystem::path base_dir = get_base_dir(filename);
        if (base_dir.empty()) {
            base_dir = ".";
        }

        if (!std::filesystem::exists(texName)) {
            texName = base_dir / texName;
            std::string newTexPath = texName.generic_string();
            fix_path(newTexPath);
            texName = newTexPath;
            if (!std::filesystem::exists(newTexPath)) {
                std::cerr << "Unable to find file: " << newTexPath << "\n";
                exit(1);
            }
        }

        GLuint texture_id;
        int w, h, comp;
        unsigned char* image = stbi_load(texName.string().c_str(), &w, &h, &comp, STBI_default);
        if (!image) {
            std::cerr << "Unable to load texture: " << texName << std::endl;
            exit(1);
        }

        std::cout << "Loaded texture: " << texName << ", w = " << w
                  << ", h = " << h << ", comp = " << comp << std::endl;

        glGenTextures(1, &texture_id);
        glBindTexture(GL_TEXTURE_2D, texture_id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        GLint format;
        if (comp == 1) format = GL_RED;
        else if (comp == 2) format = GL_RG;
        else if (comp == 3) format = GL_RGB;
        else if (comp == 4) format = GL_RGBA;
        else {
            std::cerr << "Unsupported texture format\n";
            std::exit(1);
        }

        glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format, GL_UNSIGNED_BYTE, image);
        glGenerateMipmap(GL_TEXTURE_2D);

        glBindTexture(GL_TEXTURE_2D, 0);
        stbi_image_free(image);

        data.textures.try_emplace(texname, texture_id);
    }

    void Mesh::bind_material_textures(const texture_names& mat, GLuint programId, DataTex& data) {
        // Activate and bind each texture type you care about
        // (Ambient, Diffuse, Specular, etc.) to different texture units.
        // Then set the appropriate uniform sampler in the shader.

        auto try_bind = [&programId, &data](const std::string& texName, const std::string &uniformName, int unit) {
            if (!texName.empty() && data.textures.contains(texName)) {
                glActiveTexture(GL_TEXTURE0 + unit);
                glBindTexture(GL_TEXTURE_2D, data.textures[texName]);
                glUniform1i(glGetUniformLocation(programId, uniformName.c_str()), unit);
            }
        };

        try_bind(mat.ambient_texname,            "u_ambientTex",         0);
        try_bind(mat.diffuse_texname,            "u_diffuseTex",         1);
        try_bind(mat.specular_texname,           "u_specularTex",        2);
        try_bind(mat.specular_highlight_texname, "u_specularHighTex",    3);
    }

    DataTex Mesh::load_obj(const std::string &filename) {

        tinyobj::ObjReaderConfig config;
        config.triangulation_method = "earcut";
        config.triangulate = true;
        config.vertex_color = false;

        DataTex data = DataTex();
        // Each vertex is 8 floats: pos(3), normal(3), tex(2)
        GLsizei stride = (3 + 3 + 2) * sizeof(float);
        tinyobj::ObjReader reader;
        if (!reader.ParseFromFile(filename, config)) {
            if (!reader.Error().empty()) {
                std::cerr << "TinyObjReader Error: " << reader.Error() << '\n';
            }
            return {};
        }

        if (!reader.Warning().empty()) {
            std::cout << "TinyObjReader Warning: " << reader.Warning() << '\n';
        }

        auto& inattrib = reader.GetAttrib();
        auto& inshapes = reader.GetShapes();
        materials = reader.GetMaterials();

        // Append a default material
        materials.emplace_back();

        // Load textures
        for (const tinyobj::material_t& mat : materials) {
            if(!mat.ambient_texname.empty()) load_texture(filename, mat.ambient_texname, data);
            if(!mat.diffuse_texname.empty()) load_texture(filename, mat.diffuse_texname, data);
            if(!mat.specular_texname.empty()) load_texture(filename, mat.specular_texname, data);
            if(!mat.specular_highlight_texname.empty()) load_texture(filename, mat.specular_highlight_texname, data);
        }

        glm::vec3 bmin(FLT_MAX);
        glm::vec3 bmax(-FLT_MAX);

        for (int s = 0; s < inshapes.size(); s++) {
            DrawObject o{};
            std::vector<float> buffer;  // pos(3), normal(3), tex(2)

            for (size_t f = 0; f < inshapes[s].mesh.indices.size() / 3; f++) {
                tinyobj::index_t idx0 = inshapes[s].mesh.indices[3 * f + 0];
                tinyobj::index_t idx1 = inshapes[s].mesh.indices[3 * f + 1];
                tinyobj::index_t idx2 = inshapes[s].mesh.indices[3 * f + 2];

                int current_material_id = inshapes[s].mesh.material_ids[f];
                if ((current_material_id < 0) ||
                    (current_material_id >= static_cast<int>(materials.size()))) {
                    current_material_id = static_cast<int>(materials.size()) - 1;
                }

                o.ambient = {
                        materials[current_material_id].ambient[0],
                        materials[current_material_id].ambient[1],
                        materials[current_material_id].ambient[2]
                };
                o.shininess = materials[current_material_id].shininess;

                glm::mat3x2 tc(0.0f);
                if (!inattrib.texcoords.empty() && ((idx0.texcoord_index >= 0) ||
                                                    (idx1.texcoord_index >= 0) ||
                                                    (idx2.texcoord_index >= 0))) {
                    tc[0][0] = inattrib.texcoords[2 * idx0.texcoord_index];
                    tc[0][1] = 1.0f - inattrib.texcoords[2 * idx0.texcoord_index + 1];
                    tc[1][0] = inattrib.texcoords[2 * idx1.texcoord_index];
                    tc[1][1] = 1.0f - inattrib.texcoords[2 * idx1.texcoord_index + 1];
                    tc[2][0] = inattrib.texcoords[2 * idx2.texcoord_index];
                    tc[2][1] = 1.0f - inattrib.texcoords[2 * idx2.texcoord_index + 1];
                }

                glm::mat3 v(0.0f);
                for (int k = 0; k < 3; k++) {
                    int f0 = idx0.vertex_index;
                    int f1 = idx1.vertex_index;
                    int f2 = idx2.vertex_index;
                    v[0][k] = inattrib.vertices[3 * f0 + k];
                    v[1][k] = inattrib.vertices[3 * f1 + k];
                    v[2][k] = inattrib.vertices[3 * f2 + k];

                    bmin[k] = std::min({bmin[k], v[0][k], v[1][k], v[2][k]});
                    bmax[k] = std::max({bmax[k], v[0][k], v[1][k], v[2][k]});
                }

                glm::mat3 n(0.0f);
                if (!inattrib.normals.empty()) {
                    int nf0 = idx0.normal_index;
                    int nf1 = idx1.normal_index;
                    int nf2 = idx2.normal_index;
                    if ((nf0 >= 0) || (nf1 >= 0) || (nf2 >= 0)) {
                        for (int k = 0; k < 3; k++) {
                            n[0][k] = inattrib.normals[3 * nf0 + k];
                            n[1][k] = inattrib.normals[3 * nf1 + k];
                            n[2][k] = inattrib.normals[3 * nf2 + k];
                        }
                    }
                }

                // Store vertex data: position(3), normal(3), texcoords(2)
                for (int k = 0; k < 3; k++) {
                    buffer.insert(buffer.end(), {
                            v[k][0], v[k][1], v[k][2],
                            n[k][0], n[k][1], n[k][2],
                            tc[k][0], tc[k][1]
                    });
                }
            }

            if (!inshapes[s].mesh.material_ids.empty()
                && inshapes[s].mesh.material_ids.size() > s) {
                // Use material ID of the first face
                o.material_id = inshapes[s].mesh.material_ids[0];
            } else {
                // = ID for default material.
                o.material_id = materials.size() - 1;
            }

            if (o.material_id < materials.size()) {
                tinyobj::material_t mat = materials[o.material_id];
                o.texNames.ambient_texname = mat.ambient_texname;
                o.texNames.diffuse_texname = mat.diffuse_texname;
                o.texNames.specular_texname = mat.specular_texname;
                o.texNames.specular_highlight_texname = mat.specular_highlight_texname;
            }

            if (!buffer.empty()) {
                GLuint vao;
                GLuint vbo;
                glGenVertexArrays(1, &vao);
                glBindVertexArray(vao);
                glGenBuffers(1, &vbo);
                glBindBuffer(GL_ARRAY_BUFFER, vbo);
                glBufferData(GL_ARRAY_BUFFER, buffer.size() * sizeof(float), buffer.data(), GL_STATIC_DRAW);

                glEnableVertexAttribArray(0); // pos
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);

                glEnableVertexAttribArray(1); // normal
                glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));

                glEnableVertexAttribArray(2); // texcoord
                glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));

                glBindVertexArray(0);
                o.material_size = materials.size();
                o.vao = vao;
                o.vbo = vbo;
                o.numTriangles = buffer.size() / (3 + 3 + 2) / 3;
                o.bmin = bmin;
                o.bmax = bmax;
            }

            data.m_draw_objects.push_back(o);
        }
        materials.clear();
        return data;
    }

    void Mesh::draw(GLenum face, GLenum type, GLuint programID, DataTex& data) {

        glPolygonMode(face, type);
        glEnable(GL_POLYGON_OFFSET_FILL);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glPolygonOffset(1.0, 1.0);
        for (auto const& o : data.m_draw_objects) {
            glBindVertexArray(o.vao);
            // Bind texture if valid
            if (o.material_id < o.material_size) {
                bind_material_textures(o.texNames, programID, data);
            }

            glUniform3fv(glGetUniformLocation(programID, "ambient"), 1, glm::value_ptr(o.ambient));
            glUniform1fv(glGetUniformLocation(programID, "shininess"), 1, &o.shininess);

            glDrawArrays(GL_TRIANGLES, 0, 3 * o.numTriangles);
            glBindVertexArray(0);
        }
    }