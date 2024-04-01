#include "Cycle.h"

#include "Format.h"
#include "Containers.h"

#include <SDL.h>
#include <imgui/imgui.h>
#include <imgui_impl_sdl.h>
#include <imgui_impl_opengl3.h>
#include <imnodes/ImNodesEz.h>
#include <glad/glad.h>
#include <thread>

namespace epi::cycle {

void VisualGraph::Visualize(NodeSort sort) {
  thread = std::thread(&VisualGraph::VisualizeImpl, this, sort);
}

void VisualGraph::Join() {
  thread.join();
}

void VisualGraph::InitSDL() {
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0) {
    throw cotyl::FormatExcept("Error: %s", SDL_GetError());
  }
  // Decide GL+GLSL versions
#if __APPLE__
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
#else
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
#endif

  // Create window with graphics context
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

  auto window_flags = (SDL_WindowFlags) (SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE |
                                         SDL_WINDOW_ALLOW_HIGHDPI);
  window = SDL_CreateWindow("Graph", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height,
                            window_flags);

  gl_context = SDL_GL_CreateContext((SDL_Window*)window);
  SDL_GL_MakeCurrent((SDL_Window*)window, gl_context);
  SDL_GL_SetSwapInterval(1); // Enable vsync
}

void VisualGraph::InitImGui() {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();

  // io = &ImGui::GetIO();
  //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
  //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();

  // Initialize OpenGL loader
  bool err = gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress) == 0;
  if (err)
  {
    throw std::runtime_error("Failed to initialize OpenGL loader!");
  }

  // Setup Platform/Renderer bindings
  const char *glsl_version = "#version 130";
  ImGui_ImplSDL2_InitForOpenGL((SDL_Window*)window, gl_context);
  ImGui_ImplOpenGL3_Init(glsl_version);
}

cotyl::unordered_map<u64, ImVec2> SortPositions(const Graph<u64, VisualGraph::VisualNode>& graph) {
  auto sort = graph.template LayeredTopSort<false>();
  cotyl::unordered_map<u64, ImVec2> positions{};
  positions.reserve(graph.size());

  ImVec2 pos{50, 100};
  for (const auto& layer : sort) {
    pos.y = 100;
    pos.x += 300;
    for (const auto& id : layer) {
      positions.emplace(id, pos);
      pos.y += (1 + graph.At(id).value.body.size()) * 20 + 30;
    }
  }
  return positions;
}

cotyl::unordered_map<u64, ImVec2> CirclePositions(const Graph<u64, VisualGraph::VisualNode>& graph) {
  cotyl::unordered_map<u64, ImVec2> positions{};
  const size_t num_nodes = graph.size();
  constexpr double radius = 1000;
  positions.reserve(num_nodes);
  
  double theta = 0;
  for (const auto& [node_id, node] : graph) {
    ImVec2 pos{
      (float)(radius * std::cos(theta)),
      (float)(radius * std::sin(theta))
    };
    positions.emplace(node_id, pos);
    theta += 2 * 3.14159265 / num_nodes;
  }
  return positions;
}


void VisualGraph::VisualizeImpl(NodeSort sort) {
  cotyl::unordered_map<u64, ImVec2> positions;
  switch (sort) {
    case NodeSort::Topological: {
      positions = SortPositions(graph);
      break;
    }
    case NodeSort::Circle: {
      positions = CirclePositions(graph);
      break;
    }
  }

  InitSDL();
  InitImGui();

  auto canvas = std::make_unique<ImNodes::CanvasState>();
  canvas->Style.CurveThickness = 1.0f;
  canvas->Style.ConnectionIndent = 0.0f;

  while (true) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      ImGui_ImplSDL2_ProcessEvent(&event);

      switch (event.type) {
        case SDL_WINDOWEVENT: {
          switch (event.window.event) {
            case SDL_WINDOWEVENT_RESIZED:
            case SDL_WINDOWEVENT_SIZE_CHANGED: {
              width = event.window.data1;
              height = event.window.data2;
              break;
            }
            default:
              break;
          }
          break;
        }
        case SDL_QUIT: {
          // Cleanup
          ImGui_ImplOpenGL3_Shutdown();
          ImGui_ImplSDL2_Shutdown();
          ImGui::DestroyContext();

          SDL_QuitSubSystem(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER);
          SDL_GL_DeleteContext(gl_context);
          SDL_DestroyWindow((SDL_Window*)window);
          SDL_Quit();
          return;
        }
        default:
          break;
      }
    }

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame((SDL_Window*)window);
    ImGui::NewFrame();

    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    Render(*canvas, positions);

    ImGui::Render();

    // then draw the imGui stuff over it
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // frameswap
    SDL_GL_SwapWindow((SDL_Window*)window);
  }
}

void VisualGraph::Render(ImNodes::CanvasState& canvas, cotyl::unordered_map<u64, ImVec2>& positions) {
  ImGui::SetNextWindowSize(ImVec2(width, height));
  ImGui::SetNextWindowPos(ImVec2(0, 0));
  if (ImGui::Begin(
          "ImNodes",
          nullptr,
          ImGuiWindowFlags_NoScrollbar
        | ImGuiWindowFlags_NoResize
        | ImGuiWindowFlags_NoMove))
  {
    ImNodes::BeginCanvas(&canvas);

    ImNodes::Ez::SlotInfo input = {"", 1};
    for (auto& [node_id, node] : graph) {
      auto& vnode = node.value;
      const std::string title = vnode.title.empty() ? std::to_string(vnode.id) : vnode.title;

      if (ImNodes::Ez::BeginNode((void*)vnode.id, title.c_str(), &positions.at(node_id), &vnode.selected)) {
        if (!node.from.empty()) [[likely]] {
          // blocks only contain one input
          ImNodes::Ez::InputSlots(&input, 1);
        }
        else {
          ImNodes::Ez::InputSlots(nullptr, 0);
        }

        for (const auto& line : vnode.body) {
          ImGui::Text(line.c_str());
        }

        if (!node.to.empty()) [[likely]] {
          std::vector<ImNodes::Ez::SlotInfo> outputs{};
          for (const auto& output : vnode.output_set) {
            outputs.push_back({output.c_str(), 1});
          }
          ImNodes::Ez::OutputSlots(outputs.data(), outputs.size());
        }
        else {
          ImNodes::Ez::OutputSlots(nullptr, 0);
        }

        ImNodes::Ez::EndNode();
      }
    }

    for (const auto& [node_id, node] : graph) {
      const auto& vnode = node.value;
      for (const auto& to_id : node.to) {
        ImNodes::Connection((void*)to_id, "", (void*)node_id, vnode.outputs.at(to_id).c_str());
      }
    }
    ImNodes::EndCanvas();
  }
  ImGui::End();
}

}