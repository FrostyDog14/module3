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
    int fd;
    char buffer[BUFFER_SIZE];

    fd = open(NAMEDPIPE_NAME, O_RDONLY);
    if ( fd == -1 ) {
        perror("open");
        exit(EXIT_FAILURE);
    }
    printf("%s открыт для чтения\n", NAMEDPIPE_NAME);

    int key1 = ftok(PATHNAME, 0);
    int semRead = semget(key1, 1, IPC_CREAT | 0660);
    
    if (semRead == -1) {
        perror("semget");
        exit(1);
    }

    int key2 = ftok(PATHNAME, 1);
    int semWrite = semget(key2 , 1, IPC_CREAT | 0660);

    if (semWrite == -1) {
        perror("semget");
        exit(1);
    }

    V(semWrite);
    ssize_t endofile = 1;

    for (int i = 0; ; i++)
    {
        P(semRead);
        endofile = read(fd, buffer, BUFFER_SIZE);
        if(endofile == 0)
            break;
        printf("read: %s\n", buffer);
        V(semWrite);
    }

    close(fd);
    exit(EXIT_SUCCESS);
}