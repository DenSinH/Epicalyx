#include "File.h"


namespace epi {

File::File(std::string filename) {
  file = std::ifstream(filename, std::ios::binary);
  file.seekg(0, std::ios::beg);
}

File::~File() {
  file.close();
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
  buf.push_back(value);
  return file.eof();
}

char File::GetNew() {
  char value;
  file.read(&value, 1);
  if (!value && file.eof()) {
    throw std::runtime_error("Unexpected end of file");
  }

  return value;
}

}