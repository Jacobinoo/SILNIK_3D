/**
 * @file Camera.h
 * @brief Kamera silnika - obsluguje 2 tryby: orbita i first-person.
 *
 * Tryb orbita uzywany byl wczesniej przy testach silnika (mysz przeciaga,
 * kamera krazy wokol punktu w przestrzeni). Tryb FP uzywamy w grze - oko
 * ma stala pozycje a yaw/pitch decyduja w ktora strone patrzymy.
 */
#ifndef CAMERA_H
#define CAMERA_H

#include "SceneNode.h"

/**
 * @brief Klasa kamery z dwoma trybami pracy.
 *
 * @details
 * - ORBIT: eye = target + offset(yaw,pitch) * distance, view = lookAt(eye, target, up)
 * - FIRST_PERSON: eye = fpEye (staly), kierunek wyznaczony z yaw/pitch,
 *   view = lookAt(eye, eye+forward, up)
 *
 * Projekcje (perspektywa, ortogonalna) ustawiane sa osobno - kamera trzyma
 * swoja macierz projection.
 */
class Camera : public SceneNode {
public:
    /** @brief Tryby pracy kamery. */
    enum class Mode {
        ORBIT,         ///< krazy wokol punktu target
        FIRST_PERSON   ///< stala pozycja, patrzy gdzie wskazuje yaw/pitch
    };

    /** @brief Tworzy kamere w trybie ORBIT z targetem w (0,0,0) i distance=5. */
    Camera();

    // ---- Tryb orbitalny ----
    /** @brief Ustawia srodek orbity (punkt na ktory kamera patrzy). */
    void setTarget(const Vec3& value);
    /** @brief Ustawia katy i odleglosc orbity (przelacza w tryb ORBIT). */
    void setOrbit(float yawRadians, float pitchRadians, float distance);

    // ---- Tryb pierwszoosobowy ----
    /** @brief Pelne ustawienie FP: pozycja oka + yaw + pitch (przelacza w FP). */
    void setFirstPerson(const Vec3& eyePos, float yawRadians, float pitchRadians);
    /** @brief Sama pozycja oka (yaw/pitch zostawia). */
    void setEyePosition(const Vec3& eye);

    // ---- Projekcje ----
    /** @brief Buduje macierz perspektywiczna i zapisuje w kamerze. */
    void setProjectionPerspective(float fovDegrees, float aspect, float nearPlane, float farPlane);
    /** @brief Buduje macierz ortogonalna. */
    void setProjectionOrthographic(float left, float right, float bottom, float top, float nearPlane, float farPlane);

    /** @brief Zwraca macierz widoku odpowiednia dla aktualnego trybu. */
    Mat4 viewMatrix() const;
    /** @brief Zwraca macierz projekcji ostatnio ustawiona. */
    Mat4 projectionMatrix() const;

    /** @brief Aktualne polozenie oka kamery w world space. */
    Vec3  eyePosition()   const;
    /**
     * @brief Aktualny kierunek patrzenia (znormalizowany).
     *
     * W trybie FP liczony z yaw/pitch, w ORBIT z target-eye.
     * Uzywane przez raycast w grze - origin = eyePosition, dir = lookDirection.
     */
    Vec3  lookDirection() const;

    float yawAngle()      const { return yaw;   } ///< aktualny yaw w radianach
    float pitchAngle()    const { return pitch; } ///< aktualny pitch w radianach
    Mode  mode()          const { return cameraMode; }

private:
    Mode  cameraMode;

    // ---- Stan trybu orbitalnego ----
    Vec3  orbitTarget;     ///< punkt na ktory kamera patrzy
    float orbitDistance;   ///< odleglosc oka od targetu

    // ---- Stan trybu FP ----
    Vec3  fpEye;           ///< pozycja oka w world space (FP only)

    // ---- Wspolne ----
    float yaw;             ///< rotacja wokol Y (lewo-prawo), radiany
    float pitch;           ///< rotacja gore-dol, radiany
    Mat4  projection;      ///< aktualna macierz projekcji
};

#endif
