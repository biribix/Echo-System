#pragma once

#include "raylib.h"
#include <string>
#include <vector>
#include <cstdint>

namespace Echo {

// =============================================================================
// TagComponent — Identifica a cada entidad con nombre y UUID
// =============================================================================
struct TagComponent {
    std::string name = "Entidad";
    uint64_t uuid = 0;
};

// =============================================================================
// TransformComponent — Posición, rotación y escala en el mundo 2D
// =============================================================================
struct TransformComponent {
    float position[2] = {0.0f, 0.0f};  // x, y
    float rotation = 0.0f;              // grados
    float scale[2] = {1.0f, 1.0f};     // sx, sy
};

// =============================================================================
// SpriteRendererComponent — Renderizado visual de la entidad
// =============================================================================
struct SpriteRendererComponent {
    std::string texturePath;        // path relativo al asset
    float color[4] = {1, 1, 1, 1}; // RGBA tint
    int tilesetIndex = -1;          // -1 = textura completa, >=0 = tile específico
    bool flipX = false;
    bool flipY = false;
    int layer = 0;                  // capa de renderizado (mayor = encima)
};

// =============================================================================
// AnimationComponent — Animación de sprites por frames
// =============================================================================
struct AnimationFrame {
    std::string texturePath;    // puede ser el mismo tileset
    int tileIndex = 0;          // índice del tile en el tileset
    float duration = 0.1f;      // duración de este frame en segundos
};

struct AnimationClip {
    std::string name = "Default";
    std::vector<AnimationFrame> frames;
    bool loop = true;
    float speed = 1.0f;
};

struct AnimationComponent {
    std::vector<AnimationClip> clips;
    int currentClip = 0;
    int currentFrame = 0;
    float timer = 0.0f;
    bool playing = true;
};

// =============================================================================
// ColliderComponent — Hitbox para detección de colisiones
// =============================================================================
enum class ColliderType {
    Box,
    Circle
};

struct ColliderComponent {
    ColliderType type = ColliderType::Box;
    float offset[2] = {0.0f, 0.0f};
    float size[2] = {32.0f, 32.0f};    // ancho/alto para Box, radio en size[0] para Circle
    bool isTrigger = false;             // true = solo detecta, no empuja
};

// =============================================================================
// RigidBodyComponent — Físicas básicas
// =============================================================================
enum class BodyType {
    Static,     // no se mueve (suelos, paredes)
    Dynamic,    // afectado por gravedad y fuerzas
    Kinematic   // se mueve por código, no por físicas
};

struct RigidBodyComponent {
    BodyType type = BodyType::Static;
    float velocity[2] = {0.0f, 0.0f};
    float gravityScale = 1.0f;
    float mass = 1.0f;
    bool fixedRotation = true;
};

// =============================================================================
// CameraComponent — Para definir la cámara del juego
// =============================================================================
struct CameraComponent {
    bool primary = true;
    float zoom = 1.0f;
};

// =============================================================================
// TilemapComponent — Mapa de tiles con múltiples capas
// =============================================================================

/// Una capa del tilemap. Cada celda almacena el índice del tile (-1 = vacío).
struct TilemapLayer {
    std::string name = "Capa";
    std::vector<int> data;      // tamaño = mapWidth * mapHeight
    bool visible = true;
};

struct TilemapComponent {
    std::string tilesetPath;        // path al tileset en AssetManager
    int tileSize = 32;              // tamaño de cada tile en píxeles
    int mapWidth = 30;              // ancho del mapa en tiles
    int mapHeight = 20;             // alto del mapa en tiles

    std::vector<TilemapLayer> layers;
    std::vector<bool> collisionData; // tamaño = mapWidth * mapHeight

    /// Inicializa capas y datos con el tamaño dado
    void Initialize()
    {
        int total = mapWidth * mapHeight;
        collisionData.assign(total, false);
        if (layers.empty()) {
            TilemapLayer bg;
            bg.name = "Fondo";
            bg.data.assign(total, -1);
            layers.push_back(bg);
        } else {
            for (auto& layer : layers)
                layer.data.resize(total, -1);
        }
    }

    /// Acceso seguro a un tile de una capa
    int GetTile(int layerIdx, int x, int y) const
    {
        if (layerIdx < 0 || layerIdx >= (int)layers.size()) return -1;
        if (x < 0 || x >= mapWidth || y < 0 || y >= mapHeight) return -1;
        return layers[layerIdx].data[y * mapWidth + x];
    }

    void SetTile(int layerIdx, int x, int y, int tileIndex)
    {
        if (layerIdx < 0 || layerIdx >= (int)layers.size()) return;
        if (x < 0 || x >= mapWidth || y < 0 || y >= mapHeight) return;
        layers[layerIdx].data[y * mapWidth + x] = tileIndex;
    }

    bool GetCollision(int x, int y) const
    {
        if (x < 0 || x >= mapWidth || y < 0 || y >= mapHeight) return false;
        return collisionData[y * mapWidth + x];
    }

    void SetCollision(int x, int y, bool solid)
    {
        if (x < 0 || x >= mapWidth || y < 0 || y >= mapHeight) return;
        collisionData[y * mapWidth + x] = solid;
    }
};

} // namespace Echo
