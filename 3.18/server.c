#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <errno.h>

#define SERVER_PORT 2025

#define TRUE 1
#define FALSE 0

void errorHandler(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

int sub(int a, int b)
{
    return a - b;
}

int mult(int a, int b)
{
    return a * b;
}

int division(int a, int b)
{
    return a / b;
}

void dostuff(int sock)
{
    int bytes_recv;
    int a, b;
    char buff[20 * 1024];
    int oper;
    int (*func)(int, int);

    #define str0 "\n1) -\n2) *\n3) :\r\n"
    #define str1 "Enter 1 parametr\r\n"
    #define str2 "Enter 2 parametr\r\n"

    write(sock, str0, sizeof(str0));

    bytes_recv = read(sock, &buff[0], sizeof(buff));
    if (bytes_recv < 0)
        errorHandler("ERROR reading from socket");
    oper = atoi(buff);

    switch (oper)
    {
    case 1:
        func = sub;
        break;

    case 2:
        func = mult;
        break;

    case 3:
        func = division;
        break;

    default:
        fprintf(stderr, "Wrong operation\n");
        return;
        break;
    }

    write(sock, str1, sizeof(str1));

    bytes_recv = read(sock, &buff[0], sizeof(buff));
    if (bytes_recv < 0)
        errorHandler("ERROR reading from socket");
    a = atoi(buff);

    write(sock, str2, sizeof(str2));

    bytes_recv = read(sock, &buff[0], sizeof(buff));
    if (bytes_recv < 0)
        errorHandler("ERROR reading from socket");

    b = atoi(buff);
    a = func(a, b);
    snprintf(buff, strlen(buff), "%d", a);
    buff[strlen(buff)] = '\n';

    write(sock, &buff[0], sizeof(buff));

    write(sock, NULL, 0);
    printf("-disconnect\n");

    return;
}

int main(int argc, char *argv[])
{
    int len, rc, on = 1;
    int end_server = FALSE, compress_array = FALSE;
    int close_conn;
    char buffer[80];
    int timeout;
    struct pollfd fds[200];
    int nfds = 1, current_size = 0, i, j;

    int listen_sd, new_sd;
    socklen_t clientlen;
    struct sockaddr_in serv_addr, client_addr;

    listen_sd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);

    if (listen_sd < 0) 
        errorHandler("listen");

    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(SERVER_PORT);

    if (bind(listen_sd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        errorHandler("bind");

    listen(listen_sd, 5);
    clientlen = sizeof(client_addr);

    memset(fds, 0, sizeof(fds));

    fds[0].fd = listen_sd;
    fds[0].events = POLLIN;

    timeout =  60 * 1000;

    do
    {
        printf("Waiting on poll\n");
        rc = poll(fds, nfds, timeout);

        if (rc < 0)
        {
            perror("poll failed");
            break;
        }

        if (rc == 0)
        {
            printf("poll timed out\n");
            break;
        }

        current_size = nfds;
        for (i = 0; i < current_size; i++)
        {

            if (fds[i].revents == 0)
                continue;

            if (fds[i].revents != POLLIN)
            {
                printf("  Error! revents = %d\n", fds[i].revents);
                end_server = TRUE;
                break;
            }
            if (fds[i].fd == listen_sd)
            {

                printf("Listening socket is readable\n");

                do
                {
                    new_sd = accept(listen_sd, NULL, NULL);
                    if (new_sd < 0)
                    {
                        if (errno != EWOULDBLOCK)
                        {
                            perror("  accept() failed");
                            end_server = TRUE;
                        }
                        break;
                    }

                    printf(" New incoming connection - %d\n", new_sd);
                    fds[nfds].fd = new_sd;
                    fds[nfds].events = POLLIN;
                    fds[nfds].revents = POLLIN;
                    nfds++;
                    current_size++;

                } while (new_sd != -1);
            }

            else
            {
                printf("  Descriptor %d is readable\n", fds[i].fd);

                dostuff(fds[i].fd);

                close(fds[i].fd);
                fds[i].fd = -1;
                compress_array = TRUE;

            }
        }


        if (compress_array)
        {
            compress_array = FALSE;
            for (i = 0; i < nfds; i++)
            {
                if (fds[i].fd == -1)
                {
                    for (j = i; j < nfds; j++)
                    {
                        fds[j].fd = fds[j + 1].fd;
                    }
                    i--;
                    nfds--;
                }
            }
        }

    } while (end_server == FALSE);


    for (i = 0; i < nfds; i++)
    {
        if (fds[i].fd >= 0)
            close(fds[i].fd);
    }
}
