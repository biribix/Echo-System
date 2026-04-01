#pragma once

#include <string>
#include <vector>

namespace Echo {

/// Panel visual de ImGui para explorar, importar y previsualizar assets.
class AssetPanel {
public:
    AssetPanel();

    /// Renderiza todo el panel (llamar dentro del frame ImGui)
    void Render();

    bool IsVisible() const { return m_Visible; }
    void SetVisible(bool visible) { m_Visible = visible; }
    void ToggleVisible() { m_Visible = !m_Visible; }

private:
    void RenderToolbar();
    void RenderFolderTree();
    void RenderAssetGrid();
    void RenderPreviewPanel();
    void RenderTilesetConfig();

    void ImportAsset();
    void RefreshFileTree();

    /// Nodo del árbol de carpetas
    struct FolderNode {
        std::string name;
        std::string fullPath;
        std::vector<FolderNode> children;
        std::vector<std::string> files; // paths relativos de archivos en esta carpeta
    };

    void BuildFolderTree(const std::string& dirPath, FolderNode& node);
    void RenderFolderNode(FolderNode& node);

    bool m_Visible = true;
    FolderNode m_RootNode;
    std::string m_SelectedFolder;
    std::string m_SelectedAsset;    // path relativo del asset seleccionado
    float m_ThumbnailSize = 64.0f;
    bool m_NeedsRefresh = true;

    // Tileset config para el asset seleccionado
    int m_TileWidth = 32;
    int m_TileHeight = 32;
};

} // namespace Echo
