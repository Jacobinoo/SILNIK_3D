#include "Light.h"
#include <GL/freeglut.h>

Material::Material() : ambient(0.2f, 0.2f, 0.2f), diffuse(0.8f, 0.8f, 0.8f), specular(0.0f, 0.0f, 0.0f), shininess(0.0f) {}

Material::Material(const Vec3& ambientValue, const Vec3& diffuseValue, const Vec3& specularValue, float shininessValue)
    : ambient(ambientValue), diffuse(diffuseValue), specular(specularValue), shininess(shininessValue) {}

PointLight::PointLight()
    : SceneNode("PointLight"), ambientColor(0.15f, 0.15f, 0.15f), diffuseColor(1.0f, 1.0f, 1.0f), specularColor(1.0f, 1.0f, 1.0f), constantAttenuation(1.0f), linearAttenuation(0.0f), quadraticAttenuation(0.0f), isEnabled(true), activeLightIndex(0) {}

void PointLight::setAmbient(const Vec3& value) { ambientColor = value; }
void PointLight::setDiffuse(const Vec3& value) { diffuseColor = value; }
void PointLight::setSpecular(const Vec3& value) { specularColor = value; }
void PointLight::setAttenuation(float constantValue, float linearValue, float quadraticValue) {
    constantAttenuation = constantValue;
    linearAttenuation = linearValue;
    quadraticAttenuation = quadraticValue;
}
void PointLight::setEnabled(bool value) { isEnabled = value; }
void PointLight::setLightIndex(int value) { activeLightIndex = value < 0 ? 0 : value; }
bool PointLight::enabled() const { return isEnabled; }
int PointLight::lightIndex() const { return activeLightIndex; }

void PointLight::renderSelf(const Mat4& worldMatrix) const {
    GLenum lightId = GL_LIGHT0 + activeLightIndex;
    if (!isEnabled) {
        glDisable(lightId);
        return;
    }

    GLfloat ambient[4] = { ambientColor.x, ambientColor.y, ambientColor.z, 1.0f };
    GLfloat diffuse[4] = { diffuseColor.x, diffuseColor.y, diffuseColor.z, 1.0f };
    GLfloat specular[4] = { specularColor.x, specularColor.y, specularColor.z, 1.0f };

    glPushMatrix();
    glMultMatrixf(worldMatrix.data());

    GLfloat position[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    glEnable(lightId);
    glLightfv(lightId, GL_POSITION, position);
    glLightfv(lightId, GL_AMBIENT, ambient);
    glLightfv(lightId, GL_DIFFUSE, diffuse);
    glLightfv(lightId, GL_SPECULAR, specular);
    glLightf(lightId, GL_CONSTANT_ATTENUATION, constantAttenuation);
    glLightf(lightId, GL_LINEAR_ATTENUATION, linearAttenuation);
    glLightf(lightId, GL_QUADRATIC_ATTENUATION, quadraticAttenuation);

    glPopMatrix();
}
