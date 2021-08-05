#pragma once

#include <thread>

namespace ImNodes {
struct CanvasState;
}

namespace epi::cycle {

struct Graph {

  void Visualize();
  void Join();

private:
  std::thread thread;

  void* window;
  void* gl_context;

  void InitSDL();
  void InitImGui();
  void VisualizeImpl();
  void Render(ImNodes::CanvasState& canvas) const;
};

}
