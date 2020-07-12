#ifndef _EXCEPTION_HPP_
#define _EXCEPTION_HPP_

#include <stdint.h>

#include <exception>
#include <string>

class NoneOfUidException : public std::exception {
 public:
  NoneOfUidException(uint32_t uid_) : uid(uid_) {}
  const char* what() const throw() { return std::to_string(uid).c_str(); }

 private:
  uint32_t uid;
};

class IncorrectHttpRequestException : public std::exception {
 public:
  IncorrectHttpRequestException(std::string& name_) : name(name_) {}
  const char* what() const throw() { return name.c_str(); }

 private:
  std::string name;
};

class NoneOfHttpRequestHandlerException : public std::exception {
 public:
  NoneOfHttpRequestHandlerException(std::string path_) : path(path_) {}
  const char* what() const throw() { return path.c_str(); }

 private:
  std::string path;
};

#endif  // !_EXCEPTION_HPP_
