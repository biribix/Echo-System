#pragma once

#include "entt/entt.hpp"
#include <memory>

namespace Echo {

class Scene;

/// Panel de jerarquía de escena. Muestra todas las entidades en un árbol
/// y permite crear, eliminar y seleccionar entidades.
class SceneHierarchyPanel {
public:
    SceneHierarchyPanel() = default;

    void SetScene(const std::shared_ptr<Scene>& scene) { m_Scene = scene; }

    void Render();

    /// Entidad actualmente seleccionada (entt::null si ninguna)
    entt::entity GetSelectedEntity() const { return m_SelectedEntity; }
    void SetSelectedEntity(entt::entity entity) { m_SelectedEntity = entity; }
    void ClearSelection() { m_SelectedEntity = entt::null; }

    bool IsVisible() const { return m_Visible; }
    void SetVisible(bool v) { m_Visible = v; }
    void ToggleVisible() { m_Visible = !m_Visible; }

private:
    void RenderEntityNode(entt::entity entity);
    void RenderContextMenu();

    std::shared_ptr<Scene> m_Scene;
    entt::entity m_SelectedEntity = entt::null;
    bool m_Visible = true;
};

} // namespace Echo
