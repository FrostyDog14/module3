#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>

#include <ifaddrs.h>
#include <netdb.h>

void errorHandler(char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

int main()
{
    int sockfd;
    socklen_t clilen;
    ssize_t n; 
    char sendLine[1000], receivedLine[1000];
    struct sockaddr_in client1, client2;

    bzero(&client1, sizeof(client1));
    client1.sin_family = AF_INET;
    client1.sin_port = htons(2024);
    client1.sin_addr.s_addr = htonl(INADDR_ANY);

    if((sockfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
        errorHandler("socket");

    if(bind(sockfd, (struct sockaddr *) &client1, sizeof(client1)) < 0)
    {
        close(sockfd);
        errorHandler("bind");
    }

    printf("Server started\n");

    char hostbuffer[256];
	char *IPbuffer;
	struct hostent *host_entry;
	int hostname;

    hostname = gethostname(hostbuffer, sizeof(hostbuffer));
	if(hostname == -1)
        errorHandler("gethostname");

    host_entry = gethostbyname(hostbuffer);
	if( host_entry== NULL)
        errorHandler("gethostbyname");

	IPbuffer = inet_ntoa(*((struct in_addr*) host_entry->h_addr_list[0]));

	printf("Hostname: %s\n", hostbuffer);
	printf("Host IP: %s\n", IPbuffer);

    clilen = sizeof(client2);

    while(1) 
    {
        printf("> ");
        fgets(sendLine, 1000, stdin);
        
        
        if((n = recvfrom(sockfd, receivedLine, 999, 0, (struct sockaddr *) &client2, &clilen)) < 0)
        {
            close(sockfd);
            errorHandler("recvfrom");
        }

        printf("%s\n", receivedLine);

        if(sendto(sockfd, sendLine, strlen(sendLine), 0, (struct sockaddr *) &client2, clilen) < 0)
        {
            close(sockfd);
            errorHandler("sendto");
        }
    }
    return 0;
}