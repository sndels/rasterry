#include "camera.hpp"

#include <iostream>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

void Camera::lookAt(const vec3& eye, const vec3& target, const vec3& up)
{
    vec3 fwd = normalize(target - eye);
    orient(eye, fwd, up);
}

void Camera::orient(const vec3& eye, const vec3& fwd, const vec3& up)
{
    _eye = eye;
    vec3 z = -fwd;
    vec3 right = normalize(cross(up, z));
    vec3 newUp = normalize(cross(z, right));

    // Right handed camera
    _worldToCamera = mat4{         right.x,          newUp.x,          z.x, 0.f,
                                   right.y,          newUp.y,          z.y, 0.f,
                                   right.z,          newUp.z,          z.z, 0.f,
                          -dot(right, eye), -dot(newUp, eye), -dot(z, eye), 1.f};

    _worldToClip = _cameraToClip * _worldToCamera;
}

void Camera::perspective(const float fov, const float ar, const float zN, const float zF)
{
    const float tf = 1.f / tanf(fov * 0.5);

    // Z in [0,1]
     _cameraToClip = mat4{1.f, 0.f,  0.f, 0.f,
                          0.f, 1.f,  0.f, 0.f,
                          0.f, 0.f, 0.5f, 0.f,
                          0.f, 0.f, 0.5f, 1.f} *
                    mat4{tf / ar, 0.f,                         0.f,  0.f,
                             0.f,  tf,                         0.f,  0.f,
                             0.f, 0.f,       (zF + zN) / (zN - zF), -1.f,
                             0.f, 0.f,     2 * zF * zN / (zN - zF),  0.f};

    _worldToClip = _cameraToClip * _worldToCamera;
}

const glm::vec3& Camera::eye() const
{
    return _eye;
}

const glm::mat4& Camera::worldToCamera() const
{
    return _worldToCamera;
}

const glm::mat4& Camera::cameraToClip() const
{
    return _cameraToClip;
}

const glm::mat4& Camera::worldToClip() const
{
    return _worldToClip;
}
