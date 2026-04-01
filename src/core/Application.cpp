#include "echo/Application.h"
#include "echo/AssetManager.h"

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
    // Configurar flags antes de crear la ventana
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(m_Width, m_Height, m_Title);
    SetTargetFPS(60);

    // Inicializar rlImGui (configura Dear ImGui + backend Raylib)
    rlImGuiSetup(true); // true = dark theme

    // Inicializar audio (necesario para reproducir sonidos en el editor)
    InitAudioDevice();

    // Inicializar AssetManager y escanear assets existentes
    AssetManager::Instance().SetAssetsRoot("assets");
    AssetManager::Instance().ScanAndLoadAll();

    // Crear paneles del editor
    m_AssetPanel = std::make_unique<AssetPanel>();
}

void Application::Shutdown()
{
    m_AssetPanel.reset();
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
    // Aquí irá la lógica de actualización del editor y sistemas ECS
}

void Application::Render()
{
    BeginDrawing();
    ClearBackground(DARKGRAY);

    // Comenzar frame de ImGui
    rlImGuiBegin();

    RenderGUI();

    // Finalizar frame de ImGui
    rlImGuiEnd();

    EndDrawing();
}

void Application::RenderGUI()
{
    // --- Barra de menú principal ---
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("Archivo")) {
            if (ImGui::MenuItem("Nuevo Proyecto"))  { /* TODO */ }
            if (ImGui::MenuItem("Abrir Proyecto"))  { /* TODO */ }
            if (ImGui::MenuItem("Guardar"))         { /* TODO */ }
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
            bool assetsVisible = m_AssetPanel->IsVisible();
            if (ImGui::MenuItem("Assets", nullptr, &assetsVisible)) {
                m_AssetPanel->SetVisible(assetsVisible);
            }
            ImGui::MenuItem("Escena");
            ImGui::MenuItem("Propiedades");
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    // --- Panel de Assets ---
    m_AssetPanel->Render();

    // --- Popup de ayuda para importar (mostrado desde AssetPanel) ---
    if (ImGui::BeginPopupModal("ImportHelp", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Para importar assets:");
        ImGui::BulletText("Copia tus archivos de imagen (.png, .jpg, .bmp)");
        ImGui::BulletText("o audio (.wav, .ogg, .mp3)");
        ImGui::BulletText("dentro de la carpeta 'assets/' del proyecto.");
        ImGui::Spacing();
        ImGui::Text("Subcarpetas soportadas:");
        ImGui::BulletText("assets/sprites/  - para sprites y personajes");
        ImGui::BulletText("assets/tilesets/ - para tilesets de mapas");
        ImGui::BulletText("assets/audio/   - para efectos y musica");
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.4f, 1.0f),
                           "Luego pulsa 'Refrescar' en el panel de Assets.");
        ImGui::Spacing();
        if (ImGui::Button("Entendido", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    // --- Ventana de info del motor ---
    ImGui::SetNextWindowSize(ImVec2(400, 150), ImGuiCond_FirstUseEver);
    ImGui::Begin("Echo Engine");

    ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Motor 2D Iniciado");
    ImGui::Separator();

    auto& mgr = AssetManager::Instance();
    ImGui::Text("Texturas cargadas: %d", (int)mgr.GetAllTextures().size());
    ImGui::Text("Sonidos cargados:  %d", (int)mgr.GetAllSounds().size());
    ImGui::Text("Tilesets:          %d", (int)mgr.GetAllTilesets().size());

    ImGui::End();
}

} // namespace Echo
