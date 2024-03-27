#include "File.h"

#include "Exceptions.h"

#include <iostream>

namespace epi {

File::File(const std::string& filename) {
  file = std::ifstream(filename, std::ios::binary);
  if (!file.good()) {
    throw std::runtime_error("Failed to open file!");
  }
  file.seekg(0, std::ios::beg);
}

File::~File() {
  file.close();
}

void File::PrintLoc() const {
  const std::streampos restore = file.tellg();
  const std::streampos current = file.tellg() - (std::streampos)BufSize() - (lookahead ? 1 : 0);
  std::string l;

  if (prev != -1) {
    file.seekg(prev, std::ios::beg);
    std::getline(file, l);
    std::cout << l << std::endl;
  }
  else {
    file.seekg(line, std::ios::beg);
  }
  std::getline(file, l);
  std::cout << l << std::endl;
  for (int i = 0; i < current - line; i++) {
    std::cout << '~';
  }
  std::cout << '^' << std::endl;
  std::getline(file, l);
  std::cout << l << std::endl;
  file.seekg(restore);
}

bool File::IsEOS() {
  if (lookahead) {
    return false;
  }
  if (file.eof()) {
    return true;
  }
  file.read(&lookahead, 1);
  return !lookahead && file.eof();
}

char File::GetNew() {
  char value;
  if (lookahead) {
    value = lookahead;
    lookahead = 0;
  }
  else {
    file.read(&value, 1);
  }

  if (value == '\n') {
    prev = line;
    line = file.tellg();
  }
  if (!value && file.eof()) {
    throw cotyl::EndOfFileException();
  }
  return value;
}

}