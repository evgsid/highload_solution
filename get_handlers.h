/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   get_handlers.h
 * Author: reat
 *
 * Created on August 20, 2017, 11:01 AM
 */

#ifndef GET_HANDLERS_H
#define GET_HANDLERS_H

#include "http_structs.h"

namespace get_handlers {

void GetUsers(const http_request& req, http_response& res, uint32_t id);
void GetLocations(const http_request& req, http_response& res, uint32_t id);
void GetVisits(const http_request& req, http_response& res, uint32_t id);
void GetUsersVisits(const http_request& req, http_response& res, uint32_t id);
void GetLocationsAvg(const http_request& req, http_response& res, uint32_t id);

}  // namespace get_handlers

#endif /* GET_HANDLERS_H */

