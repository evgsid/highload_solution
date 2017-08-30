/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   post_handlers.h
 * Author: reat
 *
 * Created on August 20, 2017, 11:04 AM
 */

#ifndef POST_HANDLERS_H
#define POST_HANDLERS_H

#include "http_structs.h"

namespace post_handlers {

void UpdateUsers(const http_request& req, http_response& res, uint32_t id);
void UpdateLocations(const http_request& req, http_response& res, uint32_t id);
void UpdateVisits(const http_request& req, http_response& res, uint32_t id);
void AddUsers(const http_request& req, http_response& res);
void AddLocations(const http_request& req, http_response& res);
void AddVisits(const http_request& req, http_response& res);

}  // namespace post_handlers

#endif /* POST_HANDLERS_H */

