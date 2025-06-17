#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

volatile int nclients = 0;

void dostuff(int);

void errorHandler(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

void printusers()
{ 
	if(nclients)
	    printf("%d user on-line\n",nclients);
	else 
        printf("No User on line\n");
}

int sub(int a, int b)
{
	return a-b;
}

int mult(int a, int b)
{
	return a*b;
}

int division(int a, int b)
{
	return a/b;
}

int main(int argc, char *argv[])
{
    char buff[1024];
	int sockfd, newsockfd;
	int portno; 
	int pid;
    socklen_t clilen; 
    struct sockaddr_in serv_addr, cli_addr;
	
    if (argc < 2) 
	{
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }

	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (sockfd < 0) 
       errorHandler("ERROR opening socket");
	 
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
              errorHandler("ERROR on binding");

	listen(sockfd,5);
    clilen = sizeof(cli_addr);
	
    printf("Server started!\n");
	while (1) 
	{
         newsockfd = accept(sockfd,(struct sockaddr *) &cli_addr, &clilen);
         if (newsockfd < 0) 
             errorHandler("ERROR on accept");
	    nclients++; 

	    printusers();
		
         pid = fork();
         if (pid < 0)
             errorHandler("ERROR on fork");
         if (pid == 0)  
		 {
             close(sockfd);
             dostuff(newsockfd);
             exit(0);
         }
         else close(newsockfd);
     }

     close(sockfd);
     return 0;
}

void dostuff (int sock)
{
	int bytes_recv;
	int a,b;
	char buff[20 * 1024];
    int oper;
    int (*func) (int, int);

    #define str0 "\n1) -\n2) *\n3) :\r\n"
	#define str1 "Enter 1 parametr\r\n"
	#define str2 "Enter 2 parametr\r\n" 
    
   	write(sock, str0, sizeof(str0));

	bytes_recv = read(sock,&buff[0],sizeof(buff));
	if (bytes_recv < 0) errorHandler("ERROR reading from socket");
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

	bytes_recv = read(sock,&buff[0],sizeof(buff));
	if (bytes_recv < 0) 
        errorHandler("ERROR reading from socket");
	a = atoi(buff);
	
	write(sock,str2,sizeof(str2));
	
	bytes_recv = read(sock,&buff[0],sizeof(buff));
	if (bytes_recv < 0) 
        errorHandler("ERROR reading from socket");
	
	b = atoi(buff);
	a = func(a,b);
	snprintf(buff, strlen(buff), "%d", a);
	buff[strlen(buff)] = '\n';
	
	write(sock,&buff[0],sizeof(buff));
    
    write(sock, NULL ,0);
	nclients--; 
    	printf("-disconnect\n"); 
	printusers();
    
	return;
}