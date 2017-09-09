#include "helpers.h"
#include "http_parser.h"

namespace {

const std::string empty_body("{}");

}

void NotFound(int http_method, http_response& res) {
  static const std::string not_found_post("HTTP/1.1 404 Not Found\r\n"
                                          "S: b\r\n"
                                          "C: k\r\n"
                                          "B: a\r\n"
                                          "Content-Length: 0\r\n"
                                          "\r\n");
  static const std::string not_found_get("HTTP/1.1 404 Not Found\r\n"
                                         "S: b\r\n"
                                         "C: k\r\n"
                                         "B: a\r\n"
                                         "Content-Length: 0\r\n"
                                         "\r\n");
  if (http_method == HTTP_POST) {
    res.iov[0].iov_base = (void*)not_found_post.c_str();
    res.iov[0].iov_len = not_found_post.length();
  } else {
    res.iov[0].iov_base = (void*)not_found_get.c_str();
    res.iov[0].iov_len = not_found_get.length();
  }
  res.iov_size = 1;
}

void BadRequest(int http_method, http_response& res) {
  static const std::string bad_data_post("HTTP/1.1 400 Bad Request\r\n"
                                         "S: b\r\n"
                                         "C: k\r\n"
                                         "B: a\r\n"
                                         "Content-Length: 0\r\n"
                                         "\r\n");
  static const std::string bad_data_get("HTTP/1.1 400 Bad Request\r\n"
                                        "S: b\r\n"
                                        "C: k\r\n"
                                        "B: a\r\n"
                                        "Content-Length: 0\r\n"
                                        "\r\n");
  if (http_method == HTTP_POST) {
    res.iov[0].iov_base = (void*)bad_data_post.c_str();
    res.iov[0].iov_len = bad_data_post.length();
  } else {
    res.iov[0].iov_base = (void*)bad_data_get.c_str();
    res.iov[0].iov_len = bad_data_get.length();
  }
  res.iov_size = 1;
}

void SendResponse(http_response& res, const std::string& data) {
  res.iov[0].iov_base = (void*)data.c_str();
  res.iov[0].iov_len = data.length();
  res.iov_size = 1;
}

void SendOk(http_response& res) {
  static const std::string ok = "HTTP/1.1 200 OK\r\n"
                                "S: b\r\n"
                                "C: k\r\n"
                                "B: a\r\n"
                                "Content-Length: 2\r\n"
                                "\r\n"
                                "{}";
  res.iov[0].iov_base = (void*)ok.c_str();
  res.iov[0].iov_len = ok.length();
  res.iov_size = 1;
}

bool GetInt(const char* val, int* output) {
  int base = 1;
  uint32_t number = 0;
  int sign = 1;
  if (*val == '-') {
    sign = -1;
    val++;
  }
  if (!GetUint32(val, &number)) {
    return false;
  }
  *output = number * sign;
  return true;
}

bool GetUint32(const char* val, uint32_t* output) {
  int base = 10;
  int number = 0;
  while (*val) {
    if (*val < '0' || *val > '9') {
      return false;
    }
    number = number * base + (*val - '0');
    val++;
  }
  *output = number;
  return true;
}
