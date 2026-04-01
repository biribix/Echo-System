#include "echo/TilemapEditor.h"
#include "echo/Scene.h"
#include "echo/Entity.h"
#include "echo/AssetManager.h"

#include "imgui.h"
#include "rlImGui.h"
#include "raylib.h"
#include "raymath.h"

#include <algorithm>
#include <queue>

namespace Echo {

TilemapComponent* TilemapEditor::GetTargetTilemap()
{
    if (!m_Scene || m_TargetEntity == entt::null) return nullptr;
    auto& reg = m_Scene->GetRegistry();
    if (!reg.valid(m_TargetEntity) || !reg.all_of<TilemapComponent>(m_TargetEntity))
        return nullptr;
    return &reg.get<TilemapComponent>(m_TargetEntity);
}

// =============================================================================
// Panel principal
// =============================================================================

void TilemapEditor::RenderPanel()
{
    if (!m_Visible) return;

    ImGui::SetNextWindowSize(ImVec2(300, 600), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Tilemap Editor", &m_Visible)) {

        TilemapComponent* tilemap = GetTargetTilemap();

        if (!tilemap) {
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.3f, 1.0f),
                               "No hay tilemap seleccionado.");
            ImGui::TextWrapped(
                "Para crear un tilemap:\n"
                "1. Crea una entidad en el panel Escena\n"
                "2. Agrega el componente 'Tilemap'\n"
                "3. Seleccionala y vuelve aqui");

            // Botón rápido para crear tilemap
            if (m_Scene) {
                ImGui::Spacing();
                if (ImGui::Button("Crear Entidad Tilemap", ImVec2(-1, 30))) {
                    Entity e = m_Scene->CreateEntity("Tilemap");
                    auto& tm = e.Add<TilemapComponent>();
                    tm.Initialize();
                    m_TargetEntity = e.GetHandle();
                }
            }

            ImGui::End();
            return;
        }

        RenderToolbar();
        ImGui::Separator();
        RenderTilePicker();
        ImGui::Separator();
        RenderLayerManager();
        ImGui::Separator();
        RenderMapSettings();
    }
    ImGui::End();
}

// =============================================================================
// Toolbar
// =============================================================================

void TilemapEditor::RenderToolbar()
{
    ImGui::Text("Herramienta:");

    auto ToolButton = [&](const char* label, TilemapTool tool) {
        bool active = (m_CurrentTool == tool);
        if (active) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.6f, 0.9f, 1.0f));
        if (ImGui::Button(label, ImVec2(55, 28))) m_CurrentTool = tool;
        if (active) ImGui::PopStyleColor();
    };

    ToolButton("Pincel", TilemapTool::Brush);
    ImGui::SameLine();
    ToolButton("Borrar", TilemapTool::Eraser);
    ImGui::SameLine();
    ToolButton("Rellenar", TilemapTool::Fill);

    ToolButton("Rect", TilemapTool::Rect);
    ImGui::SameLine();
    ToolButton("Colision", TilemapTool::Collision);

    ImGui::SameLine();
    ImGui::Checkbox("Grid", &m_ShowGrid);
    ImGui::SameLine();
    ImGui::Checkbox("Col", &m_ShowCollision);
}

// =============================================================================
// Tile Picker — selección visual de tile desde el tileset
// =============================================================================

