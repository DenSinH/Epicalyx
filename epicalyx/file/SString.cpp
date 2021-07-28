#include "SString.h"

#include <iostream>


namespace epi {


void SString::PrintLoc() {
  size_t error_pos = position - buf.size();
  std::cout << "..." << string.substr(std::max(0ull, error_pos - 20), 40) << "..." << std::endl;
  for (auto i = 0; i < 3 + std::min(error_pos, 20ull) - 1; i++) {
    // - 1 because we are already advanced past the error the moment we catch it
    std::cout << ' ';
  }
  std::cout << '^' << std::endl;
}

}