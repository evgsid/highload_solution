/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   socket_queue.h
 * Author: reat
 *
 * Created on August 23, 2017, 7:22 PM
 */

#ifndef SOCKET_QUEUE_H
#define SOCKET_QUEUE_H

#include <condition_variable>
#include <mutex>
#include <queue>

class SocketQueue {
 public:
  SocketQueue();
  ~SocketQueue();
  void Add(int fd);
  int Get();
 private:
   std::condition_variable cv_;
   std::mutex m_;
   std::queue<int> fds_;
};

#endif /* SOCKET_QUEUE_H */

