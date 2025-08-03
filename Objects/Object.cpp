#include "Object.hpp"

Object::Object() {}
Object::~Object() {}
void Object::setName(const std::string &_name) { name = _name; }
void Object::setModelTransform(glm::mat4 &_transform) { modelMatrix = _transform; }
