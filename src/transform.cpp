#include <iostream>

#include "transform.h"

namespace gl {
glm::mat3 Transform::rotate(const float degrees, const glm::vec3& axis){
	const float radian = degrees * (glm::pi<float>() / 180.0f);

    glm::mat3 dot = glm::outerProduct(axis, axis);
    auto cross = glm::mat3(
		0, -axis.z, axis.y,
		axis.z, 0, -axis.x,
		-axis.y, axis.x, 0
		);
	return glm::mat3(glm::mat3(cos(radian)) *
           glm::mat3(1.0f) + glm::mat3(1.0f - cos(radian)) * dot +
           glm::mat3(sin(radian)) * glm::transpose(cross));
}

void Transform::left(float degrees, glm::vec3& eye, const glm::vec3& up){
	glm::mat3 rotation = (rotate(degrees, normalize(up)));
	eye = rotation * eye; 
}

void Transform::up(float degrees, glm::vec3& eye, glm::vec3& up){
	glm::vec3 axis_for_up = glm::cross(glm::normalize(up), glm::normalize(eye));
	glm::mat3 rotation = rotate(degrees, glm::normalize(axis_for_up));
	eye = eye * rotation; 
	up = up * rotation; 
}

glm::mat4 Transform::scale(const glm::vec3& scale){
  	glm::mat4 ret;
	ret = glm::transpose(glm::mat4(
		scale.x, 0, 0, 0,
		0, scale.y, 0, 0,
		0, 0, scale.z, 0,
		0, 0, 0, 1
		));
	return ret;
}

glm::mat4 Transform::translate(const glm::vec3& translate){
  	glm::mat4 ret;
	ret = transpose(glm::mat4(
		1, 0, 0, translate.x,
		0, 1, 0, translate.y,
		0, 0, 1, translate.z,
		0, 0, 0, 1
		));	
	return ret;
}

glm::vec3 Transform::upvector(const glm::vec3 &up, const glm::vec3 & zvec){
    glm::vec3 x = glm::cross(up,zvec);
    glm::vec3 y = glm::cross(zvec,x);
    glm::vec3 ret = glm::normalize(y);
    return ret; 
}
}