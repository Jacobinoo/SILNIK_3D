#ifndef SCENENODE_H
#define SCENENODE_H

#include "Math3D.h"
#include <memory>
#include <string>
#include <vector>

class SceneNode {
public:
    SceneNode();
    explicit SceneNode(const std::string& name);
    virtual ~SceneNode();

    SceneNode* parent() const;
    const std::string& name() const;

    void setPosition(const Vec3& value);
    void setRotation(const Vec3& value);
    void setScale(const Vec3& value);

    const Vec3& position() const;
    const Vec3& rotation() const;
    const Vec3& scale() const;

    void translate(const Vec3& delta);
    void rotate(const Vec3& delta);
    void rescale(const Vec3& factor);

    void addChild(const std::shared_ptr<SceneNode>& child);
    const std::vector<std::shared_ptr<SceneNode>>& children() const;

    Mat4 localMatrix() const;
    Mat4 worldMatrix() const;

    void renderRecursive(const Mat4& parentMatrix) const;

protected:
    virtual void renderSelf(const Mat4& worldMatrix) const;

private:
    std::string nodeName;
    SceneNode* parentNode;
    Vec3 localPosition;
    Vec3 localRotation;
    Vec3 localScale;
    std::vector<std::shared_ptr<SceneNode>> childNodes;
};

#endif