#ifndef CAMERA_H
#define CAMERA_H

#include "SceneNode.h"

// Kamera obsluguje dwa tryby pracy:
//   ORBIT        - eye krazy wokol punktu docelowego (orbitalna)
//   FIRST_PERSON - eye jest staly, patrzymy w kierunku (yaw, pitch)
class Camera : public SceneNode {
public:
    enum class Mode { ORBIT, FIRST_PERSON };

    Camera();

    // ---- Tryb orbitalny ----
    void setTarget(const Vec3& value);
    void setOrbit(float yawRadians, float pitchRadians, float distance);

    // ---- Tryb pierwszoosobowy ----
    void setFirstPerson(const Vec3& eyePos, float yawRadians, float pitchRadians);
    void setEyePosition(const Vec3& eye);

    // ---- Wspolne ----
    void setProjectionPerspective(float fovDegrees, float aspect, float nearPlane, float farPlane);
    void setProjectionOrthographic(float left, float right, float bottom, float top, float nearPlane, float farPlane);

    Mat4 viewMatrix() const;
    Mat4 projectionMatrix() const;

    Vec3  eyePosition()   const;
    Vec3  lookDirection() const;
    float yawAngle()      const { return yaw;   }
    float pitchAngle()    const { return pitch; }
    Mode  mode()          const { return cameraMode; }

private:
    Mode  cameraMode;

    // Tryb orbitalny:
    Vec3  orbitTarget;
    float orbitDistance;

    // Tryb pierwszoosobowy:
    Vec3  fpEye;

    // Wspolne dla obu trybow:
    float yaw;
    float pitch;
    Mat4  projection;
};

#endif
