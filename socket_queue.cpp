/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   socket_queue.cpp
 * Author: reat
 * 
 * Created on August 23, 2017, 7:22 PM
 */

#include "socket_queue.h"

SocketQueue::SocketQueue() {
}

SocketQueue::~SocketQueue() {
}

void SocketQueue::Add(int fd) {
  std::lock_guard<std::mutex> lock(m_);
  if (fds_.empty()) {
    cv_.notify_one();
  }
  fds_.push(fd);
}

int SocketQueue::Get() {
  std::unique_lock<std::mutex> lock(m_);
  while (fds_.empty()) {
    cv_.wait(lock);
  }
  int fd = fds_.front();
  fds_.pop();
  return fd;
}

