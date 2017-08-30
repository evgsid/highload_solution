/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "structures.h"

#include <ctime>
#include <iostream>
#include <limits>
#include <string>

#include "db_instance.h"
#include "rapidjson/document.h"
#include "http_structs.h"

namespace {

const std::string kId("id");

const std::string kEmail("email");
const std::string kFirstName("first_name");
const std::string kLastName("last_name");
const std::string kGender("gender");
const std::string kBirthDate("birth_date");

const std::string kPlace("place");
const std::string kCountry("country");
const std::string kCity("city");
const std::string kDistance("distance");

const std::string kLocation("location");
const std::string kUser("user");
const std::string kVisitedAt("visited_at");
const std::string kMark("mark");

size_t GetUtf8StringLength(const char* s) {
  size_t len = 0;
  while (*s) len += (*s++ & 0xc0) != 0x80;
  return len;
}

const size_t MAX_EMAIL = 100;
const size_t MAX_FIRST_NAME = 50;
const size_t MAX_LAST_NAME = 50;

tm MIN_AGE {0, 0, 0, 1, 0, 30, -1, -1, -1};
tm MAX_AGE {0, 0, 0, 1, 0, 99, -1, -1, -1};

tm MIN_VISITED {0, 0, 0, 1, 0, 100, -1, -1, -1};
tm MAX_VISITED {0, 0, 0, 1, 0, 115, -1, -1, -1};

const size_t MAX_PLACE = std::numeric_limits<size_t>::max();;
const size_t MAX_COUNTRY = 50;
const size_t MAX_CITY = 50;
const uint32_t MIN_UINT32 = 0;
const uint32_t MAX_UINT32 = std::numeric_limits<uint32_t>::max();
const uint32_t MIN_MARK = 0;
const uint32_t MAX_MARK = 5;



bool CheckString(const rapidjson::Document::ValueType& value, size_t max_length,
                 int* update_flags, UpdateFlags flag) {
  if (*update_flags & flag) {
    return false;
  }
  if (!value.IsString()) {
    return false;
  }
  if (GetUtf8StringLength(value.GetString()) > max_length) {
    return false;
  }
  *update_flags |= flag;

  return true;
}

bool CheckDate(const rapidjson::Document::ValueType& value,
               time_t min_age, time_t max_age,
               int* update_flags, UpdateFlags flag) {
  if (*update_flags & flag) {
    return false;
  }
  if (!value.IsInt64()) {
    return false;
  }
  int64_t age = value.GetInt64();
  if (age < min_age || age > max_age) {
    return false;
  }
  *update_flags |= flag;

  return true;
}

bool CheckInt(const rapidjson::Document::ValueType& value,
              uint32_t min, uint32_t max,
              int* update_flags, UpdateFlags flag) {
  if (*update_flags & flag) {
    return false;
  }
  if (!value.IsUint()) {
    return false;
  }
  uint32_t val = value.GetUint();
  if (val < min || val > max) {
    return false;
  }
  *update_flags |= flag;

  return true;
}

}  // namespace

void User::Serialize(uint32_t id) {
  id_buf_len = sprintf(id_buf, "%u", id);
  gender_buf_len = sprintf(gender_buf, "%s", (gender == Gender::M ? "m" : "f"));
  birth_date_buf_len = sprintf(birth_date_buf, "%ld", birth_date);
  unsigned int body_size = id_str_len + id_buf_len +
          email_str_len + email.length() +
          first_name_str_len + first_name.length() +
          last_name_str_len + last_name.length() +
          gender_str_len + gender_buf_len +
          birth_date_str_len + birth_date_buf_len +
          close_str_len;
  body_size -= 4;  // \r\n\r\n between header and body

  body_size_buf_len = sprintf(body_size_buf, "%u", body_size);
}

