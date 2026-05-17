#include "Camera.h"
#include <cmath>

Camera::Camera()
    : SceneNode("Camera"), targetPoint(0.0f, 0.0f, 0.0f), yaw(0.0f), pitch(0.0f), distance(5.0f), projection(Mat4::identity()) {}

void Camera::setTarget(const Vec3& value) { targetPoint = value; }

void Camera::setOrbit(float yawRadians, float pitchRadians, float orbitDistance) {
    yaw = yawRadians;
    pitch = pitchRadians;
    distance = orbitDistance;
}

void Camera::setProjectionPerspective(float fovDegrees, float aspect, float nearPlane, float farPlane) {
    projection = Mat4::perspective(fovDegrees * 3.14159265358979323846f / 180.0f, aspect, nearPlane, farPlane);
}

void Camera::setProjectionOrthographic(float left, float right, float bottom, float top, float nearPlane, float farPlane) {
    projection = Mat4::orthographic(left, right, bottom, top, nearPlane, farPlane);
}

Vec3 Camera::eyePosition() const {
    return Vec3(
        targetPoint.x + distance * std::cos(pitch) * std::sin(yaw),
        targetPoint.y + distance * std::sin(pitch),
        targetPoint.z + distance * std::cos(pitch) * std::cos(yaw)
    );
}

Mat4 Camera::viewMatrix() const {
    Vec3 eye = eyePosition();
    return Mat4::lookAt(eye, targetPoint, Vec3(0.0f, 1.0f, 0.0f));
}

Mat4 Camera::projectionMatrix() const { return projection; }
