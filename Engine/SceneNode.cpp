#include "SceneNode.h"
#include <utility>

SceneNode::SceneNode() : SceneNode("SceneNode") {}

SceneNode::SceneNode(const std::string& name)
    : nodeName(name), parentNode(nullptr), localPosition(0.0f, 0.0f, 0.0f), localRotation(0.0f, 0.0f, 0.0f), localScale(1.0f, 1.0f, 1.0f) {}

SceneNode::~SceneNode() = default;

SceneNode* SceneNode::parent() const { return parentNode; }
const std::string& SceneNode::name() const { return nodeName; }

void SceneNode::setPosition(const Vec3& value) { localPosition = value; }
void SceneNode::setRotation(const Vec3& value) { localRotation = value; }
void SceneNode::setScale(const Vec3& value) { localScale = value; }

const Vec3& SceneNode::position() const { return localPosition; }
const Vec3& SceneNode::rotation() const { return localRotation; }
const Vec3& SceneNode::scale() const { return localScale; }

void SceneNode::translate(const Vec3& delta) { localPosition += delta; }
void SceneNode::rotate(const Vec3& delta) { localRotation += delta; }
void SceneNode::rescale(const Vec3& factor) { localScale.x *= factor.x; localScale.y *= factor.y; localScale.z *= factor.z; }

void SceneNode::addChild(const std::shared_ptr<SceneNode>& child) {
    if (!child) {
        return;
    }
    child->parentNode = this;
    childNodes.push_back(child);
}

const std::vector<std::shared_ptr<SceneNode>>& SceneNode::children() const { return childNodes; }

Mat4 SceneNode::localMatrix() const {
    return Mat4::translation(localPosition) * Mat4::rotationY(localRotation.y) * Mat4::rotationX(localRotation.x) * Mat4::rotationZ(localRotation.z) * Mat4::scale(localScale);
}

Mat4 SceneNode::worldMatrix() const {
    if (parentNode) {
        return parentNode->worldMatrix() * localMatrix();
    }
    return localMatrix();
}

void SceneNode::renderRecursive(const Mat4& parentMatrix) const {
    Mat4 world = parentMatrix * localMatrix();
    renderSelf(world);
    for (const auto& child : childNodes) {
        child->renderRecursive(world);
    }
}

void SceneNode::renderSelf(const Mat4&) const {}
