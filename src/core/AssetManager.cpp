#include "echo/AssetManager.h"
#include "raylib.h"

#include <algorithm>
#include <filesystem>

namespace fs = std::filesystem;

namespace Echo {

AssetManager& AssetManager::Instance()
{
    static AssetManager instance;
    return instance;
}

AssetManager::~AssetManager()
{
    UnloadAll();
}

void AssetManager::SetAssetsRoot(const std::string& root)
{
    m_AssetsRoot = root;
}

std::string AssetManager::FullPath(const std::string& relativePath) const
{
    return m_AssetsRoot + "/" + relativePath;
}

// =============================================================================
// Texturas
// =============================================================================

bool AssetManager::LoadTexture(const std::string& relativePath)
{
    // Si ya está cargada, no recargar
    if (m_Textures.count(relativePath) && m_Textures[relativePath].loaded)
        return true;

    std::string full = FullPath(relativePath);
    if (!FileExists(full.c_str())) {
        TraceLog(LOG_WARNING, "AssetManager: Textura no encontrada: %s", full.c_str());
        return false;
    }

    Texture2D tex = ::LoadTexture(full.c_str());
    if (tex.id == 0) {
        TraceLog(LOG_WARNING, "AssetManager: Error cargando textura: %s", full.c_str());
        return false;
    }

    TextureAsset asset;
    asset.texture = tex;
    asset.relativePath = relativePath;
    // Extraer nombre del archivo
    fs::path p(relativePath);
    asset.name = p.stem().string();
    asset.loaded = true;

    m_Textures[relativePath] = asset;
    TraceLog(LOG_INFO, "AssetManager: Textura cargada: %s (%dx%d)", relativePath.c_str(), tex.width, tex.height);
    return true;
}

const TextureAsset* AssetManager::GetTexture(const std::string& relativePath) const
{
    auto it = m_Textures.find(relativePath);
    if (it != m_Textures.end() && it->second.loaded)
        return &it->second;
    return nullptr;
}

void AssetManager::UnloadTexture(const std::string& relativePath)
{
    auto it = m_Textures.find(relativePath);
    if (it != m_Textures.end()) {
        if (it->second.loaded)
            ::UnloadTexture(it->second.texture);
        m_Textures.erase(it);
    }
}

// =============================================================================
// Sonidos
// =============================================================================

bool AssetManager::LoadSound(const std::string& relativePath)
{
    if (m_Sounds.count(relativePath) && m_Sounds[relativePath].loaded)
        return true;

    std::string full = FullPath(relativePath);
    if (!FileExists(full.c_str())) {
        TraceLog(LOG_WARNING, "AssetManager: Audio no encontrado: %s", full.c_str());
        return false;
    }

    Sound snd = ::LoadSound(full.c_str());

    SoundAsset asset;
    asset.sound = snd;
    asset.relativePath = relativePath;
    fs::path p(relativePath);
    asset.name = p.stem().string();
    asset.loaded = true;

    m_Sounds[relativePath] = asset;
    TraceLog(LOG_INFO, "AssetManager: Audio cargado: %s", relativePath.c_str());
    return true;
}

const SoundAsset* AssetManager::GetSound(const std::string& relativePath) const
{
    auto it = m_Sounds.find(relativePath);
    if (it != m_Sounds.end() && it->second.loaded)
        return &it->second;
    return nullptr;
}

void AssetManager::UnloadSound(const std::string& relativePath)
{
    auto it = m_Sounds.find(relativePath);
    if (it != m_Sounds.end()) {
        if (it->second.loaded)
            ::UnloadSound(it->second.sound);
        m_Sounds.erase(it);
    }
}

// =============================================================================
// Tilesets
// =============================================================================

bool AssetManager::CreateTileset(const std::string& texturePath, int tileWidth, int tileHeight)
{
    const TextureAsset* tex = GetTexture(texturePath);
    if (!tex) {
        // Intentar cargar primero
        if (!LoadTexture(texturePath))
            return false;
        tex = GetTexture(texturePath);
    }

    TilesetInfo info;
    info.texturePath = texturePath;
    info.tileWidth = tileWidth;
    info.tileHeight = tileHeight;
    info.columns = tex->texture.width / tileWidth;
    info.rows = tex->texture.height / tileHeight;
    info.tileCount = info.columns * info.rows;

    m_Tilesets[texturePath] = info;
    TraceLog(LOG_INFO, "AssetManager: Tileset creado: %s (%dx%d tiles, %d total)",
             texturePath.c_str(), info.columns, info.rows, info.tileCount);
    return true;
}

const TilesetInfo* AssetManager::GetTileset(const std::string& texturePath) const
{
    auto it = m_Tilesets.find(texturePath);
    if (it != m_Tilesets.end())
        return &it->second;
    return nullptr;
}

Rectangle AssetManager::GetTileRect(const std::string& texturePath, int tileIndex) const
{
    const TilesetInfo* info = GetTileset(texturePath);
    if (!info || tileIndex < 0 || tileIndex >= info->tileCount)
        return {0, 0, 0, 0};

    int col = tileIndex % info->columns;
    int row = tileIndex / info->columns;

    return {
        static_cast<float>(col * info->tileWidth),
        static_cast<float>(row * info->tileHeight),
        static_cast<float>(info->tileWidth),
        static_cast<float>(info->tileHeight)
    };
}

// =============================================================================
// Escaneo y carga masiva
// =============================================================================

bool AssetManager::IsImageExtension(const std::string& ext) const
{
    return ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp" || ext == ".gif";
}

bool AssetManager::IsAudioExtension(const std::string& ext) const
{
    return ext == ".wav" || ext == ".ogg" || ext == ".mp3";
}

void AssetManager::ScanDirectory(const std::string& dirPath)
{
    if (!fs::exists(dirPath) || !fs::is_directory(dirPath))
        return;

    for (const auto& entry : fs::recursive_directory_iterator(dirPath)) {
        if (!entry.is_regular_file())
            continue;

        // Obtener path relativo a assets root
        fs::path rel = fs::relative(entry.path(), m_AssetsRoot);
        std::string relStr = rel.generic_string(); // usar '/' siempre
        std::string ext = rel.extension().string();

        // Convertir extensión a minúsculas
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

        if (IsImageExtension(ext)) {
            LoadTexture(relStr);
        } else if (IsAudioExtension(ext)) {
            LoadSound(relStr);
        }
    }
}

void AssetManager::ScanAndLoadAll()
{
    TraceLog(LOG_INFO, "AssetManager: Escaneando carpeta: %s", m_AssetsRoot.c_str());
    ScanDirectory(m_AssetsRoot);
    TraceLog(LOG_INFO, "AssetManager: %d texturas, %d sonidos cargados",
             (int)m_Textures.size(), (int)m_Sounds.size());
}

// =============================================================================
// Limpieza
// =============================================================================

void AssetManager::UnloadAll()
{
    for (auto& [path, asset] : m_Textures) {
        if (asset.loaded)
            ::UnloadTexture(asset.texture);
    }
    m_Textures.clear();

    for (auto& [path, asset] : m_Sounds) {
        if (asset.loaded)
            ::UnloadSound(asset.sound);
    }
    m_Sounds.clear();

    m_Tilesets.clear();
    TraceLog(LOG_INFO, "AssetManager: Todos los recursos descargados");
}

} // namespace Echo
