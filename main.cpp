
#include <cstring>
#include <folly/Conv.h>
#include <fstream>
#include <iostream>
#include <memory>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <signal.h>
#include <stdio.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>
#include <vector>

#include "db_instance.h"
#include "get_handlers.h"
#include "helpers.h"
#include "http_parser.h"
#include "http_structs.h"
#include "post_handlers.h"
#include "qs_parse.h"
#include "socket_queue.h"
#include "yuarel.h"

struct HttpData {
  http_request req;
  http_response res;
  unsigned int method;
  const char* url = nullptr;
  size_t url_length = 0;
};

int OnUrl(http_parser* parser, const char *at, size_t length) {
  HttpData* http_data = (HttpData*)(parser->data);
  http_data->url = at;
  http_data->url_length = length;
  return 0;
}

int OnBody(http_parser* parser, const char *at, size_t length) {
  HttpData* http_data = (HttpData*)(parser->data);
  http_data->req.body = at;
  http_data->req.body_length = length;
  return 0;
}

void Route(HttpData& data) {
  char* url_str = (char*)data.url;
  url_str[data.url_length] = '\0';
  yuarel yua;
  int ret = yuarel_parse(&yua, url_str);
  if (ret < 0) {
    BadRequest(data.method, data.res);
    return;
  }
  char* parts[MAX_PATH_SIZE];
  ret = yuarel_split_path(yua.path, parts, MAX_PATH_SIZE);
  if (ret < 0) {
    BadRequest(data.method, data.res);
    return;
  }
  uint32_t id = 0;
//  if (data.method == HTTP_GET && ret == 1) {
//    if (!memcmp(parts[0], "stats", 6)) {
//      std::cout << "Max users: "
//                << g_users_storage->MaxIndex()
//                << std::endl;
//      std::cout << "Max locations: "
//                << g_locations_storage->MaxIndex()
//                << std::endl;
//      std::cout << "Max visits: "
//                << g_visits_storage->MaxIndex()
//                << std::endl;
//      std::cout << "0 means limit not exceeded." << std::endl;
//
//      std::cout << "Max users-visits bucket size: "
//                << g_db_instance->GetUsersVisitsIndex()->
//                    MaxBucketSize()
//                << std::endl;
//    }
//  }
  if (data.method == HTTP_GET && ret >= 2) {
    try {
      id = folly::to<uint32_t>(parts[1]);
    } catch (...) {
      NotFound(data.method, data.res);
      return;
    }
    if (ret == 2) {
      if (!memcmp(parts[0], "users", 6)) {
        get_handlers::GetUsers(data.req, data.res, id);
        return;
      } else if (!memcmp(parts[0], "locations", 10)) {
        get_handlers::GetLocations(data.req, data.res, id);
        return;
      } else if (!memcmp(parts[0], "visits", 7)) {
        get_handlers::GetVisits(data.req, data.res, id);
        return;
      }
    } else if (ret == 3) {
      if (yua.query) {
        ret = yuarel_parse_query(yua.query, '&', data.req.url_params,
                                 MAX_QUERY_SIZE);
        if (ret < 0) {
          BadRequest(data.method, data.res);
          return;
        }
        data.req.params_size = ret;
        for (int i = 0; i < ret; ++i) {
          qs_decode(data.req.url_params[i].val);
        }
      }
      if (!memcmp(parts[0], "users", 6) && !memcmp(parts[2], "visits", 7)) {
        get_handlers::GetUsersVisits(data.req, data.res, id);
        return;
      } else if (!memcmp(parts[0], "locations", 10) &&
                 !memcmp(parts[2], "avg", 4)) {
        get_handlers::GetLocationsAvg(data.req, data.res, id);
        return;
      }
    }
  } else if (data.method == HTTP_POST && ret == 2) {
    if (!memcmp(parts[1], "new", 4)) {
      if (!memcmp(parts[0], "users", 6)) {
        post_handlers::AddUsers(data.req, data.res);
        return;
      } else if (!memcmp(parts[0], "locations", 10)) {
        post_handlers::AddLocations(data.req, data.res);
        return;
      } else if (!memcmp(parts[0], "visits", 7)) {
        post_handlers::AddVisits(data.req, data.res);
        return;
      }
    } else {
      try {
        id = folly::to<uint32_t>(parts[1]);
      } catch (...) {
        NotFound(data.method, data.res);
        return;
      }
      if (!memcmp(parts[0], "users", 6)) {
        post_handlers::UpdateUsers(data.req, data.res, id);
        return;
      } else if (!memcmp(parts[0], "locations", 10)) {
        post_handlers::UpdateLocations(data.req, data.res, id);
        return;
      } else if (!memcmp(parts[0], "visits", 7)) {
        post_handlers::UpdateVisits(data.req, data.res, id);
        return;
      }
    }
  }
  NotFound(data.method, data.res);
}

