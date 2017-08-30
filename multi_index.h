/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   multi_index.h
 * Author: reat
 *
 * Created on August 13, 2017, 4:07 PM
 */

#ifndef MULTI_INDEX_H
#define MULTI_INDEX_H

#include <list>
#include <mutex>
#include <unordered_map>
#include <vector>

#include "constants.h"

template<class T>
class MultiIndex {
 public:
  MultiIndex(uint32_t storage_size) : storage_size_(storage_size),
                                      storage_(storage_size, nullptr) {
  }

  void Add(uint32_t id, T* pointer) {
    if (id >= storage_size_) {
      oversized_ = true;
      std::lock_guard<std::mutex> lock(m_);
      auto it = oversized_storage_.find(id);
      std::list<T*>* vals = nullptr;
      if (it == oversized_storage_.end()) {
        vals = new std::list<T*>();
        oversized_storage_.insert(std::make_pair(id, vals));
      } else {
        vals = it->second;
      }
      vals->push_back(pointer);
      return;
    }
    auto vals = storage_[id];
    if (!vals) {
      vals = new std::list<T*>();
      storage_[id] = vals;
    }
    vals->push_back(pointer);
  }

  void Replace(uint32_t old_id, uint32_t new_id, T* val) {
    if (old_id < storage_size_) {
      auto *old_vals = storage_[old_id];
      if (old_vals) {
        for (auto it = old_vals->begin(); it != old_vals->end(); ++it) {
          if ((*it) == val) {
            old_vals->erase(it);
            break;
          }
        }
      }
    } else if (oversized_) {
      std::lock_guard<std::mutex> lock(m_);
      auto it = oversized_storage_.find(old_id);
      if (it != oversized_storage_.end()) {
        auto *old_vals = it->second;
        for (auto it = old_vals->begin(); it != old_vals->end(); ++it) {
          if ((*it) == val) {
            old_vals->erase(it);
            break;
          }
        }
      }
    }
    Add(new_id, val);
  }

  const std::list<T*>* GetValues(uint32_t id) {
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

  uint32_t MaxBucketSize() {
    uint32_t max_bucket_size = 0;
    for (auto* b : storage_) {
      if (b && b->size() > max_bucket_size) {
        max_bucket_size = b->size();
      }
    }
    return max_bucket_size;
  }

 private:
   uint32_t storage_size_;
   std::vector<std::list<T*>*> storage_;

   bool oversized_ = false;
   std::mutex m_;
   std::unordered_map<uint32_t, std::list<T*>*> oversized_storage_;
};


#endif /* MULTI_INDEX_H */