void User::FillIO(http_response& res) {
  iovec* iov = res.iov;
  res.iov_size = iov_size;

  iov[0].iov_base = (void*)header;
  iov[0].iov_len = header_len;

  iov[1].iov_base = body_size_buf;
  iov[1].iov_len = body_size_buf_len;

  iov[2].iov_base = (void*)id_str;
  iov[2].iov_len = id_str_len;

  iov[3].iov_base = id_buf;
  iov[3].iov_len = id_buf_len;

  iov[4].iov_base = (void*)email_str;
  iov[4].iov_len = email_str_len;

  iov[5].iov_base = (void*)email.c_str();
  iov[5].iov_len = email.length();

  iov[6].iov_base = (void*)first_name_str;
  iov[6].iov_len = first_name_str_len;

  iov[7].iov_base = (void*)first_name.c_str();
  iov[7].iov_len = first_name.length();

  iov[8].iov_base = (void*)last_name_str;
  iov[8].iov_len = last_name_str_len;

  iov[9].iov_base = (void*)last_name.c_str();
  iov[9].iov_len = last_name.length();

  iov[10].iov_base = (void*)gender_str;
  iov[10].iov_len = gender_str_len;

  iov[11].iov_base = gender_buf;
  iov[11].iov_len = gender_buf_len;

  iov[12].iov_base = (void*)birth_date_str;
  iov[12].iov_len = birth_date_str_len;

  iov[13].iov_base = birth_date_buf;
  iov[13].iov_len = birth_date_buf_len;

  iov[14].iov_base = (void*)close_str;
  iov[14].iov_len = close_str_len;
}


// static
bool User::Parse(const char* data, size_t length, User* user,
                 int* update_flags, uint32_t* id) {
  rapidjson::Document d;
  d.Parse(data, length);
  if (!d.IsObject()) {
    return false;
  }

  // email - max 100, unique
  // first_name - max 50
  // last_name - max 50
  // gender - f/m
  // birth_date - 01.01.1930 - 01.01.1999
  for (auto& v : d.GetObject()) {
    if (v.value.IsNull()) {
      return false;
    }
    if (!memcmp(kId.c_str(), v.name.GetString(), kId.length() + 1)) {
      if (!id) {
        return false;
      }
      if (!CheckInt(v.value, MIN_UINT32, MAX_UINT32,
                    update_flags, UpdateFlags::UPDATE_ID)) {
        return false;
      }
      *id = v.value.GetUint();
    } else if (!memcmp(kEmail.c_str(), v.name.GetString(),
                       kEmail.length() + 1)) {
      if (!CheckString(v.value, MAX_EMAIL, update_flags,
                       UpdateFlags::UPDATE_EMAIL)) {
        return false;
      }
      std::string email = v.value.GetString();
      if (g_db_instance->GetEmailIndex()->Check(email)) {
        return false;
      }
      std::swap(user->email, email);
    } else if (!memcmp(kFirstName.c_str(), v.name.GetString(),
                     kFirstName.length() + 1)) {
      if (!CheckString(v.value, MAX_FIRST_NAME, update_flags,
                       UpdateFlags::UPDATE_FIRST_NAME)) {
        return false;
      }
      user->first_name = v.value.GetString();
    } else if (!memcmp(kLastName.c_str(), v.name.GetString(),
                     kLastName.length() + 1)) {
      if (!CheckString(v.value, MAX_LAST_NAME, update_flags,
                       UpdateFlags::UPDATE_LAST_NAME)) {
        return false;
      }
      user->last_name = v.value.GetString();
    } else if (!memcmp(kGender.c_str(), v.name.GetString(),
                     kGender.length() + 1)) {
      if (!CheckString(v.value, 1, update_flags, UpdateFlags::UPDATE_GENDER)) {
        return false;
      }
      if (v.value.GetString()[0] == 'f') {
        user->gender = Gender::F;
      } else if (v.value.GetString()[0] == 'm') {
        user->gender = Gender::M;
      } else {
        return false;
      }
    } else if (!memcmp(kBirthDate.c_str(), v.name.GetString(),
              kBirthDate.length() + 1)) {
      static const time_t min_age = mktime(&MIN_AGE);
      static const time_t max_age = mktime(&MAX_AGE);
      if (!CheckDate(v.value, min_age, max_age, update_flags,
                     UpdateFlags::UPDATE_BIRTH_DATE)) {
        return false;
      }
      user->birth_date = v.value.GetInt64();
    }
  }

  return true;
}

