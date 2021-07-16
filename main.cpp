#include <iostream>

#include "file/File.h"
#include "tokenizer/Tokenizer.h"


int main() {
  auto file = epi::File("examples/tokenization/hello_world.c");
  auto tokenizer = epi::Tokenizer(file);

  while (!tokenizer.EOS()) {
    std::cout << tokenizer.Get()->ToString() << std::endl;
  }

  return 0;
}
