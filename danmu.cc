#include "client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>

int MsgHandle(void * /*arg*/, void *msg) {
  Msg *m = (Msg*) msg;
  char *s = m->data;
  char *p, *q;
  if (strncmp(s, "type@=chatmsg", 13) == 0) {
    p = strstr(s, "nn@=");
    if (p) {
      p += 4;
      q = strchr(p, '/');
      *q++ = 0;
      printf("<%s>:", p);
      p = strstr(q, "txt@=");
      if (p) {
        p += 5;
        q = strchr(p, '/');
        *q = 0;
        printf(" %s\n", p);
      }
    }
  }
  return 0;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("%s\n", "invalid argv");
    return 0;
  }
  int rid = atoi(argv[1]);
  if (rid <= 10000) {
    printf("invalid room id: %d\n", rid);
    return -1;
  }
  Client client;
  client.Connect("openbarrage.douyutv.com", 8601);
  client.JoinRoom(rid);
  client.Watch(MsgHandle, NULL);
  return 0;
}
