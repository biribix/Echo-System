#pragma once

#include "raylib.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <cstdint>

namespace Echo {

/// Metadata de una textura cargada
struct TextureAsset {
    Texture2D texture = {};
    std::string relativePath;
    std::string name;
    bool loaded = false;
};

/// Metadata de un sonido cargado
struct SoundAsset {
    Sound sound = {};
    std::string relativePath;
    std::string name;
    bool loaded = false;
};

/// Información de un tileset (textura cortada en tiles)
struct TilesetInfo {
    std::string texturePath;
    int tileWidth = 0;
    int tileHeight = 0;
    int columns = 0;
    int rows = 0;
    int tileCount = 0;
};

/// Gestor central de assets del proyecto.
/// Carga, cachea y descarga texturas y sonidos referenciados por path relativo.
class AssetManager {
public:
    static AssetManager& Instance();

    /// Establece la carpeta raíz de assets del proyecto (ej. "assets/")
    void SetAssetsRoot(const std::string& root);
    const std::string& GetAssetsRoot() const { return m_AssetsRoot; }

    // --- Texturas ---
    /// Carga una textura desde path relativo a assets root. Devuelve true si ok.
    bool LoadTexture(const std::string& relativePath);
    /// Obtiene una textura previamente cargada (o textura vacía si no existe)
    const TextureAsset* GetTexture(const std::string& relativePath) const;
    /// Descarga una textura
    void UnloadTexture(const std::string& relativePath);

    // --- Sonidos ---
    bool LoadSound(const std::string& relativePath);
    const SoundAsset* GetSound(const std::string& relativePath) const;
    void UnloadSound(const std::string& relativePath);

    // --- Tilesets ---
    /// Registra una textura como tileset, cortándola en tiles de tileW x tileH
    bool CreateTileset(const std::string& texturePath, int tileWidth, int tileHeight);
    const TilesetInfo* GetTileset(const std::string& texturePath) const;
    /// Devuelve el rectángulo UV del tile en índice dado
    Rectangle GetTileRect(const std::string& texturePath, int tileIndex) const;

    // --- Escaneo ---
    /// Escanea la carpeta de assets y carga todos los archivos soportados
    void ScanAndLoadAll();

    // --- Consultas ---
    const std::unordered_map<std::string, TextureAsset>& GetAllTextures() const { return m_Textures; }
    const std::unordered_map<std::string, SoundAsset>& GetAllSounds() const { return m_Sounds; }
    const std::unordered_map<std::string, TilesetInfo>& GetAllTilesets() const { return m_Tilesets; }

    /// Descarga todos los recursos
    void UnloadAll();

private:
    AssetManager() = default;
    ~AssetManager();
    AssetManager(const AssetManager&) = delete;
    AssetManager& operator=(const AssetManager&) = delete;

    std::string FullPath(const std::string& relativePath) const;
    bool IsImageExtension(const std::string& ext) const;
    bool IsAudioExtension(const std::string& ext) const;
    void ScanDirectory(const std::string& dirPath);

    std::string m_AssetsRoot = "assets";
    std::unordered_map<std::string, TextureAsset> m_Textures;
    std::unordered_map<std::string, SoundAsset> m_Sounds;
    std::unordered_map<std::string, TilesetInfo> m_Tilesets;
};

} // namespace Echo
