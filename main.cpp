#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl2.h"
#include <SDL.h>
#include <cstdlib>
#include <ctime>
#include <random>
#include <stdio.h>
#include <vector>
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <SDL_opengles2.h>
#else
#include <SDL_opengl.h>
#endif

// This example can also compile and run with Emscripten! See
// 'Makefile.emscripten' for details.
// TODO fix the path
#ifdef __EMSCRIPTEN__
#include "./libs/emscripten/emscripten_mainloop_stub.h"
#endif

enum Randoms {
    MT19937,
    RAND,
    PLUS_PLUS_PLUS,
    BINARY_INT,
    BINARY_FLOAT,
    NUM_OPTIONS, // length
};

int rand_random(int low, int high) {
    static const bool _ = (srand(time(nullptr)), true);
    (void)_;

    return low + (rand() % (high - low + 1));
}

int mt19937_random(int low, int high) {
    static std::random_device       rd;
    static std::mt19937             gen(rd());
    std::uniform_int_distribution<> dis(low, high);

    return dis(gen);
}

bool mt19937_coinflip() {
    return mt19937_random(0, 1);
}

// Main code
int main(int, char **) {
    // Setup SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0) {
        printf("Error: %s\n", SDL_GetError());
        return -1;
    }

    // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
    const char *glsl_version = "#version 100";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(__APPLE__)
    // GL 3.2 Core + GLSL 150
    const char *glsl_version = "#version 150";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS,
                        SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
    // GL 3.0 + GLSL 130
    const char *glsl_version = "#version 130";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

    // From 2.0.18: Enable native IME.
