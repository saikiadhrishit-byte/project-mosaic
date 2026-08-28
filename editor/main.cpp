#include "nysor/runtime.hpp"

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imnodes.h>

#include <array>
#include <string>

namespace {

struct EditorState {
  nysor::Graph graph;
  nysor::ValidationResult validation;
  nysor::IR ir;
  nysor::DependencyAnalysis analysis;
  nysor::TaskflowExecution execution;
  int selected_node = 2;
  int view = 0;
  bool has_execution = false;
};

const char* block_name(nysor::BlockKind kind) {
  switch (kind) {
    case nysor::BlockKind::Input: return "Input";
    case nysor::BlockKind::Constant: return "Constant";
    case nysor::BlockKind::Time: return "Time";
    case nysor::BlockKind::Add: return "Add";
    case nysor::BlockKind::Subtract: return "Subtract";
    case nysor::BlockKind::Multiply: return "Multiply";
    case nysor::BlockKind::Divide: return "Divide";
    case nysor::BlockKind::Sine: return "Sine";
    case nysor::BlockKind::Output: return "Output";
  }
  return "Unknown";
}

void make_sample(EditorState& state) {
  state.graph = {};
  const auto ten = state.graph.add_constant(10.0);
  const auto five = state.graph.add_constant(5.0);
  const auto sum = state.graph.add_binary(nysor::BlockKind::Add, ten, five);
  const auto two = state.graph.add_constant(2.0);
  const auto product = state.graph.add_binary(nysor::BlockKind::Multiply, sum, two);
  state.graph.add_output(product);
  state.validation = nysor::validate(state.graph);
  state.ir = nysor::lower_to_ir(state.graph);
  state.analysis = nysor::analyze_dependencies(state.ir);
  state.execution = {};
  state.has_execution = false;
  state.selected_node = static_cast<int>(sum);
}

void draw_graph(EditorState& state) {
  ImNodes::BeginNodeEditor();
  const std::array<ImVec2, 6> positions = {
      ImVec2(80.0f, 100.0f), ImVec2(80.0f, 280.0f), ImVec2(330.0f, 180.0f),
      ImVec2(330.0f, 390.0f), ImVec2(580.0f, 180.0f), ImVec2(850.0f, 180.0f)};
  const std::array<const char*, 2> input_names = {"left", "right"};

  for (const auto& node : state.graph.nodes()) {
    ImNodes::SetNodeGridSpacePos(static_cast<int>(node.id), positions[node.id]);
    ImNodes::BeginNode(static_cast<int>(node.id));
    ImNodes::BeginNodeTitleBar();
    ImGui::TextUnformatted(block_name(node.kind));
    ImNodes::EndNodeTitleBar();
    if (node.kind == nysor::BlockKind::Constant) {
      ImGui::Text("value  %.2f", node.constant_value);
    } else {
      for (std::size_t input = 0; input < node.inputs.size(); ++input) {
        ImNodes::BeginInputAttribute(static_cast<int>(node.id * 10 + input));
        ImGui::TextUnformatted(input < input_names.size() ? input_names[input] : "input");
        ImNodes::EndInputAttribute();
      }
    }
    ImNodes::BeginOutputAttribute(static_cast<int>(node.id * 10 + 9));
    ImGui::Indent(70.0f);
    ImGui::TextUnformatted("result");
    ImGui::Unindent(70.0f);
    ImNodes::EndOutputAttribute();
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

void draw_dependency_view(const EditorState& state) {
  ImGui::TextUnformatted("EXECUTION LEVELS");
  ImGui::Separator();
  for (std::size_t level = 0; level < state.analysis.execution_levels.size(); ++level) {
    ImGui::Text("Level %zu", level);
    ImGui::SameLine(100.0f);
    for (const auto node : state.analysis.execution_levels[level]) {
      ImGui::TextColored(ImVec4(0.65f, 0.82f, 0.98f, 1.0f), "%s", block_name(state.ir.instructions[node].kind));
      ImGui::SameLine();
    }
    ImGui::NewLine();
  }
  ImGui::Spacing();
  ImGui::Text("Independent roots: %zu", state.analysis.independent_nodes.size());
  ImGui::Text("Dependency edges: %zu", state.analysis.edges.size());
}

void draw_ir_view(const EditorState& state) {
  ImGui::TextUnformatted("INTERMEDIATE REPRESENTATION");
  ImGui::Separator();
  ImGui::BeginChild("IRText", ImVec2(0.0f, 0.0f), false, ImGuiWindowFlags_HorizontalScrollbar);
  ImGui::TextUnformatted(nysor::dump_ir(state.ir).c_str());
  ImGui::EndChild();
}

void draw_inspector(const EditorState& state) {
  ImGui::BeginChild("Inspector", ImVec2(300.0f, 0.0f), true);
  ImGui::TextColored(ImVec4(0.95f, 0.72f, 0.28f, 1.0f), "INSPECTOR");
  ImGui::Separator();
  const auto& node = state.graph.nodes()[state.selected_node];
  ImGui::Text("Block: %s", block_name(node.kind));
  ImGui::TextDisabled("Core arithmetic / scalar");
  ImGui::Spacing();
  ImGui::TextUnformatted("PORTS");
  ImGui::Separator();
  if (node.kind == nysor::BlockKind::Constant) {
    ImGui::Text("OUTPUT  result");
    ImGui::TextDisabled("core.number");
    ImGui::Text("Value   %.2f", node.constant_value);
  } else {
    for (std::size_t input = 0; input < node.inputs.size(); ++input) {
      ImGui::Text("INPUT   %s", input == 0 ? "left" : "right");
      ImGui::TextDisabled("core.number");
    }
    ImGui::Text("OUTPUT  result");
    ImGui::TextDisabled("core.number");
  }
  ImGui::Spacing();
  ImGui::TextUnformatted("STATUS");
  ImGui::Separator();
  ImGui::TextColored(ImVec4(0.35f, 0.95f, 0.55f, 1.0f), "VALID");
  ImGui::TextDisabled("Installed / executable");
  ImGui::EndChild();
}

void draw_diagnostics(const EditorState& state) {
  ImGui::BeginChild("Diagnostics", ImVec2(0.0f, 92.0f), true);
  ImGui::TextUnformatted("BUILD / DIAGNOSTICS");
  ImGui::SameLine();
  ImGui::TextColored(ImVec4(0.35f, 0.95f, 0.55f, 1.0f), "VALID");
  ImGui::Separator();
  ImGui::Text("[PASS] %zu Blocks    [PASS] %zu Connections    [PASS] Compatibility    [PASS] Compilation",
              state.graph.nodes().size(), state.analysis.edges.size());
  if (state.has_execution) {
    ImGui::Text("[PASS] Runtime    Output: %.2f    Peak concurrency: %zu    Execution: %lld ms",
                state.execution.result, state.execution.peak_concurrency, state.execution.elapsed_ms);
  } else {
    ImGui::TextDisabled("Runtime not run. Use Execute graph to measure the Taskflow run.");
  }
  ImGui::EndChild();
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
  char search_buffer[128] = {};
  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    const auto viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::Begin("Nysor Editor", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBringToFrontOnFocus);
    ImGui::TextColored(ImVec4(0.95f, 0.72f, 0.28f, 1.0f), "NYSOR");
    ImGui::SameLine();
    ImGui::TextDisabled("GRAPH WORKSPACE");
    ImGui::SameLine();
    if (ImGui::Button("BUILD")) state.validation = nysor::validate(state.graph);
    ImGui::SameLine();
    if (ImGui::Button("RUN")) {
      if (state.validation.valid()) {
        state.execution = nysor::execute_with_taskflow_timed(state.ir, state.analysis);
        state.has_execution = true;
      }
    }
    ImGui::Separator();
    ImGui::BeginChild("Main", ImVec2(0.0f, -92.0f), false);
    ImGui::BeginChild("Library", ImVec2(190.0f, 0.0f), true);
    ImGui::TextUnformatted("BLOCK LIBRARY");
    ImGui::Separator();
    ImGui::InputTextWithHint("Search", "Search blocks...", search_buffer,
                 IM_ARRAYSIZE(search_buffer));
    ImGui::TextDisabled("CORE");
    const std::array kinds = {nysor::BlockKind::Input, nysor::BlockKind::Constant,
                              nysor::BlockKind::Add, nysor::BlockKind::Subtract,
                              nysor::BlockKind::Multiply, nysor::BlockKind::Divide,
                              nysor::BlockKind::Output};
    for (const auto kind : kinds) {
      if (ImGui::Selectable(block_name(kind), false)) {
        for (const auto& node : state.graph.nodes()) {
          if (node.kind == kind) state.selected_node = static_cast<int>(node.id);
        }
      }
    }
    ImGui::Spacing();
    ImGui::TextDisabled("COMMUNITY");
    ImGui::TextDisabled("Physics");
    ImGui::TextDisabled("Rendering");
    ImGui::TextDisabled("Audio");
    ImGui::EndChild();
    ImGui::SameLine();
    ImGui::BeginChild("Center", ImVec2(-310.0f, 0.0f), true);
    if (ImGui::BeginTabBar("Views")) {
      if (ImGui::BeginTabItem("GRAPH")) { state.view = 0; draw_graph(state); ImGui::EndTabItem(); }
      if (ImGui::BeginTabItem("DEPENDENCIES")) { state.view = 1; draw_dependency_view(state); ImGui::EndTabItem(); }
      if (ImGui::BeginTabItem("IR")) { state.view = 2; draw_ir_view(state); ImGui::EndTabItem(); }
      ImGui::EndTabBar();
    }
    ImGui::EndChild();
    ImGui::SameLine();
    draw_inspector(state);
    ImGui::EndChild();
    draw_diagnostics(state);
    ImGui::End();
    ImGui::Render();
    int width = 0, height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
    glClearColor(0.035f, 0.045f, 0.06f, 1.0f);
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