void TilemapEditor::RenderTilePicker()
{
    TilemapComponent* tilemap = GetTargetTilemap();
    if (!tilemap) return;

    ImGui::Text("Tile Seleccionado: %d", m_SelectedTile);

    auto& mgr = AssetManager::Instance();

    // Selector de tileset
    const auto& textures = mgr.GetAllTextures();
    if (ImGui::BeginCombo("Tileset", tilemap->tilesetPath.empty() ? "(ninguno)" : tilemap->tilesetPath.c_str())) {
        for (const auto& [path, asset] : textures) {
            if (mgr.GetTileset(path)) {  // solo mostrar texturas que son tilesets
                bool selected = (tilemap->tilesetPath == path);
                if (ImGui::Selectable(path.c_str(), selected)) {
                    tilemap->tilesetPath = path;
                    const TilesetInfo* info = mgr.GetTileset(path);
                    if (info) tilemap->tileSize = info->tileWidth;
                }
            }
        }
        ImGui::EndCombo();
    }

    // Grid de tiles del tileset
    const TilesetInfo* tilesetInfo = mgr.GetTileset(tilemap->tilesetPath);
    const TextureAsset* tex = mgr.GetTexture(tilemap->tilesetPath);

    if (tilesetInfo && tex) {
        float pickerTileSize = 32.0f;
        float availW = ImGui::GetContentRegionAvail().x;
        int pickerCols = std::max(1, (int)(availW / (pickerTileSize + 2.0f)));

        ImGui::BeginChild("TilePicker", ImVec2(0, 200), true, ImGuiWindowFlags_HorizontalScrollbar);

        int col = 0;
        for (int i = 0; i < tilesetInfo->tileCount; i++) {
            ImGui::PushID(i);

            Rectangle src = mgr.GetTileRect(tilemap->tilesetPath, i);
            ImVec2 uv0(src.x / tex->texture.width, src.y / tex->texture.height);
            ImVec2 uv1((src.x + src.width) / tex->texture.width,
                       (src.y + src.height) / tex->texture.height);

            bool selected = (m_SelectedTile == i);
            if (selected) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.3f, 1.0f));

            if (ImGui::ImageButton("##tp",
                                   reinterpret_cast<ImTextureID>(&tex->texture),
                                   ImVec2(pickerTileSize, pickerTileSize),
                                   uv0, uv1)) {
                m_SelectedTile = i;
            }

            if (selected) ImGui::PopStyleColor();

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Tile #%d", i);

            col++;
            if (col < pickerCols) ImGui::SameLine();
            else col = 0;

            ImGui::PopID();
        }

        ImGui::EndChild();
    } else {
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.3f, 1.0f),
                           "Configura un tileset en el panel Assets\ny seleccionalo arriba.");
    }
}

// =============================================================================
// Layer Manager
// =============================================================================

void TilemapEditor::RenderLayerManager()
{
    TilemapComponent* tilemap = GetTargetTilemap();
    if (!tilemap) return;

    ImGui::Text("Capas:");

    for (int i = 0; i < (int)tilemap->layers.size(); i++) {
        auto& layer = tilemap->layers[i];
        ImGui::PushID(i);

        bool isActive = (m_ActiveLayer == i);

        // Visibilidad
        ImGui::Checkbox("##vis", &layer.visible);
        ImGui::SameLine();

        // Botón de selección de capa
        if (isActive) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.5f, 0.8f, 1.0f));
        if (ImGui::Button(layer.name.c_str(), ImVec2(120, 0))) {
            m_ActiveLayer = i;
        }
        if (isActive) ImGui::PopStyleColor();

        // Mover arriba/abajo
        ImGui::SameLine();
        if (i > 0 && ImGui::SmallButton("^")) {
            std::swap(tilemap->layers[i], tilemap->layers[i - 1]);
            if (m_ActiveLayer == i) m_ActiveLayer--;
        }
        ImGui::SameLine();
        if (i < (int)tilemap->layers.size() - 1 && ImGui::SmallButton("v")) {
            std::swap(tilemap->layers[i], tilemap->layers[i + 1]);
            if (m_ActiveLayer == i) m_ActiveLayer++;
        }

        // Eliminar capa (no permitir eliminar la última)
        if (tilemap->layers.size() > 1) {
            ImGui::SameLine();
            if (ImGui::SmallButton("X")) {
                tilemap->layers.erase(tilemap->layers.begin() + i);
                if (m_ActiveLayer >= (int)tilemap->layers.size())
                    m_ActiveLayer = (int)tilemap->layers.size() - 1;
                ImGui::PopID();
                break;
            }
        }

        ImGui::PopID();
    }

    if (ImGui::Button("+ Nueva Capa")) {
        TilemapLayer newLayer;
        newLayer.name = "Capa " + std::to_string(tilemap->layers.size());
        newLayer.data.assign(tilemap->mapWidth * tilemap->mapHeight, -1);
        tilemap->layers.push_back(newLayer);
    }
}

