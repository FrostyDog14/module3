#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1" //localhost
#define SERVER_PORT 12345
#define CLIENT_PORT 11223
#define BUF_SIZE 2048

int raw_sock;
struct sockaddr_in server_addr, client_addr;

void send_message(const char *msg, int msg_len) {
    char packet[BUF_SIZE];
    memset(packet, 0, sizeof(packet));
    struct iphdr *iph = (struct iphdr*)packet;
    struct udphdr *udph = (struct udphdr*)(packet + sizeof(struct iphdr));
    char *data = packet + sizeof(struct iphdr) + sizeof(struct udphdr);

    // IP header
    iph->ihl = 5;
    iph->version = 4;
    iph->tos = 0;
    iph->tot_len = htons(sizeof(struct iphdr) + sizeof(struct udphdr) + msg_len);
    iph->id = htons(rand() % 65535);
    iph->frag_off = 0;
    iph->ttl = 64;
    iph->protocol = IPPROTO_UDP;
    iph->saddr = client_addr.sin_addr.s_addr;
    iph->daddr = server_addr.sin_addr.s_addr;
    iph->check = 0;

    // UDP header
    udph->source = htons(CLIENT_PORT);
    udph->dest = htons(SERVER_PORT);
    udph->len = htons(sizeof(struct udphdr) + msg_len);
    udph->check = 0;

    memcpy(data, msg, msg_len);

    if (sendto(raw_sock, packet, sizeof(struct iphdr) + sizeof(struct udphdr) + msg_len, 0,
               (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("sendto");
    }
}

void handle_signal(int sig) {
    printf("Sending CLOSE to server...\n");
    send_message("CLOSE", 5);
    close(raw_sock);
    exit(0);
}

int main() {
    signal(SIGINT, handle_signal);

    raw_sock = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
    if (raw_sock < 0) {
        perror("socket");
        exit(1);
    }

    int on = 1;
    if (setsockopt(raw_sock, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on)) < 0) {
        perror("setsockopt");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    memset(&client_addr, 0, sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(CLIENT_PORT);
    client_addr.sin_addr.s_addr = inet_addr("127.0.0.3");

    // Bind to port
    if (bind(raw_sock, (struct sockaddr*)&client_addr, sizeof(client_addr)) < 0) {
        perror("bind");
        exit(1);
    }

    printf("Echo-client started. Type messages:\n");

    fd_set fds;
    char buf[BUF_SIZE];
    while (1) {
        FD_ZERO(&fds);
        FD_SET(0, &fds);
        FD_SET(raw_sock, &fds);
        int maxfd = raw_sock > 0 ? raw_sock : 0;
        select(maxfd+1, &fds, NULL, NULL, NULL);

        if (FD_ISSET(0, &fds)) {
            // stdin
            if (fgets(buf, sizeof(buf), stdin) == NULL)
                break;
            int len = strlen(buf);
            if (buf[len-1] == '\n') buf[len-1] = 0, len--;
            send_message(buf, len);
        }

        if (FD_ISSET(raw_sock, &fds)) {
            // receive
            char recvbuf[BUF_SIZE];
            struct sockaddr_in src_addr;
            socklen_t addrlen = sizeof(src_addr);
            ssize_t rlen = recvfrom(raw_sock, recvbuf, BUF_SIZE, 0, (struct sockaddr*)&src_addr, &addrlen);
            if (rlen < (ssize_t)(sizeof(struct iphdr) + sizeof(struct udphdr)))
                continue;
            struct iphdr *iph = (struct iphdr*)recvbuf;
            struct udphdr *udph = (struct udphdr*)(recvbuf + iph->ihl*4);
            char *data = recvbuf + iph->ihl*4 + sizeof(struct udphdr);
            int data_len = ntohs(udph->len) - sizeof(struct udphdr);

            // Only process packets from server
            if (iph->saddr == server_addr.sin_addr.s_addr && ntohs(udph->source) == SERVER_PORT) {
                printf("Server: %.*s\n", data_len, data);
            }
        }
    }
    handle_signal(0);
    return 0;
}