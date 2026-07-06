#ifndef GUI_H
#define GUI_H

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_opengl3.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>

#ifdef __EMSCRIPTEN__
#include "../libs/emscripten/emscripten_mainloop_stub.h"
#endif

#include "camera.h"
#include "hittable.h"
#include "hittable_list.h"
#include "scene.h"
#include "multi_thread.h"

#include <thread>
#include <mutex>

class GUI_Handler{
  public:

    GUI_Handler(Camera &cam, Scene &scene, char *output_file) :
      cam(cam),
      scene(scene),
      filename(output_file)
    {}

    int SETUP() {
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
      SDL_Window* program_window = SDL_CreateWindow("PBR Ray Tracer", (int)(1200 * main_scale), (int)(800 * main_scale), window_flags);

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
    Camera &cam;
    char *filename;

// ALL THREADING DECLARATIONS
// ----------------------------
    std::atomic_bool render_started = false;
    std::atomic_bool render_cancelled = false;
    std::thread renderer_thread;
// ----------------------------


    void Object_Outliner(const ImGuiViewport *viewport, ImGuiWindowFlags &window_flags) {
      window_flags |= ImGuiWindowFlags_NoCollapse;
      window_flags |= ImGuiWindowFlags_NoMove;

      ImVec2 pos = ImVec2(viewport->WorkPos.x + viewport->WorkSize.x,
                          viewport->WorkPos.y);
      ImGui::SetNextWindowPos(pos, ImGuiCond_Always, ImVec2(1.0f, 0.0f));
      ImGui::SetNextWindowSize(ImVec2(0.2f * viewport->WorkSize.x, 0.5f * viewport->WorkSize.y), ImGuiCond_Once);

      ImGui::Begin("Object outline", 0, window_flags);
      if(ImGui::Button("Add an object", ImVec2(5.0f, 2.0f))) {

      }

      // Object list
      {
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);

        ImGui::BeginChild("##objectlist", ImVec2(0, INT_MAX), ImGuiChildFlags_Borders, window_flags);
        ImGui::EndChild();
        ImGui::PopStyleVar();
      }
      ImGui::End();
    }

    void Camera_Settings(ImGuiViewport *viewport, ImGuiWindowFlags &window_flags){

      thread_guard_condition guard_renderer(renderer_thread, render_started);

      window_flags |= ImGuiWindowFlags_NoCollapse;
      window_flags |= ImGuiWindowFlags_NoMove;

      ImVec2 pos = ImVec2(viewport->WorkPos.x + viewport->WorkSize.x,
                          viewport->WorkPos.y + 0.5f * viewport->WorkSize.y);

      ImGui::SetNextWindowPos(pos, ImGuiCond_Always, ImVec2(1.0f, 0.0f));
      ImGui::SetNextWindowSize(ImVec2(0.2f * viewport->WorkSize.x, 0.5f * viewport->WorkSize.y), ImGuiCond_Once);

      ImGui::Begin("Camera settings", 0, window_flags);

      ImGui::Text("Image Width");
      ImGui::InputInt("##imWidth", &cam.image_width);

      ImGui::Text("Aspect Ratio");
      ImGui::InputFloat("##aspectRatio", &cam.aspect_ratio);

      ImGui::Text("EnableAA"); ImGui::SameLine();
      ImGui::Checkbox("##Enable AA", &cam.enableAA);

      ImGui::Text("Field of View");
      ImGui::InputFloat("##camfov", &cam.verticalFOV, 10.0f, 160.0f);

      ImGui::Text("Camera position");
      ImGui::SliderFloat3("##campos", cam.look_from.e, -10000, 10000);

      ImGui::Text("Camera look at");
      ImGui::SliderFloat3("##camlookat", cam.look_at.e, -10000, 10000);

      ImGui::Text("Max bounces");
      ImGui::InputInt("##maxbounces", &cam.max_bounces);

      ImGui::Text("Ray sample count");
      ImGui::InputInt("##samplecount", &cam.sample_per_pixel);

      // ImGui::InputFloat()
      ImGui::Dummy(ImVec2(0.0f, 80.0f));
      ImGui::Text("Output file path:");
      ImGui::InputText("##outputfile", filename, 256);


      ImGui::BeginDisabled(render_started);
      if (ImGui::Button("RENDER")) {

        render_started = true;
        render_cancelled = false;
        Camera newRenderCam = cam;

        renderer_thread = std::thread(
          &GUI_Handler::Start_Render,
          this,
          std::ref(render_started),
          std::ref(render_cancelled)
        );
      }

      ImGui::EndDisabled();

      ImGui::SameLine();

      ImGui::BeginDisabled(!render_started);
      if (ImGui::Button("CANCEL RENDER")) {
        render_cancelled = true;
      }
      ImGui::EndDisabled();

      ImGui::End();
    }


    void Start_Render(std::atomic_bool &render_started, std::atomic_bool &render_cancelled) {
      Camera RenderCam = cam;
      Scene RenderScene = scene;
      std::string RenderFilename = filename;


      std::mutex output_mutex;

      image output_image;

      RenderCam.Render_MultiThreaded(RenderScene, RenderFilename, render_cancelled, output_image);

      RenderCam.WriteImageToFile(output_image, RenderFilename);

      render_started = false;

    }

};



#endif