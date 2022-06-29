#include "File.h"

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
  const std::streampos current = file.tellg() - (std::streampos)buf.size();
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
  if (file.eof()) {
    return true;
  }
  char value;
  file.read(&value, 1);
  if (!value && file.eof()) {
    return true;
  }
  if (value == '\n') {
    prev = line;
    line = file.tellg();
  }

  buf.push_back(value);
  return file.eof();
}

char File::GetNew() {
  char value;
  file.read(&value, 1);

  if (value == '\n') {
    prev = line;
    line = file.tellg();
  }
  if (!value && file.eof()) {
    throw std::runtime_error("Unexpected end of file");
  }
  return value;
}

}