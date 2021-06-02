#pragma once
#include <exception>
#include <sstream>
#include <string>

class WebServException : public std::exception {
 public:
  WebServException(const std::string &msg) {
    message = msg;
  }

  virtual const char *what() const throw() {
    return message.c_str();
  }

  virtual ~WebServException() throw() {}

  WebServException(const WebServException &o) throw() {
    this->message = o.message;
  }

 private:
  std::string message;

 public:
  static const char *FATAL_ERROR;
  static const char *RUNTIME_ERROR;
  static const char *SELECT_ERROR;
  static const char *BAD_LISTENER_FD;
  static const char *BIND_ERROR;
  static const char *LISTEN_ERROR;
  static const char *ACCEPT_ERROR;
  static const char *FCNTL_ERROR;
  static const char *RECV_ERROR;
  static const char *NON_BLOCK_ERROR;
};

const char *WebServException::FATAL_ERROR = "FATAL ERROR";
const char *WebServException::RUNTIME_ERROR = "RUNTIME ERROR";
const char *WebServException::BAD_LISTENER_FD = "Bad listener fd error";
const char *WebServException::BIND_ERROR = "Bind error";
const char *WebServException::LISTEN_ERROR = "Listen error";
const char *WebServException::SELECT_ERROR = "Select error";
const char *WebServException::ACCEPT_ERROR = "Accept error";
const char *WebServException::FCNTL_ERROR = "Fcntl error";
const char *WebServException::RECV_ERROR = "Recv error";
const char *WebServException::NON_BLOCK_ERROR = "Non block error";