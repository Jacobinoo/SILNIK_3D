#ifndef LIGHT_H
#define LIGHT_H

#include "SceneNode.h"

struct Material {
    Vec3 ambient;
    Vec3 diffuse;
    Vec3 specular;
    float shininess;

    Material();
    Material(const Vec3& ambientValue, const Vec3& diffuseValue, const Vec3& specularValue, float shininessValue);
};

class PointLight : public SceneNode {
public:
    PointLight();

    void setAmbient(const Vec3& value);
    void setDiffuse(const Vec3& value);
    void setSpecular(const Vec3& value);
    void setAttenuation(float constantValue, float linearValue, float quadraticValue);
    void setEnabled(bool value);
    void setLightIndex(int value);

    bool enabled() const;
    int lightIndex() const;

protected:
    void renderSelf(const Mat4& worldMatrix) const override;

private:
    Vec3 ambientColor;
    Vec3 diffuseColor;
    Vec3 specularColor;
    float constantAttenuation;
    float linearAttenuation;
    float quadraticAttenuation;
    bool isEnabled;
    int activeLightIndex;
};

#endif