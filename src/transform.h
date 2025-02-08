#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace gl {
class Transform{
public:
	static void left(float degrees, glm::vec3& eye, const glm::vec3& up);
	static void up(float degrees, glm::vec3& eye, glm::vec3& up);
    static glm::mat3 rotate(float degrees, const glm::vec3& axis);
    static glm::mat4 scale(const glm::vec3& scale);
    static glm::mat4 translate(const glm::vec3& scale);
    static glm::vec3 upvector(const glm::vec3 &up, const glm::vec3 &zvec);
};
}