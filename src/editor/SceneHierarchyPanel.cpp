#include "echo/SceneHierarchyPanel.h"
#include "echo/Scene.h"
#include "echo/Entity.h"
#include "echo/Components.h"

#include "imgui.h"

namespace Echo {

void SceneHierarchyPanel::Render()
{
    if (!m_Visible || !m_Scene) return;

    ImGui::SetNextWindowSize(ImVec2(280, 400), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Escena", &m_Visible)) {

        // Header con nombre de escena y contador
        ImGui::Text("Escena: %s", m_Scene->GetName().c_str());
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "(%d entidades)",
                           (int)m_Scene->GetEntityCount());
        ImGui::Separator();

        // Botón para crear entidad rápido
        if (ImGui::Button("+ Crear Entidad")) {
            m_Scene->CreateEntity("Nueva Entidad");
        }

        ImGui::Separator();

        // Listar todas las entidades que tengan TagComponent
        auto& reg = m_Scene->GetRegistry();
        auto view = reg.view<TagComponent>();

        for (auto entity : view) {
            RenderEntityNode(entity);
        }

        // Click en espacio vacío = deseleccionar
        if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered()) {
            m_SelectedEntity = entt::null;
        }

        // Menú contextual (click derecho en espacio vacío)
        if (ImGui::BeginPopupContextWindow("SceneContextMenu", ImGuiPopupFlags_NoOpenOverItems | ImGuiPopupFlags_MouseButtonRight)) {
            RenderContextMenu();
            ImGui::EndPopup();
        }
    }
    ImGui::End();
}

void SceneHierarchyPanel::RenderEntityNode(entt::entity entity)
{
    auto& reg = m_Scene->GetRegistry();
    auto& tag = reg.get<TagComponent>(entity);

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow
                             | ImGuiTreeNodeFlags_SpanAvailWidth
                             | ImGuiTreeNodeFlags_Leaf;

    if (m_SelectedEntity == entity)
        flags |= ImGuiTreeNodeFlags_Selected;

    // Icono según componentes que tenga
    const char* icon = "  ";
    if (reg.all_of<CameraComponent>(entity))
        icon = "[C]";
    else if (reg.all_of<SpriteRendererComponent>(entity))
        icon = "[S]";
    else if (reg.all_of<ColliderComponent>(entity))
        icon = "[H]";

    // ID único para ImGui basado en el entity handle
    ImGui::PushID(static_cast<int>(entt::to_integral(entity)));

    std::string label = std::string(icon) + " " + tag.name;
    bool opened = ImGui::TreeNodeEx(label.c_str(), flags);

    // Selección por click
    if (ImGui::IsItemClicked()) {
        m_SelectedEntity = entity;
    }

    // Menú contextual por entidad (click derecho)
    if (ImGui::BeginPopupContextItem()) {
        m_SelectedEntity = entity;

        if (ImGui::MenuItem("Duplicar")) {
            // Crear nueva entidad con mismo nombre
            Entity src(entity, m_Scene.get());
            Entity dup = m_Scene->CreateEntity(tag.name + " (copia)");

            // Copiar TransformComponent
            if (src.Has<TransformComponent>()) {
                auto& srcT = src.Get<TransformComponent>();
                auto& dstT = dup.Get<TransformComponent>();
                dstT = srcT;
                dstT.position[0] += 32.0f; // offset para que no quede encima
            }
            // Copiar SpriteRendererComponent
            if (src.Has<SpriteRendererComponent>()) {
                dup.Add<SpriteRendererComponent>() = src.Get<SpriteRendererComponent>();
            }
            // Copiar ColliderComponent
            if (src.Has<ColliderComponent>()) {
                dup.Add<ColliderComponent>() = src.Get<ColliderComponent>();
            }
            // Copiar RigidBodyComponent
            if (src.Has<RigidBodyComponent>()) {
                dup.Add<RigidBodyComponent>() = src.Get<RigidBodyComponent>();
            }
        }

        if (ImGui::MenuItem("Eliminar")) {
            m_Scene->DestroyEntity(Entity(entity, m_Scene.get()));
            m_SelectedEntity = entt::null;
            ImGui::EndPopup();
            ImGui::PopID();
            if (opened) ImGui::TreePop();
            return;
        }

        ImGui::EndPopup();
    }

    // Drag & Drop (fuente — para futuro reordenamiento)
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
        uint32_t entityId = entt::to_integral(entity);
        ImGui::SetDragDropPayload("ENTITY", &entityId, sizeof(uint32_t));
        ImGui::Text("%s", tag.name.c_str());
        ImGui::EndDragDropSource();
    }

    if (opened)
        ImGui::TreePop();

    ImGui::PopID();
}

void SceneHierarchyPanel::RenderContextMenu()
{
    if (ImGui::BeginMenu("Crear...")) {
        if (ImGui::MenuItem("Entidad Vacia")) {
            auto e = m_Scene->CreateEntity("Entidad Vacia");
            m_SelectedEntity = e.GetHandle();
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Sprite")) {
            auto e = m_Scene->CreateEntity("Sprite");
            Entity ent(e.GetHandle(), m_Scene.get());
            ent.Add<SpriteRendererComponent>();
            m_SelectedEntity = e.GetHandle();
        }
        if (ImGui::MenuItem("Jugador")) {
            auto e = m_Scene->CreateEntity("Jugador");
            Entity ent(e.GetHandle(), m_Scene.get());
            ent.Add<SpriteRendererComponent>();
            ent.Add<ColliderComponent>();
            auto& rb = ent.Add<RigidBodyComponent>();
            rb.type = BodyType::Dynamic;
            m_SelectedEntity = e.GetHandle();
        }
        if (ImGui::MenuItem("Enemigo")) {
            auto e = m_Scene->CreateEntity("Enemigo");
            Entity ent(e.GetHandle(), m_Scene.get());
            ent.Add<SpriteRendererComponent>();
            ent.Add<ColliderComponent>();
            auto& rb = ent.Add<RigidBodyComponent>();
            rb.type = BodyType::Kinematic;
            m_SelectedEntity = e.GetHandle();
        }
        if (ImGui::MenuItem("Pared / Suelo")) {
            auto e = m_Scene->CreateEntity("Pared");
            Entity ent(e.GetHandle(), m_Scene.get());
            ent.Add<ColliderComponent>();
            ent.Add<RigidBodyComponent>(); // Static por defecto
            m_SelectedEntity = e.GetHandle();
        }
        if (ImGui::MenuItem("Camara")) {
            auto e = m_Scene->CreateEntity("Camara");
            Entity ent(e.GetHandle(), m_Scene.get());
            ent.Add<CameraComponent>();
            m_SelectedEntity = e.GetHandle();
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Tilemap")) {
            auto e = m_Scene->CreateEntity("Tilemap");
            Entity ent(e.GetHandle(), m_Scene.get());
            auto& tm = ent.Add<TilemapComponent>();
            tm.Initialize();
            m_SelectedEntity = e.GetHandle();
        }
        ImGui::EndMenu();
    }
}

} // namespace Echo
