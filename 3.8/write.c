#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

#define NAMEDPIPE_NAME "/tmp/pipe"
#define BUFFER_SIZE 5
#define PATHNAME "/tmp"

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

int main()
{
    int fd, randNum;
    char buffer[BUFFER_SIZE];
    if(mkfifo(NAMEDPIPE_NAME, 0770) == -1)
    {
        if(errno != EEXIST)
        {
            perror("mkfifo");
            exit(EXIT_FAILURE);
        }
    }
    printf("%s создан\n", NAMEDPIPE_NAME);

    fd = open(NAMEDPIPE_NAME, O_RDWR);
    if ( fd == -1 ) 
    {
        perror("open");
        exit(EXIT_FAILURE);
    }
    printf("%s готов к записи\n", NAMEDPIPE_NAME);

    key_t key1 = ftok(PATHNAME, 0);
    int semRead = semget(key1, 1, IPC_CREAT | 0660);

    if (semRead == -1) {
        perror("semget");
        exit(1);
    }

    key_t key2 = ftok(PATHNAME, 1);
    int semWrite = semget(key2, 1, IPC_CREAT | 0660);

    if (semWrite == -1) {
        perror("semget");
        exit(1);
    }

    if (semctl(semRead, 0, SETVAL, 0) == -1 || semctl(semWrite, 0, SETVAL, 0) == -1) 
    {
        perror("semctl");
        exit(1);
    }

    srand(time(NULL));
    for (int i = 0; i < 5; i++)
    {
        P(semWrite);
        randNum = rand() % 1000;
        sprintf(buffer, "%d", randNum);
        write(fd, buffer, BUFFER_SIZE);
        printf("send: %s\n", buffer);
        V(semRead);
    }
    
    V(semRead);
    close(fd);
    exit(EXIT_SUCCESS);
}