// =============================================================================
// Map Settings
// =============================================================================

void TilemapEditor::RenderMapSettings()
{
    TilemapComponent* tilemap = GetTargetTilemap();
    if (!tilemap) return;

    if (ImGui::CollapsingHeader("Ajustes del Mapa")) {
        int prevW = tilemap->mapWidth;
        int prevH = tilemap->mapHeight;

        ImGui::DragInt("Ancho (tiles)", &tilemap->mapWidth, 1, 1, 500);
        ImGui::DragInt("Alto (tiles)", &tilemap->mapHeight, 1, 1, 500);
        ImGui::DragInt("Tam. Tile (px)", &tilemap->tileSize, 1, 8, 256);

        // Redimensionar si cambió
        if (tilemap->mapWidth != prevW || tilemap->mapHeight != prevH) {
            int newTotal = tilemap->mapWidth * tilemap->mapHeight;
            tilemap->collisionData.resize(newTotal, false);
            for (auto& layer : tilemap->layers) {
                // Preservar datos existentes en la nueva dimensión
                std::vector<int> newData(newTotal, -1);
                int copyW = std::min(prevW, tilemap->mapWidth);
                int copyH = std::min(prevH, tilemap->mapHeight);
                for (int y = 0; y < copyH; y++) {
                    for (int x = 0; x < copyW; x++) {
                        if (y * prevW + x < (int)layer.data.size())
                            newData[y * tilemap->mapWidth + x] = layer.data[y * prevW + x];
                    }
                }
                layer.data = std::move(newData);
            }
            // Lo mismo para colisión
            std::vector<bool> newCol(newTotal, false);
            int copyW = std::min(prevW, tilemap->mapWidth);
            int copyH = std::min(prevH, tilemap->mapHeight);
            for (int y = 0; y < copyH; y++) {
                for (int x = 0; x < copyW; x++) {
                    if (y * prevW + x < (int)tilemap->collisionData.size())
                        newCol[y * tilemap->mapWidth + x] = tilemap->collisionData[y * prevW + x];
                }
            }
            tilemap->collisionData = std::move(newCol);
        }

        ImGui::Text("Mapa: %d x %d tiles (%d x %d px)",
                     tilemap->mapWidth, tilemap->mapHeight,
                     tilemap->mapWidth * tilemap->tileSize,
                     tilemap->mapHeight * tilemap->tileSize);
    }
}

// =============================================================================
// Viewport — renderiza el mapa y maneja input del ratón
// =============================================================================

