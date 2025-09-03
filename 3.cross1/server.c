#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>

#define SERVER_PORT 12345
#define BUF_SIZE 2048
#define MAX_CLIENTS 100

typedef struct {
    struct in_addr ip;
    uint16_t port;
    int counter;
} client_t;

client_t clients[MAX_CLIENTS];
int client_count = 0;
int raw_sock;

int find_client(struct in_addr ip, uint16_t port) {
    for (int i = 0; i < client_count; ++i) {
        if (clients[i].ip.s_addr == ip.s_addr && clients[i].port == port)
            return i;
    }
    return -1;
}

void reset_client(struct in_addr ip, uint16_t port) {
    int idx = find_client(ip, port);
    if (idx != -1) {
        // Remove client by shifting
        for (int i = idx; i < client_count - 1; ++i)
            clients[i] = clients[i+1];
        client_count--;
    }
}

void handle_signal(int sig) {
    printf("Shutting down server...\n");
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

    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(raw_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        exit(1);
    }

    printf("Echo-server started on port %d\n", SERVER_PORT);

    char buf[BUF_SIZE];
    while (1) {
        struct sockaddr_in src_addr;
        socklen_t addrlen = sizeof(src_addr);
        ssize_t len = recvfrom(raw_sock, buf, BUF_SIZE, 0, (struct sockaddr*)&src_addr, &addrlen);
        if (len < (ssize_t)(sizeof(struct iphdr) + sizeof(struct udphdr)))
            continue;

        struct iphdr *iph = (struct iphdr*)buf;
        struct udphdr *udph = (struct udphdr*)(buf + iph->ihl*4);
        char *data = buf + iph->ihl*4 + sizeof(struct udphdr);
        int data_len = ntohs(udph->len) - sizeof(struct udphdr);

        if (ntohs(udph->dest) != SERVER_PORT)
            continue;

        // Prepare client info
        struct in_addr client_ip;
        client_ip.s_addr = iph->saddr;
        uint16_t client_port = ntohs(udph->source);

        // Check for CLOSE message
        if (data_len >= 5 && strncmp(data, "CLOSE", 5) == 0) {
            printf("Client %s:%d disconnected\n", inet_ntoa(client_ip), client_port);
            reset_client(client_ip, client_port);
            continue;
        }

        // Find or add client
        int idx = find_client(client_ip, client_port);
        if (idx == -1) {
            if (client_count < MAX_CLIENTS) {
                clients[client_count].ip = client_ip;
                clients[client_count].port = client_port;
                clients[client_count].counter = 1;
                idx = client_count++;
            } else {
                fprintf(stderr, "Too many clients!\n");
                continue;
            }
        } else {
            clients[idx].counter++;
        }

        // Prepare reply: original message + " N"
        char reply[BUF_SIZE];
        int reply_len = snprintf(reply, sizeof(reply), "%.*s %d", data_len, data, clients[idx].counter);

        // Build IP/UDP headers
        char packet[BUF_SIZE];
        memset(packet, 0, sizeof(packet));
        struct iphdr *r_iph = (struct iphdr*)packet;
        struct udphdr *r_udph = (struct udphdr*)(packet + sizeof(struct iphdr));
        char *r_data = packet + sizeof(struct iphdr) + sizeof(struct udphdr);

        // IP header
        r_iph->ihl = 5;
        r_iph->version = 4;
        r_iph->tos = 0;
        r_iph->tot_len = htons(sizeof(struct iphdr) + sizeof(struct udphdr) + reply_len);
        r_iph->id = htons(rand() % 65535);
        r_iph->frag_off = 0;
        r_iph->ttl = 64;
        r_iph->protocol = IPPROTO_UDP;
        r_iph->saddr = iph->daddr;
        r_iph->daddr = iph->saddr;
        r_iph->check = 0;

        // UDP header
        r_udph->source = htons(SERVER_PORT);
        r_udph->dest = htons(client_port);
        r_udph->len = htons(sizeof(struct udphdr) + reply_len);
        r_udph->check = 0;

        // Copy data
        memcpy(r_data, reply, reply_len);

        // Send packet
        struct sockaddr_in dst_addr = {0};
        dst_addr.sin_family = AF_INET;
        dst_addr.sin_port = htons(client_port);
        dst_addr.sin_addr = client_ip;

        if (sendto(raw_sock, packet, sizeof(struct iphdr) + sizeof(struct udphdr) + reply_len, 0,
                   (struct sockaddr*)&dst_addr, sizeof(dst_addr)) < 0) {
            perror("sendto");
        }
    }
    return 0;
}