void Location::Serialize(uint32_t id) {
  id_buf_len = sprintf(id_buf, "%u", id);
  distance_buf_len = sprintf(distance_buf, "%u", distance);

  unsigned int body_size = id_str_len + id_buf_len +
      place_str_len + place.length() +
      country_str_len + country.length() +
      city_str_len + city.length() +
      distance_str_len + distance_buf_len +
      close_str_len;

  body_size -= 4;  // \r\n\r\n between header and body
  body_size_buf_len = sprintf(body_size_buf, "%u", body_size);
}

void Location::FillIO(http_response& res) {
  iovec* iov = res.iov;
  res.iov_size = iov_size;

  iov[0].iov_base = (void*)header;
  iov[0].iov_len = header_len;

  iov[1].iov_base = body_size_buf;
  iov[1].iov_len = body_size_buf_len;

  iov[2].iov_base = (void*)id_str;
  iov[2].iov_len = id_str_len;

  iov[3].iov_base = id_buf;
  iov[3].iov_len = id_buf_len;

  iov[4].iov_base = (void*)place_str;
  iov[4].iov_len = place_str_len;

  iov[5].iov_base = (void*)place.c_str();
  iov[5].iov_len = place.length();

  iov[6].iov_base = (void*)country_str;
  iov[6].iov_len = country_str_len;

  iov[7].iov_base = (void*)country.c_str();
  iov[7].iov_len = country.size();

  iov[8].iov_base = (void*)city_str;
  iov[8].iov_len = city_str_len;

  iov[9].iov_base = (void*)city.c_str();
  iov[9].iov_len = city.length();

  iov[10].iov_base = (void*)distance_str;
  iov[10].iov_len = distance_str_len;

  iov[11].iov_base = distance_buf;
  iov[11].iov_len = distance_buf_len;

  iov[12].iov_base = (void*)close_str;
  iov[12].iov_len = close_str_len;
}

// static
bool Location::Parse(const char* data, size_t length, Location* location,
                     int* update_flags, uint32_t* id) {
  rapidjson::Document d;
  d.Parse(data, length);
  if (!d.IsObject()) {
    return false;
  }

  // place - unlimited
  // country - max 50
  // city - max 50
  // distance - positive
  for (auto& v : d.GetObject()) {
    if (v.value.IsNull()) {
      return false;
    }
    if (!memcmp(kId.c_str(), v.name.GetString(), kId.length() + 1)) {
      if (!id) {
        return false;
      }
      if (!CheckInt(v.value, MIN_UINT32, MAX_UINT32, update_flags,
                    UpdateFlags::UPDATE_ID)) {
        return false;
      }
      *id = v.value.GetUint();
    } else if (!memcmp(kPlace.c_str(), v.name.GetString(),
                       kPlace.length() + 1)) {
      if (!CheckString(v.value, MAX_PLACE, update_flags,
                       UpdateFlags::UPDATE_PLACE)) {
        return false;
      }
      location->place = v.value.GetString();
    } else if (!memcmp(kCountry.c_str(), v.name.GetString(),
                       kCountry.length() + 1)) {
      if (!CheckString(v.value, MAX_COUNTRY, update_flags,
                       UpdateFlags::UPDATE_COUNTRY)) {
        return false;
      }
      location->country = v.value.GetString();
    } else if (!memcmp(kCity.c_str(), v.name.GetString(), kCity.length() + 1)) {
      if (!CheckString(v.value, MAX_CITY, update_flags,
                       UpdateFlags::UPDATE_CITY)) {
        return false;
      }
      location->city = v.value.GetString();
    } else if (!memcmp(kDistance.c_str(), v.name.GetString(),
                       kDistance.length() + 1)) {
      if (!CheckInt(v.value, MIN_UINT32, MAX_UINT32, update_flags,
                    UpdateFlags::UPDATE_DISTANCE)) {
        return false;
      }
      location->distance = v.value.GetUint();
    }
  }

  return true;
}

void Visit::Serialize(uint32_t id) {
  id_buf_len = sprintf(id_buf, "%u", id);
  location_id_buf_len = sprintf(location_id_buf, "%u", location_id);
  user_id_buf_len = sprintf(user_id_buf, "%u", user_id);
  visited_at_buf_len = sprintf(visited_at_buf, "%ld", visited_at);
  mark_buf_len = sprintf(mark_buf, "%u", (uint32_t)mark);
  unsigned int body_size = id_str_len + id_buf_len +
      location_str_len + location_id_buf_len +
      user_str_len + user_id_buf_len +
      visited_str_len + visited_at_buf_len +
      mark_str_len + mark_buf_len +
      close_str_len;
  body_size -= 4;  // \r\n\r\n between header and body
  body_size_buf_len = sprintf(body_size_buf, "%u", body_size);
}

