#include "echo/PropertiesPanel.h"
#include "echo/Scene.h"
#include "echo/Entity.h"
#include "echo/Components.h"
#include "echo/AssetManager.h"

#include "imgui.h"
#include "rlImGui.h"

namespace Echo {

// =============================================================================
// Helper: header de componente colapsable con opción de eliminar
// =============================================================================

bool PropertiesPanel::RenderComponentHeader(const char* label, bool canRemove)
{
    m_RequestRemove = false;

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen
                             | ImGuiTreeNodeFlags_Framed
                             | ImGuiTreeNodeFlags_AllowItemOverlap
                             | ImGuiTreeNodeFlags_SpanAvailWidth;

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));
    bool open = ImGui::TreeNodeEx(label, flags);
    ImGui::PopStyleVar();

    if (canRemove) {
        ImGui::SameLine(ImGui::GetWindowWidth() - 30);
        if (ImGui::SmallButton("X")) {
            m_RequestRemove = true;
        }
    }

    return open;
}

// =============================================================================
// Render principal
// =============================================================================

void PropertiesPanel::Render(entt::entity selectedEntity)
{
    if (!m_Visible || !m_Scene) return;

    ImGui::SetNextWindowSize(ImVec2(320, 500), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Propiedades", &m_Visible)) {

        if (selectedEntity == entt::null || !m_Scene->GetRegistry().valid(selectedEntity)) {
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f),
                               "Selecciona una entidad en el panel Escena");
            ImGui::End();
            return;
        }

        RenderTagComponent(selectedEntity);
        RenderTransformComponent(selectedEntity);
        RenderSpriteRendererComponent(selectedEntity);
        RenderAnimationComponent(selectedEntity);
        RenderColliderComponent(selectedEntity);
        RenderRigidBodyComponent(selectedEntity);
        RenderCameraComponent(selectedEntity);

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        RenderAddComponentMenu(selectedEntity);
    }
    ImGui::End();
}

// =============================================================================
// Tag
// =============================================================================

void PropertiesPanel::RenderTagComponent(entt::entity entity)
{
    auto& reg = m_Scene->GetRegistry();
    if (!reg.all_of<TagComponent>(entity)) return;

    auto& tag = reg.get<TagComponent>(entity);

    char buffer[256];
    snprintf(buffer, sizeof(buffer), "%s", tag.name.c_str());

    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    if (ImGui::InputText("##Name", buffer, sizeof(buffer))) {
        tag.name = buffer;
    }

    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "ID: %llu", tag.uuid);
    ImGui::Separator();
}

// =============================================================================
// Transform
// =============================================================================

void PropertiesPanel::RenderTransformComponent(entt::entity entity)
{
    auto& reg = m_Scene->GetRegistry();
    if (!reg.all_of<TransformComponent>(entity)) return;

    if (RenderComponentHeader("Transform", false)) {
        auto& tc = reg.get<TransformComponent>(entity);

        ImGui::DragFloat2("Posicion", tc.position, 1.0f);
        ImGui::DragFloat("Rotacion", &tc.rotation, 1.0f, -360.0f, 360.0f, "%.1f deg");
        ImGui::DragFloat2("Escala", tc.scale, 0.01f, 0.01f, 100.0f);

        ImGui::TreePop();
    }
}

// =============================================================================
// Sprite Renderer
// =============================================================================

