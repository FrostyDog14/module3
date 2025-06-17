#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <unistd.h>

#define SERVER_PORT 2025

int main(int argc, char **argv) {
    struct protoent *protoent;
    char *file_path = "image.jpeg";
    char *server_hostname = "127.0.0.1";
    char buffer[BUFSIZ];
    in_addr_t in_addr;
    in_addr_t server_addr;
    int file_fd;
    int sockfd;
    ssize_t read_return;
    struct hostent *hostent;
    struct sockaddr_in sockaddr_in;

    file_fd = open(file_path, O_RDONLY);
    if (file_fd == -1) 
    {
        perror("open");
        exit(EXIT_FAILURE);
    }

    protoent = getprotobyname("tcp");
    if (protoent == NULL) 
    {
        perror("getprotobyname");
        exit(EXIT_FAILURE);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, protoent->p_proto);
    if (sockfd == -1) 
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    hostent = gethostbyname(server_hostname);
    if (hostent == NULL) 
    {
        fprintf(stderr, "gethostbyname");
        exit(EXIT_FAILURE);
    }

    in_addr = inet_addr(inet_ntoa(*(struct in_addr*)*(hostent->h_addr_list)));
    if (in_addr == -1) 
    {
        fprintf(stderr, "inet_addr");
        exit(EXIT_FAILURE);
    }
    sockaddr_in.sin_addr.s_addr = in_addr;
    sockaddr_in.sin_family = AF_INET;
    sockaddr_in.sin_port = htons(SERVER_PORT);

    if (connect(sockfd, (struct sockaddr*)&sockaddr_in, sizeof(sockaddr_in)) == -1) 
    {
        perror("connect");
        return EXIT_FAILURE;
    }

    while (1)
    {
        read_return = read(file_fd, buffer, BUFSIZ);
        if (read_return == 0)
            break;

        if (read_return == -1) {
            perror("read");
            exit(EXIT_FAILURE);
        }
        
        if (write(sockfd, buffer, read_return) == -1) 
        {
            perror("write");
            exit(EXIT_FAILURE);
        }
    }

    close(file_fd);
    printf("File sent\n");
    exit(EXIT_SUCCESS);
}