#include "echo/Scene.h"
#include "echo/Entity.h"
#include "echo/Components.h"

namespace Echo {

Scene::Scene(const std::string& name)
    : m_Name(name)
{
}

Entity Scene::CreateEntity(const std::string& name)
{
    entt::entity handle = m_Registry.create();
    Entity entity(handle, this);

    // Toda entidad tiene Tag y Transform por defecto
    auto& tag = entity.Add<TagComponent>();
    tag.name = name;
    tag.uuid = GenerateUUID();

    entity.Add<TransformComponent>();

    return entity;
}

void Scene::DestroyEntity(Entity entity)
{
    m_Registry.destroy(entity.GetHandle());
}

std::size_t Scene::GetEntityCount() const
{
    return m_Registry.storage<entt::entity>()->in_use();
}

uint64_t Scene::GenerateUUID()
{
    return m_NextUUID++;
}

// =============================================================================
// Entity template implementations (necesitan ver Scene completo)
// =============================================================================

template<typename T, typename... Args>
T& Entity::Add(Args&&... args)
{
    assert(IsValid());
    return m_Scene->GetRegistry().emplace<T>(m_Handle, std::forward<Args>(args)...);
}

template<typename T>
T& Entity::Get()
{
    assert(IsValid() && Has<T>());
    return m_Scene->GetRegistry().get<T>(m_Handle);
}

template<typename T>
const T& Entity::Get() const
{
    assert(IsValid() && Has<T>());
    return m_Scene->GetRegistry().get<T>(m_Handle);
}

template<typename T>
bool Entity::Has() const
{
    return IsValid() && m_Scene->GetRegistry().all_of<T>(m_Handle);
}

template<typename T>
void Entity::Remove()
{
    assert(IsValid() && Has<T>());
    m_Scene->GetRegistry().remove<T>(m_Handle);
}

bool Entity::IsValid() const
{
    return m_Handle != entt::null && m_Scene != nullptr
           && m_Scene->GetRegistry().valid(m_Handle);
}

// Instanciaciones explícitas de todos los componentes
template TagComponent& Entity::Add<TagComponent>();
template TransformComponent& Entity::Add<TransformComponent>();
template SpriteRendererComponent& Entity::Add<SpriteRendererComponent>();
template AnimationComponent& Entity::Add<AnimationComponent>();
template ColliderComponent& Entity::Add<ColliderComponent>();
template RigidBodyComponent& Entity::Add<RigidBodyComponent>();
template CameraComponent& Entity::Add<CameraComponent>();
template TilemapComponent& Entity::Add<TilemapComponent>();

template TagComponent& Entity::Get<TagComponent>();
template TransformComponent& Entity::Get<TransformComponent>();
template SpriteRendererComponent& Entity::Get<SpriteRendererComponent>();
template AnimationComponent& Entity::Get<AnimationComponent>();
template ColliderComponent& Entity::Get<ColliderComponent>();
template RigidBodyComponent& Entity::Get<RigidBodyComponent>();
template CameraComponent& Entity::Get<CameraComponent>();
template TilemapComponent& Entity::Get<TilemapComponent>();

template const TagComponent& Entity::Get<TagComponent>() const;
template const TransformComponent& Entity::Get<TransformComponent>() const;
template const SpriteRendererComponent& Entity::Get<SpriteRendererComponent>() const;
template const AnimationComponent& Entity::Get<AnimationComponent>() const;
template const ColliderComponent& Entity::Get<ColliderComponent>() const;
template const RigidBodyComponent& Entity::Get<RigidBodyComponent>() const;
template const CameraComponent& Entity::Get<CameraComponent>() const;
template const TilemapComponent& Entity::Get<TilemapComponent>() const;

template bool Entity::Has<TagComponent>() const;
template bool Entity::Has<TransformComponent>() const;
template bool Entity::Has<SpriteRendererComponent>() const;
template bool Entity::Has<AnimationComponent>() const;
template bool Entity::Has<ColliderComponent>() const;
template bool Entity::Has<RigidBodyComponent>() const;
template bool Entity::Has<CameraComponent>() const;
template bool Entity::Has<TilemapComponent>() const;

template void Entity::Remove<TagComponent>();
template void Entity::Remove<TransformComponent>();
template void Entity::Remove<SpriteRendererComponent>();
template void Entity::Remove<AnimationComponent>();
template void Entity::Remove<ColliderComponent>();
template void Entity::Remove<RigidBodyComponent>();
template void Entity::Remove<CameraComponent>();
template void Entity::Remove<TilemapComponent>();

} // namespace Echo
