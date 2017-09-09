/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "get_handlers.h"

#include <algorithm>
#include <string>

#include "db_instance.h"
#include "helpers.h"
#include "http_parser.h"

namespace get_handlers {

namespace {

const std::string kFromData("fromDate");
const std::string kToDate("toDate");
const std::string kCountry("country");
const std::string kToDistance("toDistance");
const std::string kFromAge("fromAge");
const std::string kToAge("toAge");
const std::string kGender("gender");

const std::string kEmptyVisits("HTTP/1.1 200 OK\r\n"
                               "S: b\r\n"
                               "C: k\r\n"
                               "B: a\r\n"
                               "Content-Length: 13\r\n"
                               "\r\n"
                               "{\"visits\":[]}");
const std::string kAvgEmpty("HTTP/1.1 200 OK\r\n"
                            "S: b\r\n"
                            "C: k\r\n"
                            "B: a\r\n"
                            "Content-Length: 11\r\n"
                            "\r\n"
                            "{\"avg\":0.0}");

}  // namespace

void GetUsers(const http_request& req, http_response& res, uint32_t id) {
  auto* users = g_users_storage;
  User* user = users->Get(id);
  if (!user) {
    NotFound(HTTP_GET, res);
    return;
  }
  user->FillIO(res);
}

void GetLocations(const http_request& req, http_response& res, uint32_t id) {
  auto* locations = g_locations_storage;
  Location* location = locations->Get(id);
  if (!location) {
    NotFound(HTTP_GET, res);
    return;
  }
  location->FillIO(res);
}

void GetVisits(const http_request& req, http_response& res, uint32_t id) {
  auto* visits = g_visits_storage;
  Visit* visit = visits->Get(id);
  if (!visit) {
    NotFound(HTTP_GET, res);
    return;
  }
  visit->FillIO(res);
}

void GetUsersVisits(const http_request& req, http_response& res, uint32_t id) {
  // Parse query params
  int search_flags = 0;
  int from_date = 0;
  int to_date = 0;
  std::string country;
  uint32_t to_distance = 0;
  
  for (int i = 0; i < req.params_size; ++i) {
    auto& param = req.url_params[i];
    if (!memcmp(kFromData.c_str(), param.key, kFromData.length() + 1)) {
      if (!GetInt(param.val, &from_date)) {
        BadRequest(HTTP_GET, res);
        return;
      }
      search_flags |= QueryFlags::FROM_DATE;
    } else if (!memcmp(kToDate.c_str(), param.key, kToDate.length() + 1)) {
      if (!GetInt(param.val, &to_date)) {
        BadRequest(HTTP_GET, res);
        return;
      }
      search_flags |= QueryFlags::TO_DATE;
    } else if (!memcmp(kCountry.c_str(), param.key, kCountry.length() + 1)) {
      country = param.val;
      search_flags |= QueryFlags::COUNTRY;
    } else if (!memcmp(kToDistance.c_str(), param.key,
                       kToDistance.length() + 1)) {
      if (!GetUint32(param.val, &to_distance)) {
        BadRequest(HTTP_GET, res);
        return;
      }
      search_flags |= QueryFlags::TO_DISTANCE;
    }
  }

  auto* index = g_db_instance->GetUsersVisitsIndex();
  auto* visits = index->GetValues(id);
  if (!visits || visits->empty()) {
    if (g_users_storage->Get(id)) {
      SendResponse(res, kEmptyVisits);
      return;
    } else {
      NotFound(HTTP_GET, res);
      return;
    }
  } else {
    size_t filtered_visits_index = 0;
    Visit* filtered_visits[1000];
    for (auto it = visits->begin(); it != visits->end(); ++it) {
      Visit* visit = *it;
      if (search_flags & QueryFlags::FROM_DATE) {
        if (visit->visited_at <= from_date) {
          continue;
        }
      }
      if (search_flags & QueryFlags::TO_DATE) {
        if (visit->visited_at >= to_date) {
          continue;
        }
      }
      if (search_flags & QueryFlags::COUNTRY ||
          search_flags & QueryFlags::TO_DISTANCE) {
        const Location* location =
            g_locations_storage->Get(
                visit->location_id);
        if (search_flags & QueryFlags::COUNTRY) {
          if (location->country != country) {
            continue;
          }
        }
        if (search_flags & QueryFlags::TO_DISTANCE) {
          if (location->distance >= to_distance) {
            continue;
          }
        }
      }
      filtered_visits[filtered_visits_index++] = visit;
    }

    if (!filtered_visits_index) {
      SendResponse(res, kEmptyVisits);
      return;
    }
    std::sort(&filtered_visits[0], &filtered_visits[filtered_visits_index],
        [](const Visit* l, const Visit* r) {
      return l->visited_at < r->visited_at;
    });
    if (filtered_visits_index * 7 + 3 >= MAX_IOVEC_SIZE) {
      std::cout << "GetUsersVisits: filtered size = "
                << filtered_visits_index << std::endl;
      BadRequest(HTTP_GET, res);
      return;
    }
    iovec* iov = res.iov;

    iov[0].iov_base = (void*)header;
    iov[0].iov_len = header_len;

    // iov[1] - body size

    iov[2].iov_base = (void*)visits_str;
    iov[2].iov_len = visits_str_len;

    size_t record_size = 7;
    for (size_t i = 0; i < filtered_visits_index - 1; ++i) {
      iov[3 + i * record_size].iov_base = (void*)visits_mark_str;
      iov[3 + i * record_size].iov_len = visits_mark_str_len;

      iov[3 + i * record_size + 1].iov_base = filtered_visits[i]->mark_buf;
      iov[3 + i * record_size + 1].iov_len = filtered_visits[i]->mark_buf_len;

      iov[3 + i * record_size + 2].iov_base = (void*)visited_str;
      iov[3 + i * record_size + 2].iov_len = visited_str_len;

      iov[3 + i * record_size + 3].iov_base =
          filtered_visits[i]->visited_at_buf;
      iov[3 + i * record_size + 3].iov_len =
          filtered_visits[i]->visited_at_buf_len;

      iov[3 + i * record_size + 4].iov_base = (void*)place_str;
      iov[3 + i * record_size + 4].iov_len = place_str_len;

      iov[3 + i * record_size + 5].iov_base =
          (void*)g_locations_storage->Get(
              filtered_visits[i]->location_id)->place.c_str();
      iov[3 + i * record_size + 5].iov_len =
          g_locations_storage->Get(
              filtered_visits[i]->location_id)->place.length();

      iov[3 + i * record_size + 6].iov_base = (void*)close_with_comma_str;
      iov[3 + i * record_size + 6].iov_len = close_with_comma_str_len;
    }
    int i = filtered_visits_index - 1;

    iov[3 + i * record_size].iov_base = (void*)visits_mark_str;
    iov[3 + i * record_size].iov_len = visits_mark_str_len;

    iov[3 + i * record_size + 1].iov_base = filtered_visits[i]->mark_buf;
    iov[3 + i * record_size + 1].iov_len = filtered_visits[i]->mark_buf_len;

    iov[3 + i * record_size + 2].iov_base = (void*)visited_str;
    iov[3 + i * record_size + 2].iov_len = visited_str_len;

    iov[3 + i * record_size + 3].iov_base = filtered_visits[i]->visited_at_buf;
    iov[3 + i * record_size + 3].iov_len =
        filtered_visits[i]->visited_at_buf_len;

    iov[3 + i * record_size + 4].iov_base = (void*)place_str;
    iov[3 + i * record_size + 4].iov_len = place_str_len;

    iov[3 + i * record_size + 5].iov_base =
        (void*)g_locations_storage->Get(
            filtered_visits[i]->location_id)->place.c_str();
    iov[3 + i * record_size + 5].iov_len =
        g_locations_storage->Get(
            filtered_visits[i]->location_id)->place.length();

    iov[3 + i * record_size + 6].iov_base = (void*)close_close_close_str;
    iov[3 + i * record_size + 6].iov_len = close_close_close_str_len;

    size_t body_size = 0;
    for (size_t i = 2; i <= filtered_visits_index * record_size + 2; ++i) {
      body_size += iov[i].iov_len;
    }
    body_size -= 4;  // \r\n\r\n in the visits_str.

    size_t body_size_len = sprintf(res.body_size_buf, "%ld", body_size);
    iov[1].iov_base = res.body_size_buf;
    iov[1].iov_len = body_size_len;

    res.iov_size = 3 + filtered_visits_index * record_size;
  }
}

void GetLocationsAvg(const http_request& req, http_response& res, uint32_t id) {
  // Parse query params
  int search_flags = 0;
  int from_date = 0;
  int to_date = 0;
  int from_age = 0;
  int to_age = 0;
  Gender gender = Gender::UNKNOWN;
  for (int i = 0; i < req.params_size; ++i) {
    auto& param = req.url_params[i];
    if (!memcmp(kFromData.c_str(), param.key, kFromData.length() + 1)) {
      if (!GetInt(param.val, &from_date)) {
        BadRequest(HTTP_GET, res);
        return;
      }
      search_flags |= QueryFlags::FROM_DATE;
    } else if (!memcmp(kToDate.c_str(), param.key, kToDate.length() + 1)) {
      if (!GetInt(param.val, &to_date)) {
        BadRequest(HTTP_GET, res);
        return;
      }
      search_flags |= QueryFlags::TO_DATE;
    } else if (!memcmp(kFromAge.c_str(), param.key, kFromAge.length() + 1)) {
      if (!GetInt(param.val, &from_age)) {
        BadRequest(HTTP_GET, res);
        return;
      }
      search_flags |= QueryFlags::FROM_AGE;
    } else if (!memcmp(kToAge.c_str(), param.key, kToAge.length() + 1)) {
      if (!GetInt(param.val, &to_age)) {
        BadRequest(HTTP_GET, res);
        return;
      }
      search_flags |= QueryFlags::TO_AGE;
    } else if (!memcmp(kGender.c_str(), param.key, kGender.length() + 1)) {
      size_t gender_len = strlen(param.val);
      if (gender_len == 1 && param.val[0] == 'm') {
        gender = Gender::M;
      } else if (gender_len == 1 && param.val[0] == 'f') {
        gender = Gender::F;
      } else {
        BadRequest(HTTP_GET, res);
        return;
      }
      search_flags |= QueryFlags::GENDER;
    }
  }
  auto* index = g_db_instance->GetLocationsVisitsIndex();
  auto* visits = index->GetValues(id);
  if (!visits || visits->empty()) {
    if (g_locations_storage->Get(id)) {
      SendResponse(res, kAvgEmpty);
      return;
    } else {
      NotFound(HTTP_GET, res);
      return;
    }
  } else {
    size_t filtered_visits_index = 0;
    Visit* filtered_visits[1000];
    for (auto it = visits->begin(); it != visits->end(); ++it) {
      Visit* visit = *it;
      if (search_flags & QueryFlags::FROM_DATE) {
        if (visit->visited_at <= from_date) {
          continue;
        }
      }
      if (search_flags & QueryFlags::TO_DATE) {
        if (visit->visited_at >= to_date) {
          continue;
        }
      }
      if (search_flags & QueryFlags::FROM_AGE ||
          search_flags & QueryFlags::TO_AGE) {
        const User* user =
            g_users_storage->Get(visit->user_id);
        static time_t t = g_generate_time;   // get time now
        static struct tm now = (*localtime(&t));
        if (search_flags & QueryFlags::FROM_AGE) {
          tm from_age_tm = now;
          from_age_tm.tm_year -= from_age;
          time_t from_age_t = mktime(&from_age_tm);
          if (user->birth_date > from_age_t) {
            continue;
          }
        }
        if (search_flags & QueryFlags::TO_AGE) {
          tm to_age_tm = now;
          to_age_tm.tm_year -= to_age;
          time_t to_age_t = mktime(&to_age_tm);
          if (user->birth_date < to_age_t) {
            continue;
          }
        }
      }
      if (search_flags & QueryFlags::GENDER) {
        const User* user =
            g_users_storage->Get(visit->user_id);
        if (user->gender != gender) {
          continue;
        }
      }
      filtered_visits[filtered_visits_index++] = visit;
    }
    double avg = 0.0;
    if (filtered_visits_index) {
      for (size_t i = 0; i < filtered_visits_index; ++i) {
        avg += filtered_visits[i]->mark;
      }
      avg /= filtered_visits_index;
    }
    iovec* iov = res.iov;

    iov[0].iov_base = (void*)header;
    iov[0].iov_len = header_len;

    // iov[1] - body size

    iov[2].iov_base = (void*)avg_str;
    iov[2].iov_len = avg_str_len;

    avg *= 100000;
    avg += 0.5;
    avg = (int)avg;
    avg /= 100000.0;
    size_t avg_buf_len = sprintf(res.avg_buf, "%f", avg);
    iov[3].iov_base = res.avg_buf;
    iov[3].iov_len = avg_buf_len;

    iov[4].iov_base = (void*)close_str;
    iov[4].iov_len = close_str_len;

    size_t body_size_len = sprintf(res.body_size_buf, "%lu",
            iov[2].iov_len + iov[3].iov_len + iov[4].iov_len - 4);  // \r\n\r\n
    iov[1].iov_base = res.body_size_buf;
    iov[1].iov_len = body_size_len;

    res.iov_size = 5;
  }
}

}  // namespace get_handlers