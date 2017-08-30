/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   in_memory_storage.h
 * Author: reat
 *
 * Created on August 12, 2017, 9:35 AM
 */

#ifndef INMEMORYSTORAGE_H
#define INMEMORYSTORAGE_H

#include <forward_list>
#include <iostream>
#include <mutex>
#include <unordered_map>
#include <vector>

#include "constants.h"

template <class T>
class InMemoryStorage {
public:
  typedef std::unordered_map<uint32_t, T*> Map;
  InMemoryStorage(int storage_size);

  bool Add(uint32_t id, T* data, T** pointer);
  T* Get(uint32_t id);
  uint32_t MaxIndex() { return max_index_; }
private:
  uint32_t storage_size_;
  std::vector<T*> storage_;
  uint32_t max_index_ = 0;

  bool oversized_ = false;
  std::mutex m_;
  std::unordered_map<uint32_t, T*> oversized_storage_;
};

template <class T>
InMemoryStorage<T>::InMemoryStorage(int storage_size)
    : storage_size_(storage_size), storage_(storage_size, nullptr) {
}

template <class T>
bool InMemoryStorage<T>::Add(uint32_t id, T* data, T** pointer) {
  if (id >= storage_size_) {
    oversized_ = true;
    if (id > max_index_) {
      max_index_ = id;
    }
    std::lock_guard<std::mutex> lock(m_);
    if (oversized_storage_.find(id) != oversized_storage_.end()) {
      return false;
    }
    oversized_storage_.insert(std::make_pair(id, data));
    if (pointer)
      *pointer = data;
    return true;
  }
  if (storage_[id]) {
    return false;
  }

  storage_[id] = data;
  if (pointer)
    *pointer = data;
  return true;
}

template <class T>
T* InMemoryStorage<T>::Get(uint32_t id) {
  if (id >= storage_size_) {
    if (!oversized_) {
      return nullptr;
    }
    std::lock_guard<std::mutex> lock(m_);
    auto it = oversized_storage_.find(id);
    if (it == oversized_storage_.end()) {
      return nullptr;
    }
    return it->second;
  }
  return storage_[id];
}

#endif /* INMEMORYSTORAGE_H */

