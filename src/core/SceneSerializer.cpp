#include "echo/SceneSerializer.h"
#include "echo/Scene.h"
#include "echo/Entity.h"
#include "echo/Components.h"

#include "nlohmann/json.hpp"
#include "raylib.h"

#include <fstream>

using json = nlohmann::json;

namespace Echo {

SceneSerializer::SceneSerializer(const std::shared_ptr<Scene>& scene)
    : m_Scene(scene)
{
}

// =============================================================================
// Helpers de serialización
// =============================================================================

static json SerializeTransform(const TransformComponent& tc)
{
    return {
        {"position", {tc.position[0], tc.position[1]}},
        {"rotation", tc.rotation},
        {"scale", {tc.scale[0], tc.scale[1]}}
    };
}

static json SerializeSpriteRenderer(const SpriteRendererComponent& sr)
{
    return {
        {"texturePath", sr.texturePath},
        {"color", {sr.color[0], sr.color[1], sr.color[2], sr.color[3]}},
        {"tilesetIndex", sr.tilesetIndex},
        {"flipX", sr.flipX},
        {"flipY", sr.flipY},
        {"layer", sr.layer}
    };
}

static json SerializeAnimationClip(const AnimationClip& clip)
{
    json frames = json::array();
    for (const auto& f : clip.frames) {
        frames.push_back({
            {"texturePath", f.texturePath},
            {"tileIndex", f.tileIndex},
            {"duration", f.duration}
        });
    }
    return {
        {"name", clip.name},
        {"frames", frames},
        {"loop", clip.loop},
        {"speed", clip.speed}
    };
}

static json SerializeAnimation(const AnimationComponent& anim)
{
    json clips = json::array();
    for (const auto& clip : anim.clips) {
        clips.push_back(SerializeAnimationClip(clip));
    }
    return {
        {"clips", clips},
        {"currentClip", anim.currentClip},
        {"playing", anim.playing}
    };
}

static json SerializeCollider(const ColliderComponent& col)
{
    return {
        {"type", static_cast<int>(col.type)},
        {"offset", {col.offset[0], col.offset[1]}},
        {"size", {col.size[0], col.size[1]}},
        {"isTrigger", col.isTrigger}
    };
}

static json SerializeRigidBody(const RigidBodyComponent& rb)
{
    return {
        {"type", static_cast<int>(rb.type)},
        {"velocity", {rb.velocity[0], rb.velocity[1]}},
        {"gravityScale", rb.gravityScale},
        {"mass", rb.mass},
        {"fixedRotation", rb.fixedRotation}
    };
}

static json SerializeCamera(const CameraComponent& cam)
{
    return {
        {"primary", cam.primary},
        {"zoom", cam.zoom}
    };
}

static json SerializeTilemap(const TilemapComponent& tm)
{
    json layersJson = json::array();
    for (const auto& layer : tm.layers) {
        layersJson.push_back({
            {"name", layer.name},
            {"visible", layer.visible},
            {"data", layer.data}
        });
    }

    // Convertir vector<bool> a vector<int> para JSON
    std::vector<int> colData(tm.collisionData.size());
    for (size_t i = 0; i < tm.collisionData.size(); i++)
        colData[i] = tm.collisionData[i] ? 1 : 0;

    return {
        {"tilesetPath", tm.tilesetPath},
        {"tileSize", tm.tileSize},
        {"mapWidth", tm.mapWidth},
        {"mapHeight", tm.mapHeight},
        {"layers", layersJson},
        {"collisionData", colData}
    };
}

// =============================================================================
// Save
// =============================================================================

bool SceneSerializer::Save(const std::string& filepath) const
{
    json root;
    root["scene"] = m_Scene->GetName();
    root["entities"] = json::array();

    auto& reg = m_Scene->GetRegistry();
    auto view = reg.view<TagComponent>();

    for (auto entity : view) {
        json entityJson;

        // Tag (siempre presente)
        auto& tag = reg.get<TagComponent>(entity);
        entityJson["tag"] = {{"name", tag.name}, {"uuid", tag.uuid}};

        // Transform
        if (reg.all_of<TransformComponent>(entity)) {
            entityJson["transform"] = SerializeTransform(reg.get<TransformComponent>(entity));
        }

        // SpriteRenderer
        if (reg.all_of<SpriteRendererComponent>(entity)) {
            entityJson["spriteRenderer"] = SerializeSpriteRenderer(reg.get<SpriteRendererComponent>(entity));
        }

        // Animation
        if (reg.all_of<AnimationComponent>(entity)) {
            entityJson["animation"] = SerializeAnimation(reg.get<AnimationComponent>(entity));
        }

        // Collider
        if (reg.all_of<ColliderComponent>(entity)) {
            entityJson["collider"] = SerializeCollider(reg.get<ColliderComponent>(entity));
        }

        // RigidBody
        if (reg.all_of<RigidBodyComponent>(entity)) {
            entityJson["rigidBody"] = SerializeRigidBody(reg.get<RigidBodyComponent>(entity));
        }

        // Camera
        if (reg.all_of<CameraComponent>(entity)) {
            entityJson["camera"] = SerializeCamera(reg.get<CameraComponent>(entity));
        }

        // Tilemap
        if (reg.all_of<TilemapComponent>(entity)) {
            entityJson["tilemap"] = SerializeTilemap(reg.get<TilemapComponent>(entity));
        }

        root["entities"].push_back(entityJson);
    }

    std::ofstream file(filepath);
    if (!file.is_open()) {
        TraceLog(LOG_WARNING, "SceneSerializer: No se pudo abrir para escritura: %s", filepath.c_str());
        return false;
    }

    file << root.dump(2);
    TraceLog(LOG_INFO, "SceneSerializer: Escena guardada en %s (%d entidades)",
             filepath.c_str(), (int)root["entities"].size());
    return true;
}

// =============================================================================
// Load
// =============================================================================

bool SceneSerializer::Load(const std::string& filepath)
{
    std::ifstream file(filepath);
    if (!file.is_open()) {
        TraceLog(LOG_WARNING, "SceneSerializer: No se pudo abrir: %s", filepath.c_str());
        return false;
    }

    json root;
    try {
        file >> root;
    } catch (const json::parse_error& e) {
        TraceLog(LOG_WARNING, "SceneSerializer: Error de parseo JSON: %s", e.what());
        return false;
    }

    // Limpiar escena actual
    auto& reg = m_Scene->GetRegistry();
    reg.clear();

    // Nombre de escena
    if (root.contains("scene")) {
        m_Scene->SetName(root["scene"].get<std::string>());
    }

    // Recrear entidades
    if (!root.contains("entities")) return true;

    for (const auto& entityJson : root["entities"]) {
        std::string name = "Entidad";
        uint64_t uuid = 0;

        if (entityJson.contains("tag")) {
            name = entityJson["tag"].value("name", "Entidad");
            uuid = entityJson["tag"].value("uuid", (uint64_t)0);
        }

        Entity entity = m_Scene->CreateEntity(name);
        // Sobreescribir UUID si se proporcionó
        if (uuid > 0) {
            entity.Get<TagComponent>().uuid = uuid;
        }

        // Transform
        if (entityJson.contains("transform")) {
            auto& tc = entity.Get<TransformComponent>();
            auto& t = entityJson["transform"];
            if (t.contains("position") && t["position"].is_array()) {
                tc.position[0] = t["position"][0].get<float>();
                tc.position[1] = t["position"][1].get<float>();
            }
            tc.rotation = t.value("rotation", 0.0f);
            if (t.contains("scale") && t["scale"].is_array()) {
                tc.scale[0] = t["scale"][0].get<float>();
                tc.scale[1] = t["scale"][1].get<float>();
            }
        }

        // SpriteRenderer
        if (entityJson.contains("spriteRenderer")) {
            auto& sr = entity.Add<SpriteRendererComponent>();
            auto& s = entityJson["spriteRenderer"];
            sr.texturePath = s.value("texturePath", "");
            if (s.contains("color") && s["color"].is_array()) {
                for (int i = 0; i < 4; i++)
                    sr.color[i] = s["color"][i].get<float>();
            }
            sr.tilesetIndex = s.value("tilesetIndex", -1);
            sr.flipX = s.value("flipX", false);
            sr.flipY = s.value("flipY", false);
            sr.layer = s.value("layer", 0);
        }

        // Animation
        if (entityJson.contains("animation")) {
            auto& anim = entity.Add<AnimationComponent>();
            auto& a = entityJson["animation"];
            anim.playing = a.value("playing", true);
            anim.currentClip = a.value("currentClip", 0);
            anim.clips.clear();

            if (a.contains("clips")) {
                for (const auto& clipJson : a["clips"]) {
                    AnimationClip clip;
                    clip.name = clipJson.value("name", "Default");
                    clip.loop = clipJson.value("loop", true);
                    clip.speed = clipJson.value("speed", 1.0f);

                    if (clipJson.contains("frames")) {
                        for (const auto& frameJson : clipJson["frames"]) {
                            AnimationFrame frame;
                            frame.texturePath = frameJson.value("texturePath", "");
                            frame.tileIndex = frameJson.value("tileIndex", 0);
                            frame.duration = frameJson.value("duration", 0.1f);
                            clip.frames.push_back(frame);
                        }
                    }
                    anim.clips.push_back(clip);
                }
            }
        }

        // Collider
        if (entityJson.contains("collider")) {
            auto& col = entity.Add<ColliderComponent>();
            auto& c = entityJson["collider"];
            col.type = static_cast<ColliderType>(c.value("type", 0));
            if (c.contains("offset") && c["offset"].is_array()) {
                col.offset[0] = c["offset"][0].get<float>();
                col.offset[1] = c["offset"][1].get<float>();
            }
            if (c.contains("size") && c["size"].is_array()) {
                col.size[0] = c["size"][0].get<float>();
                col.size[1] = c["size"][1].get<float>();
            }
            col.isTrigger = c.value("isTrigger", false);
        }

        // RigidBody
        if (entityJson.contains("rigidBody")) {
            auto& rb = entity.Add<RigidBodyComponent>();
            auto& r = entityJson["rigidBody"];
            rb.type = static_cast<BodyType>(r.value("type", 0));
            if (r.contains("velocity") && r["velocity"].is_array()) {
                rb.velocity[0] = r["velocity"][0].get<float>();
                rb.velocity[1] = r["velocity"][1].get<float>();
            }
            rb.gravityScale = r.value("gravityScale", 1.0f);
            rb.mass = r.value("mass", 1.0f);
            rb.fixedRotation = r.value("fixedRotation", true);
        }

        // Camera
        if (entityJson.contains("camera")) {
            auto& cam = entity.Add<CameraComponent>();
            auto& c = entityJson["camera"];
            cam.primary = c.value("primary", true);
            cam.zoom = c.value("zoom", 1.0f);
        }

        // Tilemap
        if (entityJson.contains("tilemap")) {
            auto& tm = entity.Add<TilemapComponent>();
            auto& t = entityJson["tilemap"];
            tm.tilesetPath = t.value("tilesetPath", "");
            tm.tileSize = t.value("tileSize", 32);
            tm.mapWidth = t.value("mapWidth", 30);
            tm.mapHeight = t.value("mapHeight", 20);

            int total = tm.mapWidth * tm.mapHeight;
            tm.layers.clear();

            if (t.contains("layers")) {
                for (const auto& layerJson : t["layers"]) {
                    TilemapLayer layer;
                    layer.name = layerJson.value("name", "Capa");
                    layer.visible = layerJson.value("visible", true);
                    if (layerJson.contains("data") && layerJson["data"].is_array()) {
                        layer.data = layerJson["data"].get<std::vector<int>>();
                        layer.data.resize(total, -1);
                    } else {
                        layer.data.assign(total, -1);
                    }
                    tm.layers.push_back(std::move(layer));
                }
            }
            if (tm.layers.empty()) {
                tm.Initialize();
            }

            tm.collisionData.assign(total, false);
            if (t.contains("collisionData") && t["collisionData"].is_array()) {
                auto colVec = t["collisionData"].get<std::vector<int>>();
                for (size_t i = 0; i < colVec.size() && i < (size_t)total; i++)
                    tm.collisionData[i] = (colVec[i] != 0);
            }
        }
    }

    TraceLog(LOG_INFO, "SceneSerializer: Escena cargada desde %s (%d entidades)",
             filepath.c_str(), (int)m_Scene->GetEntityCount());
    return true;
}

} // namespace Echo
