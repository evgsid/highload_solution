/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   http_response.h
 * Author: reat
 *
 * Created on August 24, 2017, 7:29 PM
 */

#ifndef HTTP_STRUCTS_H
#define HTTP_STRUCTS_H

#include <string>
#include <sys/uio.h>

#include "constants.h"
#include "yuarel.h"

struct http_response {
  int code;
  iovec iov[MAX_IOVEC_SIZE];
  size_t iov_size = 0;

  char body_size_buf[6];
  char avg_buf[8];
};

struct http_request {
  yuarel_param url_params[MAX_QUERY_SIZE];
  int params_size = 0;
  const char* body = nullptr;
  size_t body_length = 0;
};

#endif /* HTTP_STRUCTS_H */

