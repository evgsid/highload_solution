/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   structures.h
 * Author: reat
 *
 * Created on August 12, 2017, 9:53 AM
 */

#ifndef STRUCTURES_H
#define STRUCTURES_H

#include <string.h>
#include <sstream>
#include <string>
#include <sys/uio.h>

#include "http_structs.h"

enum Gender {
  M,
  F,
  UNKNOWN,
};

enum UpdateFlags {
  UPDATE_FIRST_NAME = 1,
  UPDATE_LAST_NAME = 1 << 1,
  UPDATE_BIRTH_DATE = 1 << 2,
  UPDATE_EMAIL = 1 << 3,
  UPDATE_GENDER = 1 << 4,
  UPDATE_PLACE = 1 << 5,
  UPDATE_COUNTRY = 1 << 6,
  UPDATE_CITY = 1 << 7,
  UPDATE_DISTANCE = 1 << 8,
  UPDATE_LOCATION = 1 << 9,
  UPDATE_USER = 1 << 10,
  UPDATE_VISITED_AT = 1 << 11,
  UPDATE_MARK = 1 << 12,
  UPDATE_ID = 1 << 13,
};

struct User {
  std::string first_name;
  std::string last_name;
  int64_t birth_date = 0;
  std::string email;
  Gender gender = Gender::UNKNOWN;

  static const size_t iov_size = 15;

  char id_buf[8];
  size_t id_buf_len;
  char birth_date_buf[12];
  size_t birth_date_buf_len;
  char gender_buf[2];
  size_t gender_buf_len;
  char body_size_buf[5];
  size_t body_size_buf_len;

  void Serialize(uint32_t id);
  void FillIO(http_response& res);

  static Gender GetGender(const char* gender) {
    if (strlen(gender) == 1 && gender[0] == 'm')
      return Gender::M;
    if (strlen(gender) == 1 && gender[0] == 'f')
      return Gender::F;
    return Gender::UNKNOWN;
  }

  static bool Parse(const char* data, size_t length, User* user,
                    int* update_flags, uint32_t* id = nullptr);
};

struct Location {
  std::string place;
  std::string country;
  std::string city;
  uint32_t distance = 0;

  static const size_t iov_size = 13;

  char id_buf[7];
  size_t id_buf_len;
  char distance_buf[11];
  size_t distance_buf_len;
  char body_size_buf[5];
  size_t body_size_buf_len;

  void Serialize(uint32_t id);
  void FillIO(http_response& res);

  static bool Parse(const char* data, size_t length, Location* location,
                    int* update_flags, uint32_t* id = nullptr);
};

struct Visit {
  uint32_t location_id = 0;
  uint32_t user_id = 0;
  int64_t visited_at = 0;
  char mark = 0;

  static const size_t iov_size = 13;

  char id_buf[9];
  size_t id_buf_len;
  char location_id_buf[11];
  size_t location_id_buf_len;
  char user_id_buf[11];
  size_t user_id_buf_len;
  char visited_at_buf[12];
  size_t visited_at_buf_len;
  char mark_buf[2];
  size_t mark_buf_len;
  char body_size_buf[5];
  size_t body_size_buf_len;

  void Serialize(uint32_t id);
  void FillIO(http_response& res);

  static bool Parse(const char* data, size_t length, Visit* visit,
                    int* update_flags, uint32_t* id = nullptr);
};

#endif /* STRUCTURES_H */

