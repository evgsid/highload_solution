/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   db_instance.h
 * Author: reat
 *
 * Created on August 12, 2017, 3:38 PM
 */

#ifndef DB_INSTANCE_H
#define DB_INSTANCE_H

#include <memory>

#include "in_memory_storage.h"
#include "multi_index.h"
#include "simple_index.h"
#include "structures.h"

class DBInstance {
 public:
  static DBInstance* GetDbInstance();
  void InitializeUsers();
  void InitializeLocations();
  void InitializeVisits();

  InMemoryStorage<User>* GetUsers();
  InMemoryStorage<Location>* GetLocations();
  InMemoryStorage<Visit>* GetVisits();

  MultiIndex<Visit>* GetUsersVisitsIndex();
  MultiIndex<Visit>* GetLocationsVisitsIndex();

  SimpleIndex<std::string>* GetEmailIndex();

 private:
  static std::unique_ptr<DBInstance> instance_;

  InMemoryStorage<User> users_storage_;
  InMemoryStorage<Location> locations_storage_;
  InMemoryStorage<Visit> visits_storage_;

  MultiIndex<Visit> users_visits_index_;
  MultiIndex<Visit> locations_visits_index_;

  SimpleIndex<std::string> email_index_;

  DBInstance();
  DBInstance(const DBInstance& orig) = delete;
  DBInstance& operator=(const DBInstance) = delete;
};

#endif /* DB_INSTANCE_H */

