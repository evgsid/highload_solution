/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   constants.h
 * Author: reat
 *
 * Created on August 13, 2017, 4:27 PM
 */

#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <cstdint>
#include <unistd.h>


class DBInstance;
template<class T>
class InMemoryStorage;
class User;
class Visit;
class Location;

extern const uint32_t USERS_SIZE;
extern const uint32_t LOCATIONS_SIZE;
extern const uint32_t VISITS_SIZE;

const int MAX_QUERY_SIZE = 10;
const int MAX_PATH_SIZE = 10;

const int MAX_IOVEC_SIZE = 2000;
const int MAX_IOVEC_PART = 1000;

const size_t kReadBufferSize = 2048;

extern const char* header;
extern const size_t header_len;

extern const char* id_str;
extern const size_t id_str_len;

extern const char* place_str;
extern const size_t place_str_len;

extern const char* country_str;
extern const size_t country_str_len;

extern const char* city_str;
extern const size_t city_str_len;

extern const char* distance_str;
extern const size_t distance_str_len;

extern const char* close_str;
extern const size_t close_str_len;

extern const char* close_with_comma_str;
extern const size_t close_with_comma_str_len;

extern const char* close_close_close_str;
extern const size_t close_close_close_str_len;

extern const char* email_str;
extern const size_t email_str_len;

extern const char* first_name_str;
extern const size_t first_name_str_len;

extern const char* last_name_str;
extern const size_t last_name_str_len;

extern const char* gender_str;
extern const size_t gender_str_len;

extern const char* birth_date_str;
extern const size_t birth_date_str_len;

extern const char* location_str;
extern const size_t location_str_len;

extern const char* user_str;
extern const size_t user_str_len;

extern const char* visited_str;
extern const size_t visited_str_len;

extern const char* mark_str;
extern const size_t mark_str_len;

extern const char* visits_mark_str;
extern const size_t visits_mark_str_len;

extern const char* visits_str;
extern const size_t visits_str_len;

extern const char* avg_str;
extern const size_t avg_str_len;

extern int64_t g_generate_time;

extern DBInstance* g_db_instance;

extern InMemoryStorage<User>* g_users_storage;
extern InMemoryStorage<Location>* g_locations_storage;
extern InMemoryStorage<Visit>* g_visits_storage;

enum QueryFlags {
  FROM_DATE = 1,
  TO_DATE = 1 << 1,
  COUNTRY = 1 << 2,
  TO_DISTANCE = 1 << 3,
  FROM_AGE = 1 << 4,
  TO_AGE = 1 << 5,
  GENDER = 1 << 6,
};

#endif /* CONSTANTS_H */