void Visit::FillIO(http_response& res) {
  iovec* iov = res.iov;
  res.iov_size = iov_size;

  iov[0].iov_base = (void*)header;
  iov[0].iov_len = header_len;

  iov[1].iov_base = body_size_buf;
  iov[1].iov_len = body_size_buf_len;

  iov[2].iov_base = (void*)id_str;
  iov[2].iov_len = id_str_len;

  iov[3].iov_base = id_buf;
  iov[3].iov_len = id_buf_len;

  iov[4].iov_base = (void*)location_str;
  iov[4].iov_len = location_str_len;

  iov[5].iov_base = location_id_buf;
  iov[5].iov_len = location_id_buf_len;

  iov[6].iov_base = (void*)user_str;
  iov[6].iov_len = user_str_len;

  iov[7].iov_base = user_id_buf;
  iov[7].iov_len = user_id_buf_len;

  iov[8].iov_base = (void*)visited_str;
  iov[8].iov_len = visited_str_len;

  iov[9].iov_base = visited_at_buf;
  iov[9].iov_len = visited_at_buf_len;

  iov[10].iov_base = (void*)mark_str;
  iov[10].iov_len = mark_str_len;

  iov[11].iov_base = mark_buf;
  iov[11].iov_len = mark_buf_len;

  iov[12].iov_base = (void*)close_str;
  iov[12].iov_len = close_str_len;
}

// static
bool Visit::Parse(const char* data, size_t length, Visit* visit,
                  int* update_flags, uint32_t* id) {
  rapidjson::Document d;
  d.Parse(data, length);
  if (!d.IsObject()) {
    return false;
  }

  // location - id
  // user - id
  // visited_at - 01.01.2000 - 01.01.2015
  // mark - 0..5
  for (auto& v : d.GetObject()) {
    if (v.value.IsNull()) {
      return false;
    }
    if (!memcmp(kId.c_str(), v.name.GetString(), kId.length() + 1)) {
      if (!id) {
        return false;
      }
      if (!CheckInt(v.value, MIN_UINT32, MAX_UINT32, update_flags,
                    UpdateFlags::UPDATE_ID)) {
        return false;
      }
      *id = v.value.GetUint();
    } else if (!memcmp(kLocation.c_str(), v.name.GetString(),
                       kLocation.length() + 1))  {
      if (!CheckInt(v.value, MIN_UINT32, MAX_UINT32, update_flags,
                    UpdateFlags::UPDATE_LOCATION)) {
        return false;
      }
      visit->location_id = v.value.GetUint();
      if (!g_locations_storage->Get(
              visit->location_id)) {
        return false;
      }
    } else if (!memcmp(kUser.c_str(), v.name.GetString(), kUser.length() + 1)) {
      if (!CheckInt(v.value, MIN_UINT32, MAX_UINT32, update_flags,
                    UpdateFlags::UPDATE_USER)) {
        return false;
      }
      visit->user_id = v.value.GetUint();
      if (!g_users_storage->Get(visit->user_id)) {
        return false;
      }
    } else if (!memcmp(kVisitedAt.c_str(), v.name.GetString(),
               kVisitedAt.length() + 1)) {
      static const time_t min_visited = mktime(&MIN_VISITED);
      static const time_t max_visited = mktime(&MAX_VISITED);
      if (!CheckDate(v.value, min_visited, max_visited, update_flags,
                     UpdateFlags::UPDATE_VISITED_AT)) {
        return false;
      }
      visit->visited_at = v.value.GetInt64();
    } else if (!memcmp(kMark.c_str(), v.name.GetString(), kMark.length() + 1)) {
      if (!CheckInt(v.value, MIN_MARK, MAX_MARK, update_flags,
                    UpdateFlags::UPDATE_MARK)) {
        return false;
      }
      visit->mark = v.value.GetUint();
    }
  }

  return true;
}

