#pragma once

#include <string>
#include <memory>

namespace Echo {

class Scene;

/// Serializa y deserializa escenas completas a/desde JSON.
class SceneSerializer {
public:
    explicit SceneSerializer(const std::shared_ptr<Scene>& scene);

    /// Guarda la escena a un archivo JSON
    bool Save(const std::string& filepath) const;

    /// Carga una escena desde un archivo JSON (reemplaza el contenido actual)
    bool Load(const std::string& filepath);

private:
    std::shared_ptr<Scene> m_Scene;
};

} // namespace Echo