int main() {
  g_db_instance = DBInstance::GetDbInstance();
  g_users_storage = g_db_instance->GetUsers();
  g_locations_storage = g_db_instance->GetLocations();
  g_visits_storage = g_db_instance->GetVisits();

  g_db_instance->InitializeUsers();
  g_db_instance->InitializeLocations();
  g_db_instance->InitializeVisits();

  std::ifstream options("/tmp/data/options.txt", std::ifstream::in);
  options >> g_generate_time;
  options.close();

  std::cout << "Max users: "
            << DBInstance::GetDbInstance()->GetUsers()->MaxIndex()
            << std::endl;
  std::cout << "Max locations: "
            << DBInstance::GetDbInstance()->GetLocations()->MaxIndex()
            << std::endl;
  std::cout << "Max visits: "
            << DBInstance::GetDbInstance()->GetVisits()->MaxIndex()
            << std::endl;
  std::cout << "0 means limit not exceeded." << std::endl;
  std::cout << "Options time: " << g_generate_time << std::endl;

  int listener;
  struct sockaddr_in addr;
  int epollfd = 0;
  int max_events = 1000;

  signal(SIGPIPE, SIG_IGN);

  listener = socket(AF_INET, SOCK_STREAM, 0);
  if (listener < 0) {
      perror("socket");
      exit(1);
  }

  int i = 1;
  setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, (void *)&i, sizeof(i));
  setsockopt(listener, IPPROTO_TCP, TCP_QUICKACK, (void *)&i, sizeof(i));
//  setsockopt(listener, IPPROTO_TCP, TCP_DEFER_ACCEPT, (void *)&i, sizeof(i));

  addr.sin_family = AF_INET;
  addr.sin_port = htons(8080);
//    addr.sin_port = htons(80);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  if (bind(listener, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
      perror("bind");
      exit(2);
  }

  listen(listener, 100);

  epollfd = epoll_create1(0);
  if (epollfd == -1) {
      perror("epoll_create1");
      exit(EXIT_FAILURE);
  }

  std::vector<std::thread> threads;
  unsigned int thread_nums = std::thread::hardware_concurrency();
  for (unsigned int i = 0; i < thread_nums; ++i) {
    threads.push_back(std::thread([epollfd, max_events]() {
      epoll_event events[max_events];
      http_parser_settings settings;
      memset(&settings, 0, sizeof(settings));
      settings.on_url = OnUrl;
      settings.on_body = OnBody;
      std::unique_ptr<http_parser> parser(new http_parser);
      char buf[kReadBufferSize];
      int nfds = 0;
      while (true) {
        nfds = epoll_wait(epollfd, events, max_events, -1);
        if (nfds == -1) {
          if (errno == EINTR) {
            continue;
          }
          perror("epoll_wait");
          exit(EXIT_FAILURE);
        }

        for (int n = 0; n < nfds; ++n) {
          int sock = events[n].data.fd;
          int readed = 0;
          if ((readed = read(sock, buf, kReadBufferSize)) <= 0) {
            close(sock);
            continue;
          }
          HttpData http_data;
          if (buf[0] == 'G') {
            char* url_start = buf + 4;
            http_data.url = url_start;
            char* it = url_start;
            int url_len = 0;
            while (*it++ != ' ') {
              ++url_len;
            }
            http_data.url_length = url_len;
            http_data.method = HTTP_GET;
          } else {
            http_parser_init(parser.get(), HTTP_REQUEST);
            parser->data = &http_data;
            int nparsed = http_parser_execute(
                parser.get(), &settings, buf, readed);
            if (nparsed != readed) {
              close(sock);
              continue;
            }
            http_data.method = parser->method;
          }
          Route(http_data);

          if (http_data.res.iov_size) {
            size_t batches = http_data.res.iov_size / MAX_IOVEC_PART;
            bool failed = false;
            for (size_t i = 0; i < batches; ++i) {
              if (writev(sock, &http_data.res.iov[i * MAX_IOVEC_PART],
                         MAX_IOVEC_PART) <= 0) {
                close(sock);
                failed = true;
                break;
              }
            }
            if (failed) {
              continue;
            }
            if (http_data.res.iov_size != MAX_IOVEC_PART) {
              if (writev(sock, &http_data.res.iov[batches * MAX_IOVEC_PART],
                         http_data.res.iov_size - batches * MAX_IOVEC_PART) <=
                            0) {
                close(sock);
                continue;
              }
            }
          }
          if (http_data.method == HTTP_POST) {
            close(sock);
            continue;
          }
          struct epoll_event ev;
          ev.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
          ev.data.fd = sock;
          if (epoll_ctl(epollfd, EPOLL_CTL_MOD, sock, &ev) == -1) {
            perror("epoll_ctl: conn_sock");
            exit(EXIT_FAILURE);
          }
        }
      }
    }));
  }

  int new_buf_size = 1024 * 512;
  while (true) {
    int sock = accept(listener, NULL, NULL);
    if (sock == -1) {
      perror("accept");
      exit(EXIT_FAILURE);
    }

    setsockopt(sock, SOL_SOCKET, SO_SNDBUF,
               (void *)&new_buf_size, sizeof(new_buf_size));
    setsockopt(sock, SOL_SOCKET, SO_RCVBUF,
               (void *)&new_buf_size, sizeof(new_buf_size));
    setsockopt(sock, SOL_SOCKET, SO_DONTROUTE, (void *)&i, sizeof(i));
    setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (void *)&i, sizeof(i));
    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
    ev.data.fd = sock;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, sock, &ev) == -1) {
      perror("epoll_ctl: conn_sock");
      exit(EXIT_FAILURE);
    }
  }

  for (auto& t : threads) {
    t.join();
  }
  return 0;
}
