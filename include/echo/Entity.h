#pragma once

#include "entt/entt.hpp"
#include <cassert>

namespace Echo {

class Scene;

/// Wrapper ligero sobre entt::entity.
/// Permite acceder a componentes con sintaxis limpia: entity.Get<Transform>()
class Entity {
public:
    Entity() = default;
    Entity(entt::entity handle, Scene* scene)
        : m_Handle(handle), m_Scene(scene) {}

    template<typename T, typename... Args>
    T& Add(Args&&... args);

    template<typename T>
    T& Get();

    template<typename T>
    const T& Get() const;

    template<typename T>
    bool Has() const;

    template<typename T>
    void Remove();

    bool IsValid() const;
    entt::entity GetHandle() const { return m_Handle; }
    Scene* GetScene() const { return m_Scene; }

    bool operator==(const Entity& other) const {
        return m_Handle == other.m_Handle && m_Scene == other.m_Scene;
    }
    bool operator!=(const Entity& other) const { return !(*this == other); }
    operator bool() const { return IsValid(); }
    operator entt::entity() const { return m_Handle; }

private:
    entt::entity m_Handle = entt::null;
    Scene* m_Scene = nullptr;
};

} // namespace Echo
