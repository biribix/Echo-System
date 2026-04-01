#include "echo/Application.h"
#include "echo/AssetManager.h"
#include "echo/Scene.h"
#include "echo/SceneSerializer.h"
#include "echo/Components.h"

#include "raylib.h"
#include "imgui.h"
#include "rlImGui.h"

namespace Echo {

Application::Application(int width, int height, const char* title)
    : m_Width(width), m_Height(height), m_Title(title)
{
    Init();
}

Application::~Application()
{
    Shutdown();
}

void Application::Init()
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(m_Width, m_Height, m_Title);
    SetTargetFPS(60);

    rlImGuiSetup(true);
    InitAudioDevice();

    // AssetManager
    AssetManager::Instance().SetAssetsRoot("assets");
    AssetManager::Instance().ScanAndLoadAll();

    // Escena por defecto
    m_ActiveScene = std::make_shared<Scene>("Escena Principal");

    // Paneles del editor
    m_AssetPanel = std::make_unique<AssetPanel>();

    m_SceneHierarchyPanel = std::make_unique<SceneHierarchyPanel>();
    m_SceneHierarchyPanel->SetScene(m_ActiveScene);

    m_PropertiesPanel = std::make_unique<PropertiesPanel>();
    m_PropertiesPanel->SetScene(m_ActiveScene);
}

void Application::Shutdown()
{
    m_AssetPanel.reset();
    m_SceneHierarchyPanel.reset();
    m_PropertiesPanel.reset();
    m_ActiveScene.reset();
    AssetManager::Instance().UnloadAll();
    CloseAudioDevice();
    rlImGuiShutdown();
    CloseWindow();
}

void Application::Run()
{
    while (!WindowShouldClose() && m_Running) {
        Update();
        Render();
    }
}

void Application::Update()
{
    // Atajos de teclado
    if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) {
        if (IsKeyPressed(KEY_S)) SaveScene();
        if (IsKeyPressed(KEY_O)) LoadScene();
        if (IsKeyPressed(KEY_N)) NewScene();
    }
}

void Application::Render()
{
    BeginDrawing();
    ClearBackground(DARKGRAY);

    rlImGuiBegin();
    RenderGUI();
    rlImGuiEnd();

    EndDrawing();
}

// =============================================================================
// Acciones de escena
// =============================================================================

void Application::NewScene()
{
    m_ActiveScene = std::make_shared<Scene>("Nueva Escena");
    m_SceneHierarchyPanel->SetScene(m_ActiveScene);
    m_SceneHierarchyPanel->ClearSelection();
    m_PropertiesPanel->SetScene(m_ActiveScene);
    m_CurrentScenePath.clear();
}

void Application::SaveScene()
{
    if (m_CurrentScenePath.empty()) {
        m_CurrentScenePath = "assets/" + m_ActiveScene->GetName() + ".scene.json";
    }
    SceneSerializer serializer(m_ActiveScene);
    serializer.Save(m_CurrentScenePath);
}

void Application::LoadScene()
{
    // Por ahora carga desde un path predefinido.
    // En el futuro se integraría un file dialog.
    std::string path = "assets/Escena Principal.scene.json";
    SceneSerializer serializer(m_ActiveScene);
    if (serializer.Load(path)) {
        m_SceneHierarchyPanel->SetScene(m_ActiveScene);
        m_SceneHierarchyPanel->ClearSelection();
        m_PropertiesPanel->SetScene(m_ActiveScene);
        m_CurrentScenePath = path;
    }
}

// =============================================================================
// GUI
// =============================================================================

void Application::RenderGUI()
{
    // --- Barra de menú principal ---
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("Archivo")) {
            if (ImGui::MenuItem("Nueva Escena", "Ctrl+N"))  { NewScene(); }
            if (ImGui::MenuItem("Abrir Escena", "Ctrl+O"))  { LoadScene(); }
            if (ImGui::MenuItem("Guardar Escena", "Ctrl+S")) { SaveScene(); }
            ImGui::Separator();
            if (ImGui::MenuItem("Salir")) { m_Running = false; }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Editar")) {
            if (ImGui::MenuItem("Deshacer", "Ctrl+Z")) { /* TODO */ }
            if (ImGui::MenuItem("Rehacer", "Ctrl+Y"))  { /* TODO */ }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Ver")) {
            bool assetsVis = m_AssetPanel->IsVisible();
            if (ImGui::MenuItem("Assets", nullptr, &assetsVis))
                m_AssetPanel->SetVisible(assetsVis);

            bool sceneVis = m_SceneHierarchyPanel->IsVisible();
            if (ImGui::MenuItem("Escena", nullptr, &sceneVis))
                m_SceneHierarchyPanel->SetVisible(sceneVis);

            bool propsVis = m_PropertiesPanel->IsVisible();
            if (ImGui::MenuItem("Propiedades", nullptr, &propsVis))
                m_PropertiesPanel->SetVisible(propsVis);

            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    // --- Paneles ---
    m_AssetPanel->Render();
    m_SceneHierarchyPanel->Render();
    m_PropertiesPanel->Render(m_SceneHierarchyPanel->GetSelectedEntity());

    // --- Popup de ayuda para importar ---
    if (ImGui::BeginPopupModal("ImportHelp", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Para importar assets:");
        ImGui::BulletText("Copia archivos de imagen (.png, .jpg, .bmp)");
        ImGui::BulletText("o audio (.wav, .ogg, .mp3)");
        ImGui::BulletText("dentro de la carpeta 'assets/' del proyecto.");
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.4f, 1.0f),
                           "Luego pulsa 'Refrescar' en el panel de Assets.");
        ImGui::Spacing();
        if (ImGui::Button("Entendido", ImVec2(120, 0)))
            ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }

    // --- Ventana de estado ---
    ImGui::SetNextWindowSize(ImVec2(350, 130), ImGuiCond_FirstUseEver);
    ImGui::Begin("Echo Engine");

    ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Motor 2D Iniciado");
    ImGui::Separator();

    ImGui::Text("Escena: %s", m_ActiveScene->GetName().c_str());
    ImGui::Text("Entidades: %d", (int)m_ActiveScene->GetEntityCount());

    auto& mgr = AssetManager::Instance();
    ImGui::Text("Texturas: %d | Sonidos: %d | Tilesets: %d",
                (int)mgr.GetAllTextures().size(),
                (int)mgr.GetAllSounds().size(),
                (int)mgr.GetAllTilesets().size());

    ImGui::End();
}

} // namespace Echo
