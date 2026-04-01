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

} // namespace Echo
