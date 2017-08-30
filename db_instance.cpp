/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   db_instance.cpp
 * Author: reat
 * 
 * Created on August 12, 2017, 3:38 PM
 */

#include "db_instance.h"

#include <fstream>

#include "rapidjson/document.h"
#include "unzipper.h"

namespace {

const std::string data_name("/tmp/data/data.zip");

std::vector<std::string> FileNames(const std::string& pattern) {
  zipper::Unzipper unzipper(data_name);
  std::vector<zipper::ZipEntry> entries = unzipper.entries();
  std::vector<std::string> names;
  for (auto& e : entries) {
    if (e.name.find(pattern) == 0) {
      names.push_back(e.name);
    }
  }
  unzipper.close();

  return std::move(names);
}

std::vector<unsigned char> GetFileContent(const std::string& filename) {
  std::vector<unsigned char> unzipped_entry;
  zipper::Unzipper unzipper(data_name);
  unzipper.extractEntryToMemory(filename, unzipped_entry);
  unzipper.close();

  return std::move(unzipped_entry);
}

}  // namespace

std::unique_ptr<DBInstance> DBInstance::instance_ = nullptr;

// static
DBInstance* DBInstance::GetDbInstance() {
  if (!instance_) {
    instance_.reset(new DBInstance);
  }
  return instance_.get();
}

void DBInstance::InitializeUsers() {
  std::vector<std::string> file_names = FileNames("users_");
  for (const auto& name : file_names) {
    std::vector<unsigned char> buffer = GetFileContent(name);
    rapidjson::Document d;
    d.Parse((char*)buffer.data(), buffer.size());
    rapidjson::Value& users = d["users"];

    for (auto& u : users.GetArray()) {
      User* user = new User({
        u["first_name"].GetString(),
        u["last_name"].GetString(),
        u["birth_date"].GetInt64(),
        u["email"].GetString(),
        User::GetGender(u["gender"].GetString())
      });
      uint32_t id = u["id"].GetUint();
      user->Serialize(id);
      users_storage_.Add(id, user, nullptr);
      email_index_.Add(std::string(u["email"].GetString()));
    }
  }
}

void DBInstance::InitializeLocations() {
  std::vector<std::string> file_names = FileNames("locations_");
  for (const auto& name : file_names) {
    std::vector<unsigned char> buffer = GetFileContent(name);
    rapidjson::Document d;
    d.Parse((char*)buffer.data(), buffer.size());
    rapidjson::Value& locations = d["locations"];

    for (auto& l : locations.GetArray()) {
      Location* location = new Location({
        l["place"].GetString(),
        l["country"].GetString(),
        l["city"].GetString(),
        l["distance"].GetUint()
      });
      uint32_t id = l["id"].GetUint();
      location->Serialize(id);
      locations_storage_.Add(id, location, nullptr);
    }
  }
}

void DBInstance::InitializeVisits() {
  std::vector<std::string> file_names = FileNames("visits_");
  for (const auto& name : file_names) {
    std::vector<unsigned char> buffer = GetFileContent(name);
    rapidjson::Document d;
    d.Parse((char*)buffer.data(), buffer.size());
    rapidjson::Value& visits = d["visits"];

    for (auto& v : visits.GetArray()) {
      Visit* visit = new Visit({
        v["location"].GetUint(),
        v["user"].GetUint(),
        v["visited_at"].GetInt64(),
        (char)v["mark"].GetInt()
      });
      uint32_t id = v["id"].GetUint();
      visit->Serialize(id);
      Visit* instance = nullptr;
      uint32_t user_id = visit->user_id;
      uint32_t location_id = visit->location_id;
      visits_storage_.Add(id, visit, &instance);
      users_visits_index_.Add(user_id, instance);
      locations_visits_index_.Add(location_id, instance);
    }
  }
}

InMemoryStorage<User>* DBInstance::GetUsers() {
  return &users_storage_;
}

InMemoryStorage<Location>* DBInstance::GetLocations() {
  return &locations_storage_;
}

InMemoryStorage<Visit>* DBInstance::GetVisits() {
  return &visits_storage_;
}

MultiIndex<Visit>* DBInstance::GetUsersVisitsIndex() {
  return &users_visits_index_;
}

MultiIndex<Visit>* DBInstance::GetLocationsVisitsIndex() {
  return &locations_visits_index_;
}

SimpleIndex<std::string>* DBInstance::GetEmailIndex() {
  return &email_index_;
}

DBInstance::DBInstance()
    : users_storage_(USERS_SIZE),
      locations_storage_(LOCATIONS_SIZE),
      visits_storage_(VISITS_SIZE),
      users_visits_index_(USERS_SIZE),
      locations_visits_index_(LOCATIONS_SIZE) {
}