void TilemapEditor::RenderViewport()
{
    TilemapComponent* tilemap = GetTargetTilemap();
    if (!tilemap || !m_Visible) return;

    auto& mgr = AssetManager::Instance();
    const TextureAsset* tex = mgr.GetTexture(tilemap->tilesetPath);
    const TilesetInfo* tsInfo = mgr.GetTileset(tilemap->tilesetPath);

    // Inicializar cámara
    if (!m_CameraInitialized) {
        m_Camera.target = {0, 0};
        m_Camera.offset = {(float)GetScreenWidth() / 2.0f, (float)GetScreenHeight() / 2.0f};
        m_Camera.rotation = 0.0f;
        m_Camera.zoom = 1.0f;
        m_CameraInitialized = true;
    }

    // --- Input de cámara (solo si ImGui no captura el ratón) ---
    if (!ImGui::GetIO().WantCaptureMouse) {
        // Zoom con rueda
        float wheel = GetMouseWheelMove();
        if (wheel != 0) {
            m_Camera.zoom += wheel * 0.1f;
            m_Camera.zoom = Clamp(m_Camera.zoom, 0.1f, 5.0f);
        }

        // Pan con click medio o derecho
        if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE) || IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
            Vector2 delta = GetMouseDelta();
            m_Camera.target.x -= delta.x / m_Camera.zoom;
            m_Camera.target.y -= delta.y / m_Camera.zoom;
        }

        // Posición del ratón en el mundo
        Vector2 mouseScreen = GetMousePosition();
        Vector2 mouseWorld = GetScreenToWorld2D(mouseScreen, m_Camera);

        int gridX = (int)floorf(mouseWorld.x / tilemap->tileSize);
        int gridY = (int)floorf(mouseWorld.y / tilemap->tileSize);

        // Pintar con click izquierdo
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            HandleMousePaint(*tilemap, gridX, gridY);
        }

        // Herramienta rectángulo
        if (m_CurrentTool == TilemapTool::Rect) {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                m_IsDrawingRect = true;
                m_RectStartX = gridX;
                m_RectStartY = gridY;
            }
            if (m_IsDrawingRect) {
                m_RectEndX = gridX;
                m_RectEndY = gridY;
            }
            if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && m_IsDrawingRect) {
                m_IsDrawingRect = false;
                int x0 = std::min(m_RectStartX, m_RectEndX);
                int y0 = std::min(m_RectStartY, m_RectEndY);
                int x1 = std::max(m_RectStartX, m_RectEndX);
                int y1 = std::max(m_RectStartY, m_RectEndY);
                for (int y = y0; y <= y1; y++)
                    for (int x = x0; x <= x1; x++)
                        tilemap->SetTile(m_ActiveLayer, x, y, m_SelectedTile);
            }
        }
    }

    // --- Renderizado con Raylib ---
    BeginMode2D(m_Camera);

    int ts = tilemap->tileSize;

    // Dibujar cada capa
    for (int layerIdx = 0; layerIdx < (int)tilemap->layers.size(); layerIdx++) {
        auto& layer = tilemap->layers[layerIdx];
        if (!layer.visible) continue;

        for (int y = 0; y < tilemap->mapHeight; y++) {
            for (int x = 0; x < tilemap->mapWidth; x++) {
                int tileIdx = layer.data[y * tilemap->mapWidth + x];
                if (tileIdx < 0) continue;

                if (tex && tsInfo && tileIdx < tsInfo->tileCount) {
                    Rectangle src = mgr.GetTileRect(tilemap->tilesetPath, tileIdx);
                    Rectangle dst = {(float)(x * ts), (float)(y * ts), (float)ts, (float)ts};
                    DrawTexturePro(tex->texture, src, dst, {0, 0}, 0.0f, WHITE);
                } else {
                    // Fallback: cuadrado de color si no hay textura
                    DrawRectangle(x * ts, y * ts, ts, ts,
                                  Color{100, 100, 200, 180});
                }
            }
        }
    }

    // Dibujar grid
    if (m_ShowGrid) {
        for (int x = 0; x <= tilemap->mapWidth; x++) {
            DrawLine(x * ts, 0, x * ts, tilemap->mapHeight * ts,
                     Color{255, 255, 255, 40});
        }
        for (int y = 0; y <= tilemap->mapHeight; y++) {
            DrawLine(0, y * ts, tilemap->mapWidth * ts, y * ts,
                     Color{255, 255, 255, 40});
        }
    }

    // Dibujar colisiones
    if (m_ShowCollision) {
        for (int y = 0; y < tilemap->mapHeight; y++) {
            for (int x = 0; x < tilemap->mapWidth; x++) {
                if (tilemap->GetCollision(x, y)) {
                    DrawRectangle(x * ts + 2, y * ts + 2, ts - 4, ts - 4,
                                  Color{255, 0, 0, 60});
                    DrawRectangleLines(x * ts, y * ts, ts, ts,
                                       Color{255, 50, 50, 150});
                }
            }
        }
    }

    // Preview del rectángulo en curso
    if (m_IsDrawingRect && m_CurrentTool == TilemapTool::Rect) {
        int x0 = std::min(m_RectStartX, m_RectEndX);
        int y0 = std::min(m_RectStartY, m_RectEndY);
        int x1 = std::max(m_RectStartX, m_RectEndX);
        int y1 = std::max(m_RectStartY, m_RectEndY);
        DrawRectangle(x0 * ts, y0 * ts, (x1 - x0 + 1) * ts, (y1 - y0 + 1) * ts,
                      Color{100, 200, 100, 60});
        DrawRectangleLines(x0 * ts, y0 * ts, (x1 - x0 + 1) * ts, (y1 - y0 + 1) * ts,
                           Color{100, 255, 100, 200});
    }

    // Contorno del mapa
    DrawRectangleLines(0, 0, tilemap->mapWidth * ts, tilemap->mapHeight * ts,
                       Color{255, 255, 0, 100});

    // Cursor: highlight de la celda bajo el ratón
    if (!ImGui::GetIO().WantCaptureMouse) {
        Vector2 mouseWorld = GetScreenToWorld2D(GetMousePosition(), m_Camera);
        int gx = (int)floorf(mouseWorld.x / ts);
        int gy = (int)floorf(mouseWorld.y / ts);
        if (gx >= 0 && gx < tilemap->mapWidth && gy >= 0 && gy < tilemap->mapHeight) {
            DrawRectangleLines(gx * ts, gy * ts, ts, ts, YELLOW);
        }
    }

    EndMode2D();
}

