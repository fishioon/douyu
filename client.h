#ifndef DOUYU_CLIENT_H_
#define DOUYU_CLIENT_H_

#include <stdint.h>

typedef int (*HandleFunc)(void *arg, void* msg);

class Client {
public:
  Client();
  int Connect(const char *host, int port);
  int JoinRoom(int rid);
  int Watch(HandleFunc handle, void* arg);
  void Heartbeat();
  virtual ~Client();

private:
  int rid_;
  int sockfd_;
};

#endif // DOUYU_CLIENT_H_
