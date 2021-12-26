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

void Graph::Visualize() {
  thread = std::thread(&Graph::VisualizeImpl, this);
}

void Graph::Join() {
  thread.join();
}

void Graph::InitSDL() {
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

void Graph::InitImGui() {
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

void Graph::VisualizeImpl() {
  auto sort = FindOrder();
  cotyl::unordered_map<u64, ImVec2> positions;
  ImVec2 pos{50, 100};
  for (const auto& layer : sort) {
    pos.y = 100;
    pos.x += 300;
    for (const auto& id : layer) {
      positions.emplace(id, pos);
      pos.y += (1 + nodes.at(id).body.size()) * 20 + 30;
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

    Render(*canvas, sort, positions);

    ImGui::Render();

    // then draw the imGui stuff over it
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // frameswap
    SDL_GL_SwapWindow((SDL_Window*)window);
  }
}

Graph::top_sort_t Graph::FindOrder() {
  top_sort_t result{};
  cotyl::unordered_set<u64> todo{};
  result.push_back({});
  for (const auto &[id, node] : nodes) {
    // first layer is only nodes that have no inputs
    if (!edges.contains(id)) {
      result.back().push_back(id);
    }
    else {
      todo.insert(id);
    }
  }

  if (result.back().empty()) [[unlikely]] {
    // possible if only loops exist in fist layer
    result.pop_back();
  }

  while (!todo.empty()) {
    result.push_back({});
    for (const auto& id : todo) {
      // check if all inputs are done
      bool allow_add = true;
      for (const auto& e : edges[id]) {
        if (todo.contains(e.from)) {
          allow_add = false;
          break;
        }
      }

      if (allow_add) {
        result.back().push_back(id);
      }
    }

    // for cycles, pick the node with the least inputs left
    if (result.back().empty()) {
      u32 least_inputs = -1;
      u64 least_id = 0;
      for (const auto& id : todo) {
        u32 inputs = 0;
        for (const auto& e : edges[id]) {
          if (todo.contains(e.from)) {
            inputs++;
          }
        }

        if (inputs < least_inputs) {
          least_inputs = inputs;
          least_id = id;
        }
        else if (inputs == least_inputs) {
          least_id = std::min(least_id, id);
        }
      }

      result.back().push_back(least_id);
    }

    for (const auto& id : result.back()) {
      todo.erase(id);
    }
  }

  return std::move(result);
}

void Graph::Render(ImNodes::CanvasState& canvas, const top_sort_t& sort, cotyl::unordered_map<u64, ImVec2>& positions) {
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
    for (const auto& layer : sort) {
      for (const auto& id : layer) {
        auto& node = nodes.at(id);
        std::string title = std::to_string(node.id);
        if (!node.title.empty()) {
          title += " : " + node.title;
        }
        if (ImNodes::Ez::BeginNode(&node.id, title.c_str(), &positions.at(id), &node.selected))
        {
          if (edges.contains(id)) [[likely]] {
            // blocks only contain one input
            ImNodes::Ez::InputSlots(&input, 1);
          }
          else {
            ImNodes::Ez::InputSlots(nullptr, 0);
          }

          for (const auto& line : node.body) {
            ImGui::Text(line.c_str());
          }

          if (!node.outputs.empty()) [[likely]] {
            std::vector<ImNodes::Ez::SlotInfo> outputs{};
            for (const auto& output : node.outputs) {
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
    }

    for (const auto &[to, to_edges] : edges) {
      for (const auto& edge : to_edges) {
        ImNodes::Connection(&nodes.at(to), "", &nodes.at(edge.from), edge.output.c_str());
      }
    }
    ImNodes::EndCanvas();
  }
  ImGui::End();
}

}