// =============================================================================
// Lógica de pintura
// =============================================================================

void TilemapEditor::HandleMousePaint(TilemapComponent& tilemap, int gx, int gy)
{
    if (gx < 0 || gx >= tilemap.mapWidth || gy < 0 || gy >= tilemap.mapHeight)
        return;

    switch (m_CurrentTool) {
    case TilemapTool::Brush:
        tilemap.SetTile(m_ActiveLayer, gx, gy, m_SelectedTile);
        break;

    case TilemapTool::Eraser:
        tilemap.SetTile(m_ActiveLayer, gx, gy, -1);
        break;

    case TilemapTool::Fill:
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            int target = tilemap.GetTile(m_ActiveLayer, gx, gy);
            if (target != m_SelectedTile)
                FloodFill(tilemap, m_ActiveLayer, gx, gy, target, m_SelectedTile);
        }
        break;

    case TilemapTool::Collision:
        tilemap.SetCollision(gx, gy, !IsKeyDown(KEY_LEFT_SHIFT));
        break;

    case TilemapTool::Rect:
        // Manejado en RenderViewport
        break;
    }
}

void TilemapEditor::FloodFill(TilemapComponent& tilemap, int layerIdx,
                               int x, int y, int targetTile, int newTile)
{
    if (x < 0 || x >= tilemap.mapWidth || y < 0 || y >= tilemap.mapHeight)
        return;
    if (tilemap.GetTile(layerIdx, x, y) != targetTile)
        return;

    // BFS para evitar stack overflow en mapas grandes
    std::queue<std::pair<int,int>> queue;
    queue.push({x, y});
    tilemap.SetTile(layerIdx, x, y, newTile);

    const int dx[] = {0, 0, -1, 1};
    const int dy[] = {-1, 1, 0, 0};

    while (!queue.empty()) {
        auto [cx, cy] = queue.front();
        queue.pop();

        for (int d = 0; d < 4; d++) {
            int nx = cx + dx[d];
            int ny = cy + dy[d];
            if (nx >= 0 && nx < tilemap.mapWidth && ny >= 0 && ny < tilemap.mapHeight) {
                if (tilemap.GetTile(layerIdx, nx, ny) == targetTile) {
                    tilemap.SetTile(layerIdx, nx, ny, newTile);
                    queue.push({nx, ny});
                }
            }
        }
    }
}

} // namespace Echo
