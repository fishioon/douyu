
#include "client.h"

#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <thread>
#include <time.h>
#include <unistd.h>

const int max_send_size = 256;
const int max_recv_size = 40960;

struct Msg {
  uint32_t length;
  uint32_t length2;
  uint16_t msg_type;
  uint8_t encrypt;
  uint8_t reserve;
  char data[0];
};

Msg *NewSendMsg() {
  Msg *msg = (Msg *)malloc(sizeof(Msg) + 256);
  msg->msg_type = 689;
  msg->reserve = 0;
  msg->encrypt = 0;
  return msg;
}

Client::Client() {}

Client::~Client() {}

int Client::Connect(const char *host_addr, int port) {
  struct hostent *host = gethostbyname(host_addr);
  if (!host) {
    fprintf(stderr, "gethostbyname failed, host:%s\n", host_addr);
    return -1;
  }

  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  struct sockaddr_in servaddr;
  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(port);
  servaddr.sin_addr = *((struct in_addr *)host->h_addr);

  if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
    fprintf(stderr, "connect socket failed, port:%d addr:%s\n", port,
            host_addr);
    return -1;
  }
  sockfd_ = sockfd;

  // std::thread(Heartbeat);
  return sockfd;
}

int Client::JoinRoom(int rid) {
  // login
  Msg *msg = (Msg *)malloc(sizeof(Msg) + max_send_size);
  Msg *recv_msg = (Msg *)malloc(sizeof(Msg) + max_recv_size);
  msg->msg_type = 689;
  msg->reserve = 0;
  msg->encrypt = 0;

  int n = snprintf(msg->data, max_send_size, "type@=loginreq/");
  msg->length = n + 1 + 8;
  msg->length2 = msg->length;
  send(sockfd_, msg, msg->length + 4, 0);
  recv(sockfd_, recv_msg, sizeof(Msg) + max_recv_size, 0);
  printf("login res: %s\n", recv_msg->data);

  // join room
  n = snprintf(msg->data, max_send_size, "type@=joingroup/rid@=%d/gid@=-9999/",
               rid);
  msg->length = n + 1 + 8;
  msg->length2 = msg->length;
  send(sockfd_, msg, msg->length + 4, 0);

  free(msg);
  free(recv_msg);
  return 0;
}

void *KeepLive(void *arg) {
  Client *client = (Client *)arg;
  client->Heartbeat();
  return NULL;
}

int Client::Watch(HandleFunc handle, void *arg) {
  pthread_t tid;
  pthread_create(&tid, NULL, KeepLive, this);
  Msg *recv_msg = (Msg *)malloc(sizeof(Msg) + max_recv_size);
  int ret = 1;
  while (ret > 0) {
    ret = recv(sockfd_, recv_msg, sizeof(Msg) + max_recv_size, 0);
    handle(arg, recv_msg->data);
  }
  free(recv_msg);
  return ret;
}

void Client::Heartbeat() {
  Msg *msg = (Msg *)malloc(sizeof(Msg) + max_send_size);
  msg->msg_type = 689;
  msg->reserve = 0;
  msg->encrypt = 0;
  while (true) {
    printf("heartbeat\n");
    int n = snprintf(msg->data, max_send_size, "type@=keeplive/tick@=%d/",
                     (int)time(NULL));
    msg->length = n + 1 + 8;
    msg->length2 = msg->length;
    send(sockfd_, msg, msg->length + 4, 0);
    sleep(45);
  }
  free(msg);
}
