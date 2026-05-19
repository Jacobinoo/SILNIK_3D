#ifndef PRIMITIVENODE_H
#define PRIMITIVENODE_H

#include "Light.h"
#include "Texture.h"
#include <memory>

// Bazowa klasa dla renderowanych prymitywów 3D.
// Przechowuje materiał (oświetlenie Phonga) i opcjonalną teksturę.
// Podklasy implementują drawGeometry() z właściwymi UV.
class PrimitiveNode : public SceneNode {
public:
    explicit PrimitiveNode(const std::string& name);

    void setMaterial(const Material& value);
    const Material& material() const;

    void setTexture(const std::shared_ptr<Texture>& tex);
    std::shared_ptr<Texture> texture() const;

protected:
    void renderSelf(const Mat4& worldMatrix) const override;
    virtual void drawGeometry() const = 0;

private:
    Material surfaceMaterial;
    std::shared_ptr<Texture> surfaceTexture;
};

// Sześcian z poprawnymi normalnymi i współrzędnymi UV na każdej ścianie.
class CubeNode : public PrimitiveNode {
public:
    explicit CubeNode(float size);
    void setSize(float value);
    float size() const;

protected:
    void drawGeometry() const override;

private:
    float cubeSize;
};

// Walec (boczna powierzchnia + dwa dyski) z normalnymi i UV.
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
    void drawGeometry() const override;

private:
    float cylinderRadius;
    float cylinderHeight;
    int cylinderSlices;
};

// Sfera z siatką stacks×slices, normalnymi i UV w układzie sferycznym.
class SphereNode : public PrimitiveNode {
public:
    SphereNode(float radius, int stacks = 18, int slices = 36);
    void setRadius(float value);
    float radius() const;

protected:
    void drawGeometry() const override;

private:
    float sphereRadius;
    int sphereStacks;
    int sphereSlices;
};

// Stożek (boczna powierzchnia z trójkątów + dolny dysk) z UV i normalnymi.
// Podstawa w y = -h/2, wierzchołek w y = +h/2. Normalne boczne sa
// nachylone zgodnie z katem zwezania.
class ConeNode : public PrimitiveNode {
public:
    ConeNode(float radius, float height, int slices = 24);
    void setRadius(float value);
    void setHeight(float value);
    void setSlices(int value);

    float radius() const;
    float height() const;
    int   slices() const;

protected:
    void drawGeometry() const override;

private:
    float coneRadius;
    float coneHeight;
    int   coneSlices;
};

// Torus (donut) - parametryzowany dwoma promieniami (major R, minor r).
// Powierzchnia: (x,y,z) = ((R+r*cos(v))*cos(u), r*sin(v), (R+r*cos(v))*sin(u))
class TorusNode : public PrimitiveNode {
public:
    TorusNode(float majorRadius, float minorRadius,
              int majorSlices = 24, int minorSlices = 12);
    void setMajorRadius(float v);
    void setMinorRadius(float v);
    float majorRadius() const;
    float minorRadius() const;

protected:
    void drawGeometry() const override;

private:
    float torusMajor;
    float torusMinor;
    int   torusMajorSlices;
    int   torusMinorSlices;
};

// Płaski czworokąt w płaszczyźnie XZ, normalny skierowany w górę.
class PlaneNode : public PrimitiveNode {
public:
    PlaneNode(float width, float depth);
    void setSize(float w, float d);

protected:
    void drawGeometry() const override;

private:
    float planeWidth;
    float planeDepth;
};

#endif
