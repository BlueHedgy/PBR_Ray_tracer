#ifndef GUI_H
#define GUI_H

#include <thread>
#include <mutex>

#ifdef __EMSCRIPTEN__
#include "../libs/emscripten/emscripten_mainloop_stub.h"
#endif

#include "camera.h"
#include "hittable.h"
#include "hittable_list.h"
#include "scene.h"
#include "multi_thread.h"
#include "gui_image_load.h"
#include <atomic>

class GUI_Handler{
  public:

    GUI_Handler(Scene &scene, char *output_file) :
      scene(scene),
      filename(output_file)
    {}

    int SETUP() {

      int root_window_width = 1200;
      int root_window_height = 800;

      if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD))
      {
          printf("Error: SDL_Init(): %s\n", SDL_GetError());
          return 1;
      }

      // Setup Dear ImGui context

      // Setup Platform/Renderer backends
      const char* glsl_version = "#version 330";
      SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
      SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
      SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
      SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

      SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
      SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
      SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

      float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
      SDL_WindowFlags window_flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN | SDL_WINDOW_HIGH_PIXEL_DENSITY;
      SDL_Window* program_window = SDL_CreateWindow(
        "PBR Ray Tracer",
        (int)(root_window_width * main_scale),
        (int)(root_window_height * main_scale),
        window_flags
      );

      if (program_window == nullptr)
      {
        printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
        return 1;
      }
      SDL_GLContext gl_context = SDL_GL_CreateContext(program_window);
      if (gl_context == nullptr)
      {
        printf("Error: SDL_GL_CreateContext(): %s\n", SDL_GetError());
        return 1;
      }

      SDL_GL_MakeCurrent(program_window, gl_context);
      SDL_GL_SetSwapInterval(1); // Enable vsync
      SDL_SetWindowPosition(program_window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
      SDL_ShowWindow(program_window);

      IMGUI_CHECKVERSION();
      ImGui::CreateContext();
      ImGuiIO& io = ImGui::GetIO();
      io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
      io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
      io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // IF using Docking Branch

      ImGui::StyleColorsDark();

      // Setup scaling
      ImGuiStyle& style = ImGui::GetStyle();
      style.ScaleAllSizes(main_scale);
      style.FontScaleDpi = main_scale;


      ImGui_ImplSDL3_InitForOpenGL(program_window, gl_context);
      ImGui_ImplOpenGL3_Init(glsl_version);

      bool show_demo_window = true;
      bool show_another_window = false;
      ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

      bool done = false;

      while (!done) {
        SDL_Event event;

        while(SDL_PollEvent(&event)) {
          ImGui_ImplSDL3_ProcessEvent(&event);
          if (event.type == SDL_EVENT_QUIT){
            done = true;
          }

          if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED &&
              event.window.windowID == SDL_GetWindowID(program_window)) {
            done = true;
          }
        }

        if (SDL_GetWindowFlags(program_window) & SDL_WINDOW_MINIMIZED) {
          SDL_Delay(10);
          continue;
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();
        ImGui::DockSpaceOverViewport();

        ImGuiViewport *viewport = ImGui::GetMainViewport();
        ImVec2 pos = ImVec2(0, 0);
        ImGuiWindowFlags imgui_window_flags = 0;
        // Demo window
        ImGui::ShowDemoWindow();

        // ACTUAL GUI HERE_____________________________________________________________
        { Object_Outliner(viewport, imgui_window_flags); }
        { Camera_Settings(viewport, imgui_window_flags); }
        { Render_Viewer(viewport, imgui_window_flags); }

        // Rendering
        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(program_window);
      }

      ImGui_ImplOpenGL3_Shutdown();
      ImGui_ImplSDL3_Shutdown();
      ImGui::DestroyContext();

      SDL_GL_DestroyContext(gl_context);
      SDL_DestroyWindow(program_window);
      SDL_Quit();

      return 0;
    }

    private:
    Scene &scene;
    char *filename;

    // ALL RENDER VIEWER DECLARATIONS
    display_image_data d_imdata;
    Camera& active_cam = scene.get_active_cam();
    Camera RenderCam;
// ------------------------------

// ALL THREADING DECLARATIONS
    std::atomic_bool render_started = false;
    std::atomic_bool render_cancelled = false;
    std::atomic_bool render_done = false;
    std::thread renderer_thread;
// ----------------------------


    void Object_Outliner(const ImGuiViewport *viewport, ImGuiWindowFlags &window_flags);

    void Camera_Settings(ImGuiViewport *viewport, ImGuiWindowFlags &window_flags);

    void Start_Render(std::atomic_bool &render_started, std::atomic_bool &render_cancelled, std::atomic_bool &render_done);

    void Render_Viewer(ImGuiViewport *viewport, ImGuiWindowFlags &window_flags);

    void SetupRenderViewer();
};



#endif