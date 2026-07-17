#include "gui_setup.h"
#include <chrono>

void GUI_Handler::Camera_Settings(ImGuiViewport *viewport,  ImGuiWindowFlags &window_flags){
  window_flags = 0;
  window_flags |= ImGuiWindowFlags_NoCollapse;
  window_flags |= ImGuiWindowFlags_NoMove;
  window_flags |= ImGuiWindowFlags_AlwaysAutoResize;

  ImVec2 pos = ImVec2(viewport->WorkPos.x + viewport->WorkSize.x,
                      viewport->WorkPos.y + 0.5f * viewport->WorkSize.y);

  ImGui::SetNextWindowPos(pos, ImGuiCond_Always, ImVec2(1.0f, 0.0f));
  ImGui::SetNextWindowSize(ImVec2(0.2f * viewport->WorkSize.x, 0.5f * viewport->WorkSize.y), ImGuiCond_Always);


  ImGui::Begin("Camera settings", 0, window_flags);

  ImGui::Text("Image Width");
  ImGui::InputInt("##imWidth", &active_cam.image_width);

  ImGui::Text("Aspect Ratio");
  ImGui::InputFloat("##aspectRatio", &active_cam.aspect_ratio);

  ImGui::Text("EnableAA"); ImGui::SameLine();
  ImGui::Checkbox("##Enable AA", &active_cam.enableAA);

  ImGui::Text("Field of View");
  ImGui::InputFloat("##camfov", &active_cam.verticalFOV, 10.0f, 160.0f);

  ImGui::Text("Camera position");
  ImGui::SliderFloat3("##campos", active_cam.look_from.e, -10000, 10000);

  ImGui::Text("Camera look at");
  ImGui::SliderFloat3("##camlookat", active_cam.look_at.e, -10000, 10000);

  ImGui::Text("Max bounces");
  ImGui::InputInt("##maxbounces", &active_cam.max_bounces);

  ImGui::Text("Ray sample count");
  ImGui::InputInt("##samplecount", &active_cam.sample_per_pixel);

  // ImGui::InputFloat()
  // ImGui::Dummy(ImVec2(0.0f, 80.0f));
  ImGui::Text("Output file path:");
  ImGui::InputText("##outputfile", filename, 256);


  ImGui::BeginDisabled(render_started);
  if (ImGui::Button("RENDER")) {

    // Make a copy of the main camera to avoid GUI change affecting the launched render
    RenderCam = active_cam;
    RenderCam.initialize();
    RenderCam.render_width = RenderCam.image_width;
    RenderCam.render_height = RenderCam.image_height;

    std::cout << RenderCam.render_width << " " << RenderCam.render_height << std::endl;
    if (!render_started) SetupRenderViewer();

    render_started = true;
    render_cancelled = false;

    scene.process_object_bvh();
    renderer_thread = std::thread(
      &GUI_Handler::Start_Render,
      this,
      std::ref(render_started),
      std::ref(render_cancelled),
      std::ref(render_done)
    );
  }

  // Check if the renderer thread is done and thus joinable, then join
  thread_guard_condition guard_renderer(renderer_thread, render_started);

  ImGui::EndDisabled();

  ImGui::SameLine();

  ImGui::BeginDisabled(!render_started);
  if (ImGui::Button("CANCEL RENDER")) {
    render_cancelled = true;
  }
  ImGui::EndDisabled();

  ImGui::End();
}

void GUI_Handler::Start_Render(std::atomic_bool &render_started, std::atomic_bool &render_cancelled, std::atomic_bool &render_done) {
  render_done = false;

  std::string RenderFilename = filename;

  image output_image;

  // calculate BVH only at render time to prevent stacked erroneous calculation
  hittable_list bvh_objects = scene.process_object_bvh();

  auto start = std::chrono::high_resolution_clock::now();

  RenderCam.Render_MultiThreaded(scene.get_lights(), bvh_objects , filename, render_cancelled, output_image, d_imdata);

  auto end = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
  std::cout << "Render Time: " << duration.count() << " ms" << std::endl;

  render_started = false;
  render_done = true;

}
