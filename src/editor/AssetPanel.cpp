#include "echo/AssetPanel.h"
#include "echo/AssetManager.h"

#include "raylib.h"
#include "imgui.h"
#include "rlImGui.h"

#include <filesystem>
#include <algorithm>

namespace fs = std::filesystem;

namespace Echo {

AssetPanel::AssetPanel()
{
    m_RootNode.name = "assets";
    m_RootNode.fullPath = "assets";
    m_SelectedFolder = "assets";
}

// =============================================================================
// Render principal
// =============================================================================

void AssetPanel::Render()
{
    if (!m_Visible) return;

    if (m_NeedsRefresh) {
        RefreshFileTree();
        m_NeedsRefresh = false;
    }

    ImGui::SetNextWindowSize(ImVec2(800, 400), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Assets", &m_Visible)) {
        RenderToolbar();
        ImGui::Separator();

        // Layout: Árbol izquierda | Grid centro | Preview derecha
        float panelWidth = ImGui::GetContentRegionAvail().x;
        float treeWidth = 200.0f;
        float previewWidth = m_SelectedAsset.empty() ? 0.0f : 250.0f;
        float gridWidth = panelWidth - treeWidth - previewWidth - 16.0f;

        // --- Columna izquierda: Árbol de carpetas ---
        ImGui::BeginChild("FolderTree", ImVec2(treeWidth, 0), true);
        RenderFolderTree();
        ImGui::EndChild();

        ImGui::SameLine();

        // --- Columna central: Grid de thumbnails ---
        ImGui::BeginChild("AssetGrid", ImVec2(gridWidth, 0), true);
        RenderAssetGrid();
        ImGui::EndChild();

        // --- Columna derecha: Preview ---
        if (!m_SelectedAsset.empty()) {
            ImGui::SameLine();
            ImGui::BeginChild("Preview", ImVec2(previewWidth, 0), true);
            RenderPreviewPanel();
            ImGui::EndChild();
        }
    }
    ImGui::End();
}

// =============================================================================
// Toolbar
// =============================================================================

void AssetPanel::RenderToolbar()
{
    if (ImGui::Button("Importar Asset")) {
        ImportAsset();
    }
    ImGui::SameLine();
    if (ImGui::Button("Refrescar")) {
        m_NeedsRefresh = true;
        AssetManager::Instance().ScanAndLoadAll();
    }
    ImGui::SameLine();
    ImGui::SetNextItemWidth(100);
    ImGui::SliderFloat("Tam.", &m_ThumbnailSize, 32.0f, 128.0f, "%.0f px");
}

// =============================================================================
// Árbol de carpetas
// =============================================================================

void AssetPanel::RefreshFileTree()
{
    m_RootNode.children.clear();
    m_RootNode.files.clear();
    BuildFolderTree(AssetManager::Instance().GetAssetsRoot(), m_RootNode);
}

void AssetPanel::BuildFolderTree(const std::string& dirPath, FolderNode& node)
{
    if (!fs::exists(dirPath) || !fs::is_directory(dirPath))
        return;

    for (const auto& entry : fs::directory_iterator(dirPath)) {
        if (entry.is_directory()) {
            FolderNode child;
            child.name = entry.path().filename().string();
            child.fullPath = entry.path().generic_string();
            BuildFolderTree(child.fullPath, child);
            node.children.push_back(std::move(child));
        } else if (entry.is_regular_file()) {
            fs::path rel = fs::relative(entry.path(), AssetManager::Instance().GetAssetsRoot());
            node.files.push_back(rel.generic_string());
        }
    }

    // Ordenar alfabéticamente
    std::sort(node.children.begin(), node.children.end(),
              [](const FolderNode& a, const FolderNode& b) { return a.name < b.name; });
    std::sort(node.files.begin(), node.files.end());
}

void AssetPanel::RenderFolderTree()
{
    RenderFolderNode(m_RootNode);
}

void AssetPanel::RenderFolderNode(FolderNode& node)
{
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
    if (node.children.empty())
        flags |= ImGuiTreeNodeFlags_Leaf;
    if (node.fullPath == m_SelectedFolder)
        flags |= ImGuiTreeNodeFlags_Selected;

    bool open = ImGui::TreeNodeEx(node.name.c_str(), flags);

    if (ImGui::IsItemClicked())
        m_SelectedFolder = node.fullPath;

    if (open) {
        for (auto& child : node.children)
            RenderFolderNode(child);
        ImGui::TreePop();
    }
}

// =============================================================================
// Grid de thumbnails
// =============================================================================

void AssetPanel::RenderAssetGrid()
{
    auto& mgr = AssetManager::Instance();
    const auto& textures = mgr.GetAllTextures();

    float cellSize = m_ThumbnailSize + 8.0f;
    float panelWidth = ImGui::GetContentRegionAvail().x;
    int columns = std::max(1, static_cast<int>(panelWidth / cellSize));

    int col = 0;
    for (const auto& [path, asset] : textures) {
        // Filtrar por carpeta seleccionada si hay alguna
        if (!m_SelectedFolder.empty() && m_SelectedFolder != "assets") {
            fs::path assetFull = fs::path(mgr.GetAssetsRoot()) / path;
            fs::path selFolder(m_SelectedFolder);
            // Verificar que el asset está dentro de la carpeta seleccionada
            auto rel = fs::relative(assetFull, selFolder);
            std::string relStr = rel.generic_string();
            if (relStr.find("..") == 0)
                continue;
        }

        ImGui::PushID(path.c_str());

        // Botón con imagen
        bool selected = (m_SelectedAsset == path);
        if (selected) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.5f, 0.8f, 1.0f));
        }

        // rlImGui convierte Texture2D a ImTextureID
        ImGui::BeginGroup();
        if (ImGui::ImageButton("##thumb",
                               reinterpret_cast<ImTextureID>(&asset.texture),
                               ImVec2(m_ThumbnailSize, m_ThumbnailSize))) {
            m_SelectedAsset = path;
        }

        // Nombre truncado debajo
        std::string displayName = asset.name;
        float textWidth = ImGui::CalcTextSize(displayName.c_str()).x;
        if (textWidth > m_ThumbnailSize) {
            // Truncar con "..."
            while (displayName.size() > 3 &&
                   ImGui::CalcTextSize((displayName + "...").c_str()).x > m_ThumbnailSize) {
                displayName.pop_back();
            }
            displayName += "...";
        }
        ImGui::TextUnformatted(displayName.c_str());
        ImGui::EndGroup();

        if (selected) {
            ImGui::PopStyleColor();
        }

        col++;
        if (col < columns)
            ImGui::SameLine();
        else
            col = 0;

        ImGui::PopID();
    }

    if (textures.empty()) {
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f),
                           "No hay assets cargados.\nUsa 'Importar Asset' o coloca archivos en la carpeta assets/");
    }
}

