#pragma once
#include "../Shading/Shader.hpp"
#include <string>
#include <unordered_map>

class Object
{
public:
    std::string name; // 如何确保name 唯一? 让Name设置交给一个类管理,而不是输入名字就传给对象
    glm::mat4 modelMatrix = glm::identity<glm::mat4>();
    Object();
    virtual void draw(glm::mat4 modelMatrix, Shader &shaders) = 0;
    virtual ~Object();
    void setName(const std::string &_name);
    void setModelTransform(glm::mat4 &_transform);
};
// using Scene = std::vector<std::unique_ptr<Object>>;

class Scene
{
private:
    size_t m_nextObjectID = 0;
public:
    // std::vector<std::unique_ptr<Object>> objects;
    std::unordered_map<size_t, std::unique_ptr<Object>> m_objectMap;
    std::vector<size_t> m_eraseVector;

public:
    Scene() = default;
    ~Scene() = default;
    // 禁用拷贝构造和赋值
    Scene(const Scene &) = delete;
    Scene &operator=(const Scene &) = delete;
    // 启用移动构造和赋值
    Scene(Scene &&) = default;
    Scene &operator=(Scene &&) = default;

public: // 迭代器方法
    auto begin()
    {
        return m_objectMap.begin();
    }

    auto end()
    {
        return m_objectMap.end();
    }
    auto begin() const
    {
        return m_objectMap.begin();
    }

    auto end() const
    {
        return m_objectMap.end();
    }

public:
    //// @brief 添加对象,返回对象ID
    size_t addObject(std::unique_ptr<Object> obj)
    {
        size_t id = m_nextObjectID++;
        m_objectMap[id] = std::move(obj);
        return id;
    }

    /// @brief 通过ID获取对象引用
    auto getObject(size_t id) -> Object&
    {
        auto it = m_objectMap.find(id);
        if (it == m_objectMap.end())
            throw std::runtime_error("Object with ID " + std::to_string(id) + " not found.");
        return *(it->second);
    }

    /// @brief 通过ID删除对象
    void removeObject(size_t id)
    {
        m_eraseVector.push_back(id);
        // m_objectMap.erase(id);
    }

    void update(){
        deferredErase();
    }
    ///////////////内部方法
private:
    /// @brief 延迟删除对象,在帧更新时调用
    void deferredErase()
    {
        for (auto id : m_eraseVector)
        {
            m_objectMap.erase(id);
        }
        m_eraseVector.clear();
    }
};