#ifdef SDL_HINT_IME_SHOW_UI
    SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif

    // Create window with graphics context
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_WindowFlags window_flags =
        (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window *window = SDL_CreateWindow("Random Visualizer", SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
    if (window == nullptr) {
        printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
        return -1;
    }

    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    // SDL_GL_SetSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;     // Enable Docking
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport

    // io.ConfigViewportsNoAutoMerge = true;
    // io.ConfigViewportsNoTaskBarIcon = true;

    ImGui::StyleColorsLight();

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform
    // windows can look identical to regular ones.
    ImGuiStyle &style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding              = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Main loop
    bool done = false;

#ifdef __EMSCRIPTEN__
    // For an Emscripten build we are disabling file-system access, so let's not
    // attempt to do a fopen() of the imgui.ini file. You may manually call
    // LoadIniSettingsFromMemory() to load settings from your own storage.
    io.IniFilename = nullptr;
    EMSCRIPTEN_MAINLOOP_BEGIN
#else
    while (!done)
#endif
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to
        // tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to
        // your main application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input
        // data to your main application, or clear/overwrite your copy of the
        // keyboard data. Generally you may always pass all inputs to dear imgui,
        // and hide them from your application based on those two flags.
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE &&
                event.window.windowID == SDL_GetWindowID(window))
                done = true;
        }
        if (SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED) {
            SDL_Delay(10);
            continue;
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        static bool               isActive     = true;
        static bool               isActiveOnce = false;
        static int                trials       = 500;
        static std::vector<float> values(1000);
        static int                numValues = 50;
        static Randoms            selected  = Randoms::MT19937;

        // ImGui::SetNextWindowSize(io.DisplaySize);
        // ImGui::SetNextWindowPos(ImVec2(0, 0));
        // ImGui::SetNextWindowViewport(ImGui::GetMainViewport()->ID);
        // ImGui::Begin("Main Window", nullptr,
        //              ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
        //                  ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
        //
        // // Create a dockspace in the main window
        // ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        // ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
        //
        // ImGui::End();

        // Docking logic
        static bool first_time = true;

        if (first_time) {
            // ImGui::SetNextWindowPos(ImVec2(0, 0));                   // Top-left
            // ImGui::SetNextWindowSize(ImVec2(300, io.DisplaySize.y)); // Left docked
            // window ImGui::SetNextWindowDockID(dockspace_id,
            //                            ImGuiCond_Always); // Dock it to the left
        }
        // ImGui::SetNextWindowFocus();

        // SETTINGS WINDOW
        {

            static const char *options[] = {"mt19937", "rand", "plus plus pls",
                                            "binary split (ints)", "binary split (float)"};

            static_assert(sizeof(options) / sizeof(options[0]) == Randoms::NUM_OPTIONS,
                          "Number of random options labels does not match number of randoms");

            ImGui::Begin("Options");

            ImGui::Checkbox("Active", &isActive);

            if (ImGui::ListBox("Random Option", reinterpret_cast<int *>(&selected), options,
                               Randoms::NUM_OPTIONS) ||
                ImGui::SliderInt("Trials", &trials, 1, 50000, "%d", ImGuiSliderFlags_AlwaysClamp) ||
                ImGui::SliderInt("NumValues", &numValues, 5, 1000, "%d",
                                 ImGuiSliderFlags_AlwaysClamp)) {
                isActiveOnce = true;
                for (int i = 0; i < numValues; i++) {
                    values[i] = 0;
                }
            }

            ImGui::End();
        }

        // PLOT WINDOW
        {
            if (first_time) {
                // ImGui::SetNextWindowPos(ImVec2(300, 0)); // Next to the left window
                // ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x - 300, io.DisplaySize.y));
                // ImGui::SetNextWindowDockID(dockspace_id, ImGuiCond_Always);
            }

            // ImGui::SetNextWindowFocus();
            // values.reserve(numValues);

            ImGui::Begin("Plot");

            // if active, update/add to the values
            if (isActive || isActiveOnce) {
                switch (selected) {
                case MT19937:
                    for (int i = 0; i < trials; i++) {
                        values[mt19937_random(1, numValues - 2)]++;
                    }
                    break;
                case RAND:
                    for (int i = 0; i < trials; i++) {
                        values[rand_random(1, numValues - 2)]++;
                    }
                    break;
                case PLUS_PLUS_PLUS:
                    for (int i = 0; i < trials; i++) {
                        int t = 0;
                        for (int j = 0; j < numValues - 1; j++) {
                            t += mt19937_coinflip();
                        }
                        values[t]++;
                    }
                    break;
                case BINARY_INT:
                    for (int i = 0; i < trials; i++) {
                        int low  = 0;
                        int high = numValues - 1;
                        while (low != high) {
                            int mid = (high + low) / 2;
                            if ((high + low) % 2 == 1) {
                                mid += mt19937_coinflip();
                            }
                            if (mt19937_coinflip()) {
                                low = mid;
                            } else {
                                high = mid;
                            }
                        }
                        values[low]++;
                    }
                    break;
                case BINARY_FLOAT:
                    for (int i = 0; i < trials; i++) {
                        double low  = 1;
                        double high = numValues - 1;
                        while (std::abs(low - high) > 0.1) {
                            double mid = (high + low) / 2;
                            if (mt19937_coinflip()) {
                                low = mid;
                            } else {
                                high = mid;
                            }
                        }
                        values[low]++;
                    }
                    break;
                case NUM_OPTIONS:
                    // shouldn't be possible
                    break;
                }
            }

            isActiveOnce = false;

            ImVec2 custom_graph_size = ImGui::GetWindowSize();

            custom_graph_size.x -= 20;
            custom_graph_size.y -= 100;

            ImGui::PlotLines("", values.data(), numValues, 0, nullptr, FLT_MAX, FLT_MAX,
                             custom_graph_size);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate,
                        io.Framerate);

            ImGui::End();
        }

        if (first_time) {
            first_time = false;
        }

        // Rendering
        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w,
                     clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Update and Render additional Platform Windows
        // (Platform functions may change the current OpenGL context, so we
        // save/restore it to make it easier to paste this code elsewhere.
        //  For this specific demo app we could also call SDL_GL_MakeCurrent(window,
        //  gl_context) directly)
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            SDL_Window   *backup_current_window  = SDL_GL_GetCurrentWindow();
            SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
        }

        SDL_GL_SwapWindow(window);
    }
#ifdef __EMSCRIPTEN__
    EMSCRIPTEN_MAINLOOP_END;
#endif

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
