#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <fcntl.h>
#include <time.h>
#include <semaphore.h>

#define SEMNAME1 "sem1"
#define SEMNAME2 "sem2"


void errorHandler(char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

void Child(int n, sem_t *semRead, sem_t *semWrite)
{
    usleep(10);
    char strNum[4];
    int num;
    int fd = open("file.txt",  O_RDONLY , 0660);
    sem_post(semWrite);
    for (int i = 0; i < n ; i++)
    {
        sem_wait(semRead);
        read(fd, strNum, sizeof(strNum));
        printf("Child get: %s\n", strNum);
        sem_post(semWrite);
    }
    close(fd);
}

void Parent(int n, sem_t *semRead, sem_t *semWrite)
{
    int fd = open("file.txt",  O_WRONLY | O_APPEND | O_CREAT, 0660);
    char strNum[4];
    int randNum;
    
    srand(time(NULL));

    for (int i = 0; i < n; i++)
    {
        sem_wait(semWrite);
        randNum = rand() % 1000;
        snprintf(strNum, sizeof(strNum), "%d", randNum);
        write(fd, strNum, sizeof(strNum));    
        sem_post(semRead);
    }
    close(fd);
}

int main(int argc, char *argv[])
{
    if(argc != 2)
    {
        fprintf(stderr, "Wrong number of arg!\n");
        exit(EXIT_FAILURE);
    }

    // семафор для считывания
    sem_t *semToRead = sem_open(SEMNAME1, O_CREAT, 0600, 0);
    if(errno == EEXIST)
        semToRead = sem_open(SEMNAME1, 0600);
    if(semToRead == SEM_FAILED)
        errorHandler("sem_open");

    // семафор для записи
    sem_t *semToWrite = sem_open(SEMNAME2, O_CREAT, 0600, 0);
    if(errno == EEXIST)
        semToWrite = sem_open(SEMNAME2, 0600);
    if(semToWrite == SEM_FAILED)
        errorHandler("sem_open");


    int n = atoi(argv[1]);

    pid_t pid;
    switch (pid = fork())
    {
    case -1:
        perror("fork");
        exit(EXIT_FAILURE);
        break;

    case 0: // Child proccess
        Child(n, semToRead, semToWrite);
        exit(EXIT_SUCCESS);

    default: // Parent proccess
        Parent(n, semToRead, semToWrite);
        wait(NULL);
        remove("file.txt");
        printf("Program finished!\n");
        exit(EXIT_SUCCESS);
    }
}