void PropertiesPanel::RenderSpriteRendererComponent(entt::entity entity)
{
    auto& reg = m_Scene->GetRegistry();
    if (!reg.all_of<SpriteRendererComponent>(entity)) return;

    if (RenderComponentHeader("Sprite Renderer")) {
        auto& sr = reg.get<SpriteRendererComponent>(entity);

        // Selector de textura (lista desplegable con assets cargados)
        auto& mgr = AssetManager::Instance();
        const auto& textures = mgr.GetAllTextures();

        if (ImGui::BeginCombo("Textura", sr.texturePath.empty() ? "(ninguna)" : sr.texturePath.c_str())) {
            // Opción vacía
            if (ImGui::Selectable("(ninguna)", sr.texturePath.empty())) {
                sr.texturePath.clear();
                sr.tilesetIndex = -1;
            }
            for (const auto& [path, asset] : textures) {
                bool selected = (sr.texturePath == path);
                if (ImGui::Selectable(path.c_str(), selected)) {
                    sr.texturePath = path;
                    sr.tilesetIndex = -1;
                }
                if (selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        // Preview de la textura seleccionada
        if (!sr.texturePath.empty()) {
            const TextureAsset* tex = mgr.GetTexture(sr.texturePath);
            if (tex) {
                float previewSize = 64.0f;
                float scale = std::min(previewSize / tex->texture.width,
                                       previewSize / tex->texture.height);
                rlImGuiImageSize(&tex->texture,
                                 static_cast<int>(tex->texture.width * scale),
                                 static_cast<int>(tex->texture.height * scale));

                // Si es un tileset, mostrar selector de tile
                const TilesetInfo* tileset = mgr.GetTileset(sr.texturePath);
                if (tileset) {
                    ImGui::SliderInt("Tile Index", &sr.tilesetIndex, -1, tileset->tileCount - 1);
                    if (sr.tilesetIndex == -1) {
                        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "(textura completa)");
                    }
                }
            }
        }

        ImGui::ColorEdit4("Color", sr.color);
        ImGui::Checkbox("Flip X", &sr.flipX);
        ImGui::SameLine();
        ImGui::Checkbox("Flip Y", &sr.flipY);
        ImGui::DragInt("Capa", &sr.layer, 1.0f, -100, 100);

        if (m_RequestRemove) {
            reg.remove<SpriteRendererComponent>(entity);
        }

        ImGui::TreePop();
    }
}

// =============================================================================
// Animation
// =============================================================================

void PropertiesPanel::RenderAnimationComponent(entt::entity entity)
{
    auto& reg = m_Scene->GetRegistry();
    if (!reg.all_of<AnimationComponent>(entity)) return;

    if (RenderComponentHeader("Animacion")) {
        auto& anim = reg.get<AnimationComponent>(entity);

        ImGui::Checkbox("Reproduciendo", &anim.playing);

        // Lista de clips
        for (int i = 0; i < static_cast<int>(anim.clips.size()); i++) {
            auto& clip = anim.clips[i];
            ImGui::PushID(i);

            bool isCurrentClip = (anim.currentClip == i);
            if (isCurrentClip)
                ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.3f, 0.5f, 0.8f, 0.5f));

            char clipLabel[128];
            snprintf(clipLabel, sizeof(clipLabel), "Clip: %s (%d frames)###clip",
                     clip.name.c_str(), (int)clip.frames.size());

            if (ImGui::CollapsingHeader(clipLabel)) {
                char nameBuf[128];
                snprintf(nameBuf, sizeof(nameBuf), "%s", clip.name.c_str());
                if (ImGui::InputText("Nombre", nameBuf, sizeof(nameBuf)))
                    clip.name = nameBuf;

                ImGui::DragFloat("Velocidad", &clip.speed, 0.01f, 0.01f, 10.0f);
                ImGui::Checkbox("Loop", &clip.loop);

                if (ImGui::Button("Activar Clip")) {
                    anim.currentClip = i;
                    anim.currentFrame = 0;
                    anim.timer = 0.0f;
                }
                ImGui::SameLine();
                if (ImGui::Button("+ Frame")) {
                    clip.frames.push_back({});
                }

                // Lista de frames
                for (int f = 0; f < static_cast<int>(clip.frames.size()); f++) {
                    ImGui::PushID(f);
                    auto& frame = clip.frames[f];
                    ImGui::Text("Frame %d:", f);
                    ImGui::SameLine();
                    ImGui::SetNextItemWidth(60);
                    ImGui::DragInt("Tile", &frame.tileIndex, 1, 0, 999);
                    ImGui::SameLine();
                    ImGui::SetNextItemWidth(60);
                    ImGui::DragFloat("Dur", &frame.duration, 0.01f, 0.01f, 5.0f, "%.2fs");
                    ImGui::PopID();
                }
            }

            if (isCurrentClip)
                ImGui::PopStyleColor();

            ImGui::PopID();
        }

        if (ImGui::Button("+ Nuevo Clip")) {
            AnimationClip newClip;
            newClip.name = "Clip " + std::to_string(anim.clips.size());
            anim.clips.push_back(newClip);
        }

        if (m_RequestRemove) {
            reg.remove<AnimationComponent>(entity);
        }

        ImGui::TreePop();
    }
}

// =============================================================================
// Collider
// =============================================================================