// =============================================================================
// Panel de Preview
// =============================================================================

void AssetPanel::RenderPreviewPanel()
{
    ImGui::Text("Preview");
    ImGui::Separator();

    auto& mgr = AssetManager::Instance();
    const TextureAsset* tex = mgr.GetTexture(m_SelectedAsset);

    if (tex) {
        ImGui::Text("Nombre: %s", tex->name.c_str());
        ImGui::Text("Path: %s", tex->relativePath.c_str());
        ImGui::Text("Tamano: %d x %d", tex->texture.width, tex->texture.height);
        ImGui::Separator();

        // Preview escalada para caber en el panel
        float availWidth = ImGui::GetContentRegionAvail().x;
        float scale = std::min(1.0f, availWidth / static_cast<float>(tex->texture.width));
        ImVec2 previewSize(tex->texture.width * scale, tex->texture.height * scale);

        rlImGuiImageSize(&tex->texture, static_cast<int>(previewSize.x), static_cast<int>(previewSize.y));

        ImGui::Spacing();
        RenderTilesetConfig();
    } else {
        // Podría ser un audio
        const SoundAsset* snd = mgr.GetSound(m_SelectedAsset);
        if (snd) {
            ImGui::Text("Nombre: %s", snd->name.c_str());
            ImGui::Text("Path: %s", snd->relativePath.c_str());
            ImGui::Text("Tipo: Audio");
            ImGui::Separator();
            if (ImGui::Button("Reproducir")) {
                PlaySound(snd->sound);
            }
        }
    }

    ImGui::Spacing();
    if (ImGui::Button("Deseleccionar")) {
        m_SelectedAsset.clear();
    }
}

