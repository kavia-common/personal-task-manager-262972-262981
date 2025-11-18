#include <iostream>
#include <string>
#include <chrono>
#include <fstream>
#include <sstream>

#ifdef APP_WITH_GUI
#include <SDL.h>
#include <SDL_opengl.h>
#include "imgui.h"
#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_opengl2.h"
#include "persistence/Json.h"
#endif

#include "app/Application.h"

/*
 PUBLIC_INTERFACE
 int main(int argc, char** argv)
 
 Entry point for the To-Do native app.
 - Initializes SDL2 and Dear ImGui when built with GUI.
 - Creates and runs the main application loop.
 - Ensures tasks are loaded at startup and saved on exit.
 
 Parameters:
 - argc: argument count
 - argv: argument vector
 
 Returns:
 - int process exit code
*/
int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

#ifndef APP_WITH_GUI
    // Headless fallback: basic CLI lifecycle
    Application app;
    if (!app.initialize()) {
        std::cerr << "Failed to initialize application." << std::endl;
        return 1;
    }
    app.load();
    std::cout << "To-Do App (headless). Tasks loaded: " << app.taskCount() << std::endl;
    app.save();
    std::cout << "Saved.\n";
    return 0;
#else
    // SDL initialization
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0) {
        std::cerr << "Error: " << SDL_GetError() << std::endl;
        return -1;
    }

    // Prepare window geometry from settings.json
    int winW = 960, winH = 600;
    int winX = SDL_WINDOWPOS_CENTERED, winY = SDL_WINDOWPOS_CENTERED;
    bool themeDark = false;
    // Read settings.json if present (same directory as tasks.json)
    // We create a temp Storage to resolve path without loading tasks.
    Storage tempStorage;
    std::string settingsPath = tempStorage.settingsFile();
    {
        std::ifstream sifs(settingsPath);
        if (sifs.is_open()) {
            try {
                nlohmann::json sj;
                sifs >> sj;
                if (sj.contains("windowW")) winW = sj.value("windowW", winW);
                if (sj.contains("windowH")) winH = sj.value("windowH", winH);
                if (sj.contains("themeDark")) themeDark = sj.value("themeDark", themeDark);
            } catch (...) {
                // ignore malformed settings
            }
        }
    }

    // GL context attributes (OpenGL 2 for portability)
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
#if __APPLE__
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
#endif

    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window* window = SDL_CreateWindow("To-Do App", winX, winY, winW, winH, window_flags);
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // Style
    if (themeDark) ImGui::StyleColorsDark(); else ImGui::StyleColorsLight();

    // Ocean Professional theme tweaks
    ImGuiStyle& style = ImGui::GetStyle();
    style.FrameRounding = 6.0f;
    style.WindowRounding = 8.0f;
    style.GrabRounding = 6.0f;
    style.ChildRounding = 8.0f;
    style.TabRounding = 6.0f;
    style.WindowPadding = ImVec2(14, 12);
    style.FramePadding = ImVec2(10, 6);
    style.ItemSpacing = ImVec2(8, 8);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.145f, 0.388f, 0.922f, 1.0f); // #2563EB
    style.Colors[ImGuiCol_Button]       = ImVec4(0.145f, 0.388f, 0.922f, 0.85f);
    style.Colors[ImGuiCol_ButtonHovered]= ImVec4(0.145f, 0.388f, 0.922f, 1.0f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.104f, 0.306f, 0.748f, 1.0f);
    style.Colors[ImGuiCol_CheckMark]    = ImVec4(0.960f, 0.619f, 0.043f, 1.0f); // #F59E0B
    style.Colors[ImGuiCol_Header]       = ImVec4(0.960f, 0.619f, 0.043f, 0.65f);
    style.Colors[ImGuiCol_HeaderHovered]= ImVec4(0.960f, 0.619f, 0.043f, 0.85f);
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.960f, 0.619f, 0.043f, 1.0f);
    style.Colors[ImGuiCol_WindowBg]     = themeDark ? ImVec4(0.10f, 0.11f, 0.12f, 1.0f) : ImVec4(0.976f, 0.980f, 0.984f, 1.0f);

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL2_Init();

    Application app;
    if (!app.initialize()) {
        std::cerr << "Failed to initialize application." << std::endl;
        return 1;
    }
    app.load();

    bool done = false;
    while (!done) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
                done = true;
            app.onEvent(event);
        }

        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        app.render();

        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(0.972f, 0.976f, 0.984f, 1.0f); // bg
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);

        app.maybeAutoSave();
    }

    app.save();

    // Persist window geometry and theme to settings.json
    int x, y, w, h;
    SDL_GetWindowPosition(window, &x, &y);
    SDL_GetWindowSize(window, &w, &h);
    nlohmann::json sj;
    sj["windowX"] = x;
    sj["windowY"] = y;
    sj["windowW"] = w;
    sj["windowH"] = h;
    sj["themeDark"] = themeDark;
    // write using Storage atomicWrite via a small helper
    {
        std::ofstream ofs(settingsPath + ".tmp", std::ios::binary | std::ios::trunc);
        if (ofs.is_open()) {
            ofs << sj.dump(2);
            ofs.flush();
        }
#if defined(_WIN32)
        MoveFileExA(std::string(settingsPath + ".tmp").c_str(), settingsPath.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
#else
        std::rename(std::string(settingsPath + ".tmp").c_str(), settingsPath.c_str());
#endif
    }

    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
#endif
}
