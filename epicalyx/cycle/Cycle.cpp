#include "Cycle.h"

#include "Format.h"

#include <SDL.h>
#include <imgui/imgui.h>
#include <imgui_impl_sdl.h>
#include <imgui_impl_opengl3.h>
#include <imnodes/ImNodesEz.h>
#include <glad/glad.h>
#include <thread>

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

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

  auto window_flags = (SDL_WindowFlags) (SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | // SDL_WINDOW_RESIZABLE |
                                         SDL_WINDOW_ALLOW_HIGHDPI);
  window = SDL_CreateWindow("Graph", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT,
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

    Render(*canvas);

    ImGui::Render();

    // then draw the imGui stuff over it
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // frameswap
    SDL_GL_SwapWindow((SDL_Window*)window);
  }
}

void Graph::Render(ImNodes::CanvasState& canvas) const {
  ImGui::SetNextWindowSize(ImVec2(WINDOW_WIDTH, WINDOW_HEIGHT));
  ImGui::SetNextWindowPos(ImVec2(0, 0));
  if (ImGui::Begin(
          "ImNodes",
          nullptr,
          ImGuiWindowFlags_NoScrollbar
        | ImGuiWindowFlags_NoScrollbar
        | ImGuiWindowFlags_NoResize
        | ImGuiWindowFlags_NoMove))
  {
    ImNodes::BeginCanvas(&canvas);

    struct Node
    {
      ImVec2 pos{};
      bool selected{};
      ImNodes::Ez::SlotInfo inputs[1];
      ImNodes::Ez::SlotInfo outputs[1];
    };

    static Node nodes[3] = {
            {{50, 100}, false, {{"", 1}}, {{"le", 1}}},
            {{500, 50}, false, {{"", 1}}, {{"", 1}}},
            {{250, 100}, false, {{"", 1}}, {{"", 1}}},
    };

    for (Node& node : nodes)
    {
      if (ImNodes::Ez::BeginNode(&node, "Node Title", &node.pos, &node.selected))
      {
        ImNodes::Ez::InputSlots(node.inputs, 1);
        ImGui::Text("Test text");
        ImGui::Text("Test text");
        ImGui::Text("Test text");
        ImNodes::Ez::OutputSlots(node.outputs, 1);
        ImNodes::Ez::EndNode();
      }
    }

    ImNodes::Connection(&nodes[1], "", &nodes[0], "le");
    ImNodes::Connection(&nodes[2], "", &nodes[0], "le");
    ImNodes::Connection(&nodes[1], "", &nodes[2], "");

    ImNodes::EndCanvas();
  }
  ImGui::End();
}

}