void PropertiesPanel::RenderColliderComponent(entt::entity entity)
{
    auto& reg = m_Scene->GetRegistry();
    if (!reg.all_of<ColliderComponent>(entity)) return;

    if (RenderComponentHeader("Collider")) {
        auto& col = reg.get<ColliderComponent>(entity);

        const char* typeNames[] = {"Box", "Circle"};
        int currentType = static_cast<int>(col.type);
        if (ImGui::Combo("Tipo", &currentType, typeNames, 2)) {
            col.type = static_cast<ColliderType>(currentType);
        }

        ImGui::DragFloat2("Offset", col.offset, 0.5f);

        if (col.type == ColliderType::Box) {
            ImGui::DragFloat2("Tamano", col.size, 0.5f, 1.0f, 9999.0f);
        } else {
            ImGui::DragFloat("Radio", &col.size[0], 0.5f, 1.0f, 9999.0f);
        }

        ImGui::Checkbox("Es Trigger", &col.isTrigger);

        if (m_RequestRemove) {
            reg.remove<ColliderComponent>(entity);
        }

        ImGui::TreePop();
    }
}

// =============================================================================
// RigidBody
// =============================================================================

void PropertiesPanel::RenderRigidBodyComponent(entt::entity entity)
{
    auto& reg = m_Scene->GetRegistry();
    if (!reg.all_of<RigidBodyComponent>(entity)) return;

    if (RenderComponentHeader("RigidBody")) {
        auto& rb = reg.get<RigidBodyComponent>(entity);

        const char* bodyTypes[] = {"Static", "Dynamic", "Kinematic"};
        int currentType = static_cast<int>(rb.type);
        if (ImGui::Combo("Tipo", &currentType, bodyTypes, 3)) {
            rb.type = static_cast<BodyType>(currentType);
        }

        if (rb.type != BodyType::Static) {
            ImGui::DragFloat2("Velocidad", rb.velocity, 0.5f);
            ImGui::DragFloat("Gravedad", &rb.gravityScale, 0.01f, 0.0f, 10.0f);
            ImGui::DragFloat("Masa", &rb.mass, 0.1f, 0.01f, 1000.0f);
            ImGui::Checkbox("Rot. Fija", &rb.fixedRotation);
        }

        if (m_RequestRemove) {
            reg.remove<RigidBodyComponent>(entity);
        }

        ImGui::TreePop();
    }
}

// =============================================================================
// Camera
// =============================================================================

void PropertiesPanel::RenderCameraComponent(entt::entity entity)
{
    auto& reg = m_Scene->GetRegistry();
    if (!reg.all_of<CameraComponent>(entity)) return;

    if (RenderComponentHeader("Camara")) {
        auto& cam = reg.get<CameraComponent>(entity);

        ImGui::Checkbox("Principal", &cam.primary);
        ImGui::DragFloat("Zoom", &cam.zoom, 0.01f, 0.1f, 10.0f);

        if (m_RequestRemove) {
            reg.remove<CameraComponent>(entity);
        }

        ImGui::TreePop();
    }
}

// =============================================================================
// Menú "Agregar Componente"
// =============================================================================

void PropertiesPanel::RenderAddComponentMenu(entt::entity entity)
{
    auto& reg = m_Scene->GetRegistry();

    float buttonWidth = ImGui::GetContentRegionAvail().x;
    if (ImGui::Button("+ Agregar Componente", ImVec2(buttonWidth, 30))) {
        ImGui::OpenPopup("AddComponentPopup");
    }

    if (ImGui::BeginPopup("AddComponentPopup")) {

        if (!reg.all_of<SpriteRendererComponent>(entity)) {
            if (ImGui::MenuItem("Sprite Renderer")) {
                reg.emplace<SpriteRendererComponent>(entity);
                ImGui::CloseCurrentPopup();
            }
        }
        if (!reg.all_of<AnimationComponent>(entity)) {
            if (ImGui::MenuItem("Animacion")) {
                reg.emplace<AnimationComponent>(entity);
                ImGui::CloseCurrentPopup();
            }
        }
        if (!reg.all_of<ColliderComponent>(entity)) {
            if (ImGui::MenuItem("Collider")) {
                reg.emplace<ColliderComponent>(entity);
                ImGui::CloseCurrentPopup();
            }
        }
        if (!reg.all_of<RigidBodyComponent>(entity)) {
            if (ImGui::MenuItem("RigidBody")) {
                reg.emplace<RigidBodyComponent>(entity);
                ImGui::CloseCurrentPopup();
            }
        }
        if (!reg.all_of<CameraComponent>(entity)) {
            if (ImGui::MenuItem("Camara")) {
                reg.emplace<CameraComponent>(entity);
                ImGui::CloseCurrentPopup();
            }
        }

        ImGui::EndPopup();
    }
}

} // namespace Echo