// =============================================================================
// Configuración de Tileset
// =============================================================================

void AssetPanel::RenderTilesetConfig()
{
    auto& mgr = AssetManager::Instance();
    const TilesetInfo* existing = mgr.GetTileset(m_SelectedAsset);

    if (existing) {
        ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.4f, 1.0f), "Tileset configurado");
        ImGui::Text("Tile: %d x %d", existing->tileWidth, existing->tileHeight);
        ImGui::Text("Grid: %d x %d (%d tiles)", existing->columns, existing->rows, existing->tileCount);

        // Preview de tiles individuales
        ImGui::Separator();
        ImGui::Text("Tiles:");

        const TextureAsset* tex = mgr.GetTexture(m_SelectedAsset);
        if (tex) {
            float tileDisplaySize = 32.0f;
            float availW = ImGui::GetContentRegionAvail().x;
            int cols = std::max(1, static_cast<int>(availW / (tileDisplaySize + 4.0f)));
            int col = 0;

            for (int i = 0; i < existing->tileCount && i < 256; i++) {
                Rectangle src = mgr.GetTileRect(m_SelectedAsset, i);
                ImGui::PushID(i);

                // Calcular UVs normalizados
                ImVec2 uv0(src.x / tex->texture.width, src.y / tex->texture.height);
                ImVec2 uv1((src.x + src.width) / tex->texture.width,
                           (src.y + src.height) / tex->texture.height);

                ImGui::ImageButton("##tile",
                                   reinterpret_cast<ImTextureID>(&tex->texture),
                                   ImVec2(tileDisplaySize, tileDisplaySize),
                                   uv0, uv1);

                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Tile #%d", i);
                }

                col++;
                if (col < cols)
                    ImGui::SameLine();
                else
                    col = 0;

                ImGui::PopID();
            }
        }
    } else {
        ImGui::Text("Configurar como Tileset:");
        ImGui::SetNextItemWidth(100);
        ImGui::InputInt("Tile W", &m_TileWidth);
        ImGui::SetNextItemWidth(100);
        ImGui::InputInt("Tile H", &m_TileHeight);
        m_TileWidth = std::max(1, m_TileWidth);
        m_TileHeight = std::max(1, m_TileHeight);

        if (ImGui::Button("Crear Tileset")) {
            mgr.CreateTileset(m_SelectedAsset, m_TileWidth, m_TileHeight);
        }
    }
}

// =============================================================================
// Importar asset (copiar archivo externo al proyecto)
// =============================================================================

void AssetPanel::ImportAsset()
{
    // Usar diálogo nativo de Raylib (tinyfd integrado)
    // En plataformas donde no esté disponible, usamos el file dialog de Raylib
    const char* filters[] = { "*.png", "*.jpg", "*.jpeg", "*.bmp", "*.gif",
                               "*.wav", "*.ogg", "*.mp3" };

    // Nota: Raylib no tiene file dialog nativo. Usamos la API del sistema.
    // En Windows podemos usar tinyfiledialogs o similar.
    // Por ahora, mostramos un popup indicando dónde colocar los archivos.

    ImGui::OpenPopup("ImportHelp");
}

} // namespace Echo
