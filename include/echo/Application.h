#pragma once

#include "echo/AssetPanel.h"
#include <memory>

namespace Echo {

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

    int m_Width;
    int m_Height;
    const char* m_Title;
    bool m_Running = true;

    // Paneles del editor
    std::unique_ptr<AssetPanel> m_AssetPanel;
};

} // namespace Echo
