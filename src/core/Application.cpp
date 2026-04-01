#include "echo/Application.h"

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
}

void Application::Shutdown()
{
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
            ImGui::MenuItem("Assets");
            ImGui::MenuItem("Escena");
            ImGui::MenuItem("Propiedades");
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    // --- Ventana principal de bienvenida ---
    ImGui::SetNextWindowSize(ImVec2(500, 200), ImGuiCond_FirstUseEver);
    ImGui::Begin("Echo Engine");

    ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Motor 2D Iniciado");
    ImGui::Separator();
    ImGui::Text("Bienvenido a Echo Engine v0.1.0");
    ImGui::Text("Motor de videojuegos 2D visual - No-Code");
    ImGui::Spacing();
    ImGui::TextWrapped(
        "Este es el punto de partida del editor. Los paneles de Assets, "
        "Escena, Tilemap y Propiedades se iran integrando modulo a modulo."
    );

    ImGui::End();
}

} // namespace Echo
