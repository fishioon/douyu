#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024
#define DANMU_PORT  8601
#define DANMU_IP    "125.88.176.8"
#define USERNAME    "username"
#define PASSWORD    "passwd"

struct MsgInfo {
    int len;
    int code;
    int magic;
    char content[BUFFER_SIZE];
};

int sock_conn(int port, const char* addr) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = inet_addr(addr);

    if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        fprintf(stderr, "connect socket failed, port:%d, addr:%s\n", port, addr);
        return -1;
    }
    return sockfd;
}

int sock_close(int sockfd) {
    close(sockfd);
    return 0;
}

int process_danmu(int msg_len, MsgInfo* msg) {
    if (msg_len <= 12 || msg->len <= 8) {
        printf("msg_len:%d\n", msg_len);
        return -1;
    }
    char* p = strstr(msg->content, "content@=");
    if (!p) return -1;
    char* content = p + sizeof("content@=")-1;
    p = strchr(content, '/');
    if (!p) return -1;
    *p++ = 0;
    char* snick = p + sizeof("snick@=")-1;
    p = strchr(snick, '/');
    if (!p) return -1;
    *p++ = 0;
    printf("%s:\t%s\n", snick, content);
    return 0;
}

int douyu_danmu(int sockfd, const char* username,
        const char* passwd, int room_id, int group_id) {
    MsgInfo msg;
    int ct_len = snprintf(msg.content, sizeof(msg.content),
            "type@=loginreq/username@%s=/password@=%s/roomid@=%d/ct@=2/",
            username, passwd, room_id);
    msg.len = ct_len + 1 + sizeof(msg.code) + sizeof(msg.magic);
    msg.code = msg.len;
    msg.magic = 0x2b1;
    send(sockfd, &msg, msg.len+sizeof(msg.len), 0);
    recv(sockfd, &msg, sizeof(msg), 0);
    printf("recv:%s\n", msg.content);

    ct_len = snprintf(msg.content, sizeof(msg.content),
            "type@=joingroup/rid@=%d/gid@=%d/", room_id, group_id);
    msg.len = ct_len + 1 + sizeof(msg.code) + sizeof(msg.magic);
    msg.code = msg.len;
    msg.magic = 0x2b1;
    send(sockfd, &msg, msg.len+sizeof(msg.len), 0);
    int ret = 0;
    while (true) {
        ret = recv(sockfd, &msg, sizeof(msg), 0);
        if (process_danmu(ret, &msg) == -1) {
            send(sockfd, &msg, 0, 0);
        }
    }
    return 0;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("usage: danmu room_id group_id\n");
        return 0;
    }
    const char* username = argc == 4 ? argv[3] : USERNAME;
    const char* passwd = argc == 4 ? argv[4] : PASSWORD;
    int room_id = atoi(argv[1]);
    int group_id = atoi(argv[2]);
    int sockfd = sock_conn(DANMU_PORT, DANMU_IP);
    douyu_danmu(sockfd, username, passwd, room_id, group_id);
    return 0;
}
