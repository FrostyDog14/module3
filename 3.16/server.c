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

int main(int argc, char **argv) 
{
    char buffer[BUFSIZ];
    char *file_path = "image.jpeg";
    int client_sockfd;
    int file_fd;
    int server_sockfd;
    socklen_t client_len;
    ssize_t read_stop;
    struct protoent *protoent;
    struct sockaddr_in client_address, server_address;

    protoent = getprotobyname("tcp");
    if (protoent == NULL) 
    {
        perror("getprotobyname");
        exit(EXIT_FAILURE);
    }

    server_sockfd = socket(AF_INET, SOCK_STREAM, protoent->p_proto);
    if (server_sockfd == -1) 
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(SERVER_PORT);

    if (bind(server_sockfd, (struct sockaddr*)&server_address, sizeof(server_address)) == -1) 
    {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    if (listen(server_sockfd, 5) == -1)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Server started\n");

    client_len = sizeof(client_address);

    client_sockfd = accept( server_sockfd, (struct sockaddr*)&client_address, &client_len);
    if(client_sockfd == -1)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    file_fd = open(file_path, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);

    if (file_fd == -1) 
    {
        perror("open");
        exit(EXIT_FAILURE);
    }
    
    do 
    {
        read_stop = read(client_sockfd, buffer, BUFSIZ);

        if (read_stop == -1) 
        {
            perror("read");
            exit(EXIT_FAILURE);
        }
        
        if (write(file_fd, buffer, read_stop) == -1) 
        {
            perror("write");
            exit(EXIT_FAILURE);
        }
    } while (read_stop > 0);

    close(file_fd);
    close(client_sockfd);

    return EXIT_SUCCESS;
}
