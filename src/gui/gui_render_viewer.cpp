#include "gui_setup.h"

void GUI_Handler::Render_Viewer(ImGuiViewport *viewport, ImGuiWindowFlags &window_flags) {
  window_flags |= ImGuiWindowFlags_NoCollapse;
  window_flags |= ImGuiWindowFlags_NoMove;

  ImVec2 pos = ImVec2(viewport->WorkPos.x, viewport->WorkPos.y);
  ImGui::SetNextWindowPos(pos, ImGuiCond_Always, ImVec2(0.0f, 0.0f));
  ImGui::SetNextWindowSize(ImVec2(0.8f * viewport->WorkSize.x, viewport->WorkSize.y), ImGuiCond_Always);

  ImGui::Begin("Render Viewer");

  bool is_rendered = LoadRenderedTexture(
    d_imdata,
    RenderCam.render_width,
    RenderCam.render_height
  );

  ImGui::Text("size = %d x %d", RenderCam.render_width, RenderCam.render_height);
  ImGui::Image((ImTextureID)(intptr_t)d_imdata.out_texture, ImVec2(RenderCam.render_width, RenderCam.render_height));
  ImGui::End();

}

void GUI_Handler::SetupRenderViewer() {
  d_imdata.output_image_data = std::vector<float>(RenderCam.render_width * RenderCam.render_height * 4);
}