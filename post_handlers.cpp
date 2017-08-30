/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "post_handlers.h"

#include "db_instance.h"
#include "helpers.h"
#include "http_parser.h"
#include "multi_index.h"
#include "simple_index.h"
#include "structures.h"

namespace post_handlers {

void UpdateUsers(const http_request& req, http_response& res, uint32_t id) {
  User* user = g_users_storage->Get(id);
  if (!user) {
    NotFound(HTTP_POST, res);
    return;
  }
  User new_user = *user;
  int update_flags = 0;
  if (!User::Parse(req.body, req.body_length, &new_user, &update_flags)) {
    BadRequest(HTTP_POST, res);
    return;
  }
  if (update_flags & UpdateFlags::UPDATE_EMAIL) {
    g_db_instance->GetEmailIndex()->Replace(
        user->email, new_user.email);
  }
  std::swap(*user, new_user);
  user->Serialize(id);
  SendOk(res);
}

void UpdateLocations(const http_request& req, http_response& res, uint32_t id) {
  Location* location = g_locations_storage->Get(id);
  if (!location) {
    NotFound(HTTP_POST, res);
    return;
  }
  Location new_location = *location;
  int update_flags = 0;
  if (!Location::Parse(req.body, req.body_length,
                       &new_location, &update_flags)) {
    BadRequest(HTTP_POST, res);
    return;
  }
  std::swap(*location, new_location);
  location->Serialize(id);
  SendOk(res);
}

void UpdateVisits(const http_request& req, http_response& res, uint32_t id) {
  Visit* visit = g_visits_storage->Get(id);
  if (!visit) {
    NotFound(HTTP_POST, res);
    return;
  }
  Visit new_visit = *visit;
  int update_flags = 0;
  if (!Visit::Parse(req.body, req.body_length, &new_visit, &update_flags)) {
    BadRequest(HTTP_POST, res);
    return;
  }
  if (update_flags & UpdateFlags::UPDATE_LOCATION) {
    auto* index = g_db_instance->GetLocationsVisitsIndex();
    index->Replace(visit->location_id, new_visit.location_id, visit);
  }
  if (update_flags & UpdateFlags::UPDATE_USER) {
    auto* index = g_db_instance->GetUsersVisitsIndex();
    index->Replace(visit->user_id, new_visit.user_id, visit);
  }
  std::swap(*visit, new_visit);
  visit->Serialize(id);
  SendOk(res);
}

void AddUsers(const http_request& req, http_response& res) {
  User* user = new User;
  int update_flags = 0;
  uint32_t id = 0;
  if (!User::Parse(req.body, req.body_length, user, &update_flags, &id)) {
    BadRequest(HTTP_POST, res);
    return;
  }
  if (update_flags & UPDATE_ID &&
      update_flags & UPDATE_EMAIL &&
      update_flags & UPDATE_FIRST_NAME &&
      update_flags & UPDATE_LAST_NAME &&
      update_flags & UPDATE_GENDER &&
      update_flags & UPDATE_BIRTH_DATE) {
    if (!g_users_storage->Add(id, user, nullptr)) {
      BadRequest(HTTP_POST, res);
      return;
    }
    user->Serialize(id);
  } else {
    BadRequest(HTTP_POST, res);
    return;
  }
  SendOk(res);
}

void AddLocations(const http_request& req, http_response& res) {
  Location* location = new Location;
  int update_flags = 0;
  uint32_t id = 0;
  if (!Location::Parse(req.body, req.body_length,
                       location, &update_flags, &id)) {
    BadRequest(HTTP_POST, res);
    return;
  }
  if (update_flags & UPDATE_ID &&
      update_flags & UPDATE_PLACE &&
      update_flags & UPDATE_COUNTRY &&
      update_flags & UPDATE_CITY &&
      update_flags & UPDATE_DISTANCE) {
    if (!g_locations_storage->Add(
            id, location, nullptr)) {
      BadRequest(HTTP_POST, res);
      return;
    }
    location->Serialize(id);
  } else {
    BadRequest(HTTP_POST, res);
    return;
  }
  SendOk(res);
}

void AddVisits(const http_request& req, http_response& res) {
  Visit* visit = new Visit;
  int update_flags = 0;
  uint32_t id = 0;
  if (!Visit::Parse(req.body, req.body_length, visit, &update_flags, &id)) {
    BadRequest(HTTP_POST, res);
    return;
  }
  if (update_flags & UPDATE_ID &&
      update_flags & UPDATE_LOCATION &&
      update_flags & UPDATE_USER &&
      update_flags & UPDATE_VISITED_AT &&
      update_flags & UPDATE_MARK) {
    Visit* instance = nullptr;
    uint32_t user_id = visit->user_id;
    uint32_t location_id = visit->location_id;
    if (!g_visits_storage->Add(id, visit, &instance)) {
      BadRequest(HTTP_POST, res);
      return;
    }
    visit->Serialize(id);

    g_db_instance->GetUsersVisitsIndex()->Add(
        user_id, instance);
    g_db_instance->GetLocationsVisitsIndex()->Add(
        location_id, instance);
  } else {
    BadRequest(HTTP_POST, res);
    return;
  }
  SendOk(res);
}

}  // namespace post_handlers