#pragma once

/*
 * Some standard exceptions
 * */

#include <stdexcept>
#include <string>


namespace epi::cotyl {

struct Exception : public std::runtime_error {
  Exception(std::string&& title, std::string&& body) : 
      std::runtime_error(std::move(body)), title{std::move(title)} { }

  std::string title;
};

struct UnreachableException : Exception {
  UnreachableException() : 
      Exception("Unreachable", "This code path is unreachable...") { }
};

struct UnimplementedException : Exception {
  UnimplementedException(std::string&& message) : 
      Exception("Unimplemented", std::move(message)) { }
  
  UnimplementedException() : 
      UnimplementedException("This action is not implemented") { }
};

}