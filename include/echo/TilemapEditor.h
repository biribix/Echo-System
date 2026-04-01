#pragma once

#include "echo/Components.h"
#include "entt/entt.hpp"
#include "raylib.h"
#include <memory>
#include <string>

namespace Echo {

class Scene;

/// Herramientas de pintura del tilemap
enum class TilemapTool {
    Brush,      // Pincel: pinta un tile
    Eraser,     // Borrador: pone -1
    Fill,       // Relleno (flood fill)
    Rect,       // Rectángulo: pinta un área rectangular
    Collision   // Pintar/borrar colisiones
};

/// Editor visual de Tilemaps.
/// Panel ImGui con: tile picker, toolbar, layer manager y viewport interactivo.
class TilemapEditor {
public:
    TilemapEditor() = default;

    void SetScene(const std::shared_ptr<Scene>& scene) { m_Scene = scene; }

    /// Panel principal del editor (ImGui)
    void RenderPanel();

    /// Renderiza el tilemap en el viewport de Raylib (fuera de ImGui)
    void RenderViewport();

    bool IsVisible() const { return m_Visible; }
    void SetVisible(bool v) { m_Visible = v; }
    void ToggleVisible() { m_Visible = !m_Visible; }

    /// Establece qué entidad-tilemap editar
    void SetTargetEntity(entt::entity entity) { m_TargetEntity = entity; }
    entt::entity GetTargetEntity() const { return m_TargetEntity; }

private:
    // Sub-paneles
    void RenderToolbar();
    void RenderTilePicker();
    void RenderLayerManager();
    void RenderMapSettings();
    void RenderViewportCanvas();

    // Lógica de pintura
    void HandleMousePaint(TilemapComponent& tilemap, int mouseGridX, int mouseGridY);
    void FloodFill(TilemapComponent& tilemap, int layerIdx, int x, int y, int targetTile, int newTile);

    // Obtiene el TilemapComponent del target (o nullptr)
    TilemapComponent* GetTargetTilemap();

    std::shared_ptr<Scene> m_Scene;
    entt::entity m_TargetEntity = entt::null;
    bool m_Visible = true;

    // Estado de herramienta
    TilemapTool m_CurrentTool = TilemapTool::Brush;
    int m_SelectedTile = 0;
    int m_ActiveLayer = 0;
    bool m_ShowGrid = true;
    bool m_ShowCollision = true;

    // Cámara del viewport del editor
    Camera2D m_Camera = {};
    bool m_CameraInitialized = false;
    bool m_IsDraggingCamera = false;
    Vector2 m_DragStart = {};

    // Rectángulo drag
    bool m_IsDrawingRect = false;
    int m_RectStartX = 0, m_RectStartY = 0;
    int m_RectEndX = 0, m_RectEndY = 0;
};

} // namespace Echo
