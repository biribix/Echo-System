#pragma once

#include "entt/entt.hpp"
#include <memory>

namespace Echo {

class Scene;

/// Panel de propiedades. Muestra y permite editar los componentes
/// de la entidad actualmente seleccionada.
class PropertiesPanel {
public:
    PropertiesPanel() = default;

    void SetScene(const std::shared_ptr<Scene>& scene) { m_Scene = scene; }

    /// Renderiza el panel para la entidad dada
    void Render(entt::entity selectedEntity);

    bool IsVisible() const { return m_Visible; }
    void SetVisible(bool v) { m_Visible = v; }
    void ToggleVisible() { m_Visible = !m_Visible; }

private:
    void RenderTagComponent(entt::entity entity);
    void RenderTransformComponent(entt::entity entity);
    void RenderSpriteRendererComponent(entt::entity entity);
    void RenderAnimationComponent(entt::entity entity);
    void RenderColliderComponent(entt::entity entity);
    void RenderRigidBodyComponent(entt::entity entity);
    void RenderCameraComponent(entt::entity entity);
    void RenderTilemapComponent(entt::entity entity);
    void RenderAddComponentMenu(entt::entity entity);

    /// Helper para renderizar un header de componente con botón de eliminar
    bool RenderComponentHeader(const char* label, bool canRemove = true);

    std::shared_ptr<Scene> m_Scene;
    bool m_Visible = true;
    bool m_RequestRemove = false;
};

} // namespace Echo
