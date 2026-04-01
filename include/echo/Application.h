#pragma once

#include "echo/AssetPanel.h"
#include "echo/SceneHierarchyPanel.h"
#include "echo/PropertiesPanel.h"
#include <memory>

namespace Echo {

class Scene;
class SceneSerializer;

class Application {
public:
    Application(int width, int height, const char* title);
    ~Application();

    // No copiable
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    /// Ejecuta el loop principal del motor
    void Run();

private:
    void Init();
    void Shutdown();
    void Update();
    void Render();
    void RenderGUI();

    void NewScene();
    void SaveScene();
    void LoadScene();

    int m_Width;
    int m_Height;
    const char* m_Title;
    bool m_Running = true;
    std::string m_CurrentScenePath;

    // Escena activa
    std::shared_ptr<Scene> m_ActiveScene;

    // Paneles del editor
    std::unique_ptr<AssetPanel> m_AssetPanel;
    std::unique_ptr<SceneHierarchyPanel> m_SceneHierarchyPanel;
    std::unique_ptr<PropertiesPanel> m_PropertiesPanel;
};

} // namespace Echo
