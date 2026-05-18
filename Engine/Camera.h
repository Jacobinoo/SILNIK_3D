#ifndef CAMERA_H
#define CAMERA_H

#include "SceneNode.h"

class Camera : public SceneNode {
public:
    Camera();

    void setTarget(const Vec3& value);
    void setOrbit(float yawRadians, float pitchRadians, float distance);
    void setProjectionPerspective(float fovDegrees, float aspect, float nearPlane, float farPlane);
    void setProjectionOrthographic(float left, float right, float bottom, float top, float nearPlane, float farPlane);

    Mat4 viewMatrix() const;
    Mat4 projectionMatrix() const;
    Vec3 eyePosition() const;

private:
    Vec3 targetPoint;
    float yaw;
    float pitch;
    float distance;
    Mat4 projection;
};

#endif