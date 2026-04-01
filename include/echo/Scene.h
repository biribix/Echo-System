#pragma once

#include "entt/entt.hpp"
#include <string>
#include <cstdint>

namespace Echo {

class Entity;

/// Escena del motor. Contiene el registry ECS y gestiona entidades.
class Scene {
public:
    Scene(const std::string& name = "Escena Sin Titulo");
    ~Scene() = default;

    /// Crea una entidad con TagComponent y TransformComponent por defecto
    Entity CreateEntity(const std::string& name = "Entidad");

    /// Destruye una entidad
    void DestroyEntity(Entity entity);

    /// Acceso directo al registry (para los sistemas y paneles)
    entt::registry& GetRegistry() { return m_Registry; }
    const entt::registry& GetRegistry() const { return m_Registry; }

    /// Nombre de la escena
    const std::string& GetName() const { return m_Name; }
    void SetName(const std::string& name) { m_Name = name; }

    /// Número de entidades activas
    std::size_t GetEntityCount() const;

    /// Genera un UUID simple (incremental para esta sesión)
    uint64_t GenerateUUID();

private:
    std::string m_Name;
    entt::registry m_Registry;
    uint64_t m_NextUUID = 1;
};

} // namespace Echo
