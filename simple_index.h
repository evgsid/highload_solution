/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   simple_index.h
 * Author: reat
 *
 * Created on August 12, 2017, 10:18 AM
 */

#ifndef SIMPLE_INDEX_H
#define SIMPLE_INDEX_H

#include <unordered_set>
#include <mutex>

template<class T>
class SimpleIndex {
 public:
  SimpleIndex() {
  }
  void Add(T&& val) {
    std::lock_guard<std::mutex> lock(m);
    index.insert(val);
  }

  bool Check(const T& val) {
    std::lock_guard<std::mutex> lock(m);
    return index.find(val) != index.end();
  }

  void Replace(const T& old_val, const T& new_val) {
    std::lock_guard<std::mutex> lock(m);
    index.erase(old_val);
    index.insert(new_val);
  }

 private:
  std::mutex m;
  std::unordered_set<T> index;
};

#endif /* SIMPLE_INDEX_H */

