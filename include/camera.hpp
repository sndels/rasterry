#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <glm/glm.hpp>

class Camera {
public:
    Camera() = default;

    void lookAt(const glm::vec3& eye, const glm::vec3& target, const glm::vec3& up);
    void orient(const glm::vec3& eye, const glm::vec3& fwd, const glm::vec3& up);
    void perspective(const float fov, const float ar, const float zN, const float zF);

    const glm::vec3& eye() const;
    const glm::mat4& worldToCamera() const;
    const glm::mat4& cameraToClip() const;
    const glm::mat4& worldToClip() const;

private:
    glm::vec3 _eye;
    glm::mat4 _worldToClip;
    glm::mat4 _worldToCamera;
    glm::mat4 _cameraToClip;
};

#endif // CAMERA_HPP
