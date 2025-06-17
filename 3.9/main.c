#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <fcntl.h>
#include <time.h>

#define SEM_KEY_1 1
#define SEM_KEY_2 2

void V(int semid)
{
    struct sembuf buf;
    buf.sem_num = 0;
    buf.sem_op = 1;
    buf.sem_flg = SEM_UNDO;
    semop(semid, &buf, 1);
}   

void P(int semid)
{
    struct sembuf buf;
    buf.sem_num = 0;
    buf.sem_op = -1;
    buf.sem_flg = SEM_UNDO;
    semop(semid, &buf, 1);
}

void Child(int n, int semRead, int semWrite)
{
    usleep(10);
    char strNum[4];
    int num;
    int fd = open("file.txt",  O_RDONLY , 0660);
    V(semWrite);
    for (int i = 0; i < n ; i++)
    {
        P(semRead);
        read(fd, strNum, sizeof(strNum));
        printf("Child get: %s\n", strNum);
        V(semWrite);
    }
    close(fd);
}

void Parent(int n, int semRead, int semWrite)
{
    int fd = open("file.txt",  O_WRONLY | O_APPEND | O_CREAT, 0660);
    char strNum[4];
    int randNum;
    
    srand(time(NULL));

    for (int i = 0; i < n; i++)
    {
        P(semWrite);
        randNum = rand() % 1000;
        snprintf(strNum, sizeof(strNum), "%d", randNum);
        write(fd, strNum, sizeof(strNum));    
        V(semRead);
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

    int semRead = semget(SEM_KEY_1, 1, IPC_CREAT | 0666);
    if (semRead == -1) {
        perror("semget");
        exit(1);
    }

    int semWrite = semget(SEM_KEY_2, 1, IPC_CREAT | 0666);
    if (semWrite == -1) {
        perror("semget");
        exit(1);
    }

    if (semctl(semRead, 0, SETVAL, 0) == -1 || semctl(semWrite, 0, SETVAL, 0) == -1) 
    {
        perror("semctl");
        exit(1);
    }

    int n = atoi(argv[1]);

    pid_t pid;
    switch (pid = fork())
    {
    case -1:
        perror("fork");
        exit(EXIT_FAILURE);
        break;

    case 0: // Child proccess
        Child(n, semRead, semWrite);
        exit(EXIT_SUCCESS);

    default: // Parent proccess
        Parent(n, semRead, semWrite);
        wait(NULL);
        remove("file.txt");
        exit(EXIT_SUCCESS);
    }
}
