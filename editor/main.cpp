#include "nysor/runtime.hpp"

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imnodes.h>

#include <array>
#include <iostream>
#include <string>

namespace {

struct EditorState {
  nysor::Graph graph;
  nysor::ValidationResult validation;
  double output = 0.0;
};

void make_sample(EditorState& state) {
  state.graph = {};
  const auto ten = state.graph.add_constant(10.0);
  const auto five = state.graph.add_constant(5.0);
  const auto sum = state.graph.add_binary(nysor::BlockKind::Add, ten, five);
  const auto two = state.graph.add_constant(2.0);
  const auto product = state.graph.add_binary(nysor::BlockKind::Multiply, sum, two);
  state.graph.add_output(product);
  state.validation = nysor::validate(state.graph);
}

const char* block_name(nysor::BlockKind kind) {
  switch (kind) {
    case nysor::BlockKind::Input: return "Input";
    case nysor::BlockKind::Constant: return "Constant";
    case nysor::BlockKind::Add: return "Add";
    case nysor::BlockKind::Subtract: return "Subtract";
    case nysor::BlockKind::Multiply: return "Multiply";
    case nysor::BlockKind::Divide: return "Divide";
    case nysor::BlockKind::Output: return "Output";
  }
  return "Unknown";
}

void draw_graph(const EditorState& state) {
  ImNodes::BeginNodeEditor();
  const std::array<ImVec2, 6> positions = {
      ImVec2(80.0f, 100.0f), ImVec2(80.0f, 280.0f), ImVec2(330.0f, 180.0f),
      ImVec2(330.0f, 390.0f), ImVec2(580.0f, 180.0f), ImVec2(850.0f, 180.0f)};

  for (const auto& node : state.graph.nodes()) {
    ImNodes::SetNodeGridSpacePos(static_cast<int>(node.id), positions[node.id]);
    ImNodes::BeginNode(static_cast<int>(node.id));
    ImNodes::BeginNodeTitleBar();
    ImGui::TextUnformatted(block_name(node.kind));
    ImNodes::EndNodeTitleBar();

    if (node.kind == nysor::BlockKind::Constant) {
      ImGui::Text("%.2f", node.constant_value);
    } else {
      for (std::size_t input = 0; input < node.inputs.size(); ++input) {
        ImNodes::BeginInputAttribute(static_cast<int>(node.id * 10 + input));
        ImGui::Text("in %zu", input + 1);
        ImNodes::EndInputAttribute();
      }
    }
    if (node.kind != nysor::BlockKind::Constant) {
      ImNodes::BeginOutputAttribute(static_cast<int>(node.id * 10 + 9));
      ImGui::Text("out");
      ImNodes::EndOutputAttribute();
    }
    ImNodes::EndNode();
  }

  for (const auto& node : state.graph.nodes()) {
    for (std::size_t input = 0; input < node.inputs.size(); ++input) {
      ImNodes::Link(static_cast<int>(node.id * 100 + input),
            static_cast<int>(node.inputs[input] * 10 + 9),
                    static_cast<int>(node.id * 10 + input));
    }
  }
  ImNodes::EndNodeEditor();
}

}  // namespace

int main() {
  if (!glfwInit()) return 1;
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  GLFWwindow* window = glfwCreateWindow(1280, 760, "Nysor Editor", nullptr, nullptr);
  if (!window) {
    glfwTerminate();
    return 1;
  }
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImNodes::CreateContext();
  ImGui::StyleColorsDark();
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init("#version 330");

  EditorState state;
  make_sample(state);
  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Nysor Editor");
    ImGui::BeginChild("Blocks", ImVec2(170.0f, 0.0f), true);
    ImGui::TextUnformatted("Blocks");
    ImGui::Separator();
    for (const auto kind : {nysor::BlockKind::Input, nysor::BlockKind::Constant,
                nysor::BlockKind::Add,
                            nysor::BlockKind::Subtract, nysor::BlockKind::Multiply,
                            nysor::BlockKind::Divide, nysor::BlockKind::Output}) {
      ImGui::Selectable(block_name(kind), false);
    }
    if (ImGui::Button("Reset sample")) make_sample(state);
    if (ImGui::Button("Execute")) {
      if (state.validation.valid()) state.output = nysor::execute(nysor::lower_to_ir(state.graph));
    }
    ImGui::EndChild();
    ImGui::SameLine();
    ImGui::BeginChild("Graph", ImVec2(0.0f, 0.0f), true);
    draw_graph(state);
    ImGui::EndChild();
    ImGui::End();

    ImGui::Begin("Inspector");
    ImGui::Text("Nodes: %zu", state.graph.nodes().size());
    ImGui::Text("Output: %.2f", state.output);
    ImGui::TextColored(state.validation.valid() ? ImVec4(0.3f, 1.0f, 0.4f, 1.0f)
                                                 : ImVec4(1.0f, 0.3f, 0.3f, 1.0f),
                       state.validation.valid() ? "Valid graph" : "Invalid graph");
    for (const auto& error : state.validation.errors) ImGui::TextWrapped("%s", error.c_str());
    ImGui::End();

    ImGui::Render();
    int width = 0, height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
    glClearColor(0.055f, 0.065f, 0.08f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window);
  }

  ImNodes::DestroyContext();
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}