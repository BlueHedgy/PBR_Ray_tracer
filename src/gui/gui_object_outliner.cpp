#include "gui_setup.h"

void GUI_Handler::Object_Outliner(const ImGuiViewport *viewport, ImGuiWindowFlags &window_flags) {
  window_flags = 0;
  window_flags |= ImGuiWindowFlags_NoCollapse;
  window_flags |= ImGuiWindowFlags_NoMove;

  ImVec2 pos = ImVec2(viewport->WorkPos.x + viewport->WorkSize.x,
                      viewport->WorkPos.y);
  ImGui::SetNextWindowPos(pos, ImGuiCond_Always, ImVec2(1.0f, 0.0f));
  ImGui::SetNextWindowSize(ImVec2(0.2f * viewport->WorkSize.x, 0.5f * viewport->WorkSize.y), ImGuiCond_Always);

  ImGui::Begin("Object outline", 0, window_flags);

  if(ImGui::Button("Add an object")) {
      
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