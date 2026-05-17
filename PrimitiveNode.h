#ifndef PRIMITIVENODE_H
#define PRIMITIVENODE_H

#include "Light.h"

class PrimitiveNode : public SceneNode {
public:
    explicit PrimitiveNode(const std::string& name);

    void setMaterial(const Material& value);
    const Material& material() const;

protected:
    void renderSelf(const Mat4& worldMatrix) const override;
    virtual void drawGeometry(bool solid) const = 0;

private:
    Material surfaceMaterial;
};

class CubeNode : public PrimitiveNode {
public:
    explicit CubeNode(float size);
    void setSize(float value);
    float size() const;

protected:
    void drawGeometry(bool solid) const override;

private:
    float cubeSize;
};

class CylinderNode : public PrimitiveNode {
public:
    CylinderNode(float radius, float height, int slices = 24);
    void setRadius(float value);
    void setHeight(float value);
    void setSlices(int value);

    float radius() const;
    float height() const;
    int slices() const;

protected:
    void drawGeometry(bool solid) const override;

private:
    float cylinderRadius;
    float cylinderHeight;
    int cylinderSlices;
};

#endif