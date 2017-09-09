/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   helpers.h
 * Author: reat
 *
 * Created on August 20, 2017, 2:37 PM
 */

#ifndef HELPERS_H
#define HELPERS_H

#include <string>

#include "http_structs.h"

typedef void(*failed_handler)(int, http_response&);

void NotFound(int http_method, http_response& res);

void BadRequest(int http_method, http_response& res);

void SendResponse(http_response& res, const std::string& data);

void SendOk(http_response& res);

bool GetInt(const char* val, int* output);

bool GetUint32(const char* val, uint32_t* output);

#endif /* HELPERS_H */

