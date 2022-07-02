#pragma once

/*
 * Some standard exceptions
 * */

#include <stdexcept>


namespace epi::cotyl {

struct EndOfFileException : public std::runtime_error {
  EndOfFileException() : std::runtime_error("Unexpected end of file") {}
};

struct UnexpectedIdentifierException : public std::runtime_error {
  UnexpectedIdentifierException() : std::runtime_error("Unexpected identifier") {}
};

struct UnimplementedException : public std::runtime_error {
  UnimplementedException() : std::runtime_error("Unimplemented") {}
  UnimplementedException(const std::string& message) : std::runtime_error("Unimplemented: " + message) {}
};

}