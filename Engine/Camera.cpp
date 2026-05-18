#include "Camera.h"
#include <cmath>

namespace {
    const float PI = 3.14159265358979323846f;
}

Camera::Camera()
    : SceneNode("Camera"),
      cameraMode(Mode::ORBIT),
      orbitTarget(0.0f, 0.0f, 0.0f),
      orbitDistance(5.0f),
      fpEye(0.0f, 1.5f, 0.0f),
      yaw(0.0f), pitch(0.0f),
      projection(Mat4::identity()) {}

void Camera::setTarget(const Vec3& value) {
    orbitTarget = value;
    cameraMode  = Mode::ORBIT;
}

void Camera::setOrbit(float yawRad, float pitchRad, float dist) {
    yaw           = yawRad;
    pitch         = pitchRad;
    orbitDistance = dist;
    cameraMode    = Mode::ORBIT;
}

void Camera::setFirstPerson(const Vec3& eyePos, float yawRad, float pitchRad) {
    fpEye      = eyePos;
    yaw        = yawRad;
    pitch      = pitchRad;
    cameraMode = Mode::FIRST_PERSON;
}

void Camera::setEyePosition(const Vec3& eye) {
    fpEye = eye;
}

void Camera::setProjectionPerspective(float fovDeg, float aspect, float n, float f) {
    projection = Mat4::perspective(fovDeg * PI / 180.0f, aspect, n, f);
}

void Camera::setProjectionOrthographic(float l, float r, float b, float t, float n, float f) {
    projection = Mat4::orthographic(l, r, b, t, n, f);
}

Vec3 Camera::eyePosition() const {
    if (cameraMode == Mode::FIRST_PERSON) {
        return fpEye;
    }
    // Orbit: eye = target + offset(yaw, pitch) * distance
    return Vec3(
        orbitTarget.x + orbitDistance * std::cos(pitch) * std::sin(yaw),
        orbitTarget.y + orbitDistance * std::sin(pitch),
        orbitTarget.z + orbitDistance * std::cos(pitch) * std::cos(yaw)
    );
}

Vec3 Camera::lookDirection() const {
    if (cameraMode == Mode::FIRST_PERSON) {
        // yaw=0, pitch=0 -> patrzymy w kierunku -Z (do przodu)
        // yaw>0 -> obrot w prawo (+X), pitch>0 -> patrz w gore (+Y)
        return Vec3(
             std::sin(yaw) * std::cos(pitch),
             std::sin(pitch),
            -std::cos(yaw) * std::cos(pitch)
        );
    }
    return normalize(orbitTarget - eyePosition());
}

Mat4 Camera::viewMatrix() const {
    Vec3 eye    = eyePosition();
    Vec3 center = (cameraMode == Mode::FIRST_PERSON)
        ? (eye + lookDirection())
        :  orbitTarget;
    return Mat4::lookAt(eye, center, Vec3(0.0f, 1.0f, 0.0f));
}

Mat4 Camera::projectionMatrix() const { return projection; }
