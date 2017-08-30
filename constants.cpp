/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "constants.h"

#include <cstring>

#include "db_instance.h"

const uint32_t USERS_SIZE = 1005000;
const uint32_t LOCATIONS_SIZE = 771000;
const uint32_t VISITS_SIZE = 10008000;

const char* header = "HTTP/1.1 200 OK\r\n"
                     "S: b\r\n"
                     "C: k\r\n"
                     "B: a\r\n"
                     "Content-Length: ";
const size_t header_len = strlen(header);

const char* id_str = "\r\n\r\n{\"id\":";
const size_t id_str_len = strlen(id_str);

const char* place_str = ",\"place\":\"";
const size_t place_str_len = strlen(place_str);

const char* country_str = "\",\"country\":\"";
const size_t country_str_len = strlen(country_str);

const char* city_str = "\",\"city\":\"";
const size_t city_str_len = strlen(city_str);

const char* distance_str = "\",\"distance\":";
const size_t distance_str_len = strlen(distance_str);

const char* close_str = "}";
const size_t close_str_len = 1;

const char* close_with_comma_str = "\"},";
const size_t close_with_comma_str_len = 3;

const char* close_close_close_str = "\"}]}";
const size_t close_close_close_str_len = 4;

const char* email_str = ",\"email\":\"";
const size_t email_str_len = strlen(email_str);

const char* first_name_str = "\",\"first_name\":\"";
const size_t first_name_str_len = strlen(first_name_str);

const char* last_name_str = "\",\"last_name\":\"";
const size_t last_name_str_len = strlen(last_name_str);

const char* gender_str = "\",\"gender\":\"";
const size_t gender_str_len = strlen(gender_str);

const char* birth_date_str = "\",\"birth_date\":";
const size_t birth_date_str_len = strlen(birth_date_str);

const char* location_str = ",\"location\":";
const size_t location_str_len = strlen(location_str);

const char* user_str = ",\"user\":";
const size_t user_str_len = strlen(user_str);

const char* visited_str = ",\"visited_at\":";
const size_t visited_str_len = strlen(visited_str);

const char* mark_str = ",\"mark\":";
const size_t mark_str_len = strlen(mark_str);

const char* visits_mark_str = "{\"mark\":";
const size_t visits_mark_str_len = strlen(visits_mark_str);

const char* visits_str = "\r\n\r\n{\"visits\":[";
const size_t visits_str_len = strlen(visits_str);

const char* avg_str = "\r\n\r\n{\"avg\": ";
const size_t avg_str_len = strlen(avg_str);

int64_t g_generate_time = -1;

DBInstance* g_db_instance = nullptr;

InMemoryStorage<User>* g_users_storage = nullptr;
InMemoryStorage<Location>* g_locations_storage = nullptr;
InMemoryStorage<Visit>* g_visits_storage = nullptr;
