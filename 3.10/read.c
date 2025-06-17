#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/ipc.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>

#define NAMEDPIPE_NAME "/tmp/pipe"
#define BUFFER_SIZE 5
#define SEM_READ "sem_read"
#define SEM_WRITE "sem_write"

int main() {
    if (mkfifo(NAMEDPIPE_NAME, 0660) == -1 && errno != EEXIST) {
        perror("mkfifo");
        exit(EXIT_FAILURE);
    }
    errno = 0;

    printf("%s has been created\n", NAMEDPIPE_NAME);

    int pipe_fd = open(NAMEDPIPE_NAME, O_RDONLY);
    if (pipe_fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
        
    }

    sem_t *semRead = sem_open(SEM_READ, O_CREAT, 0600, 0);
    if (errno == EEXIST) {
        semRead = sem_open(SEM_READ, 0600);
    }
    if (semRead == SEM_FAILED) {
        perror("sem_open read");
        exit(EXIT_FAILURE);
        
    }

    sem_t *semWrite = sem_open(SEM_WRITE, O_CREAT, 0600, 0);
    if (errno == EEXIST) {
        semWrite = sem_open(SEM_WRITE, 0600);
    }
    if (semWrite == SEM_FAILED) {
        perror("sem_open write");
        exit(EXIT_FAILURE);
    }

    char buffer[BUFFER_SIZE];
    ssize_t bytesRead;
    sem_post(semWrite);
    while (1) {
        sem_wait(semRead);
        bytesRead = read(pipe_fd, buffer, BUFFER_SIZE);
        if (bytesRead == 0) {
            break;
        }
        printf("read: %s\n", buffer);
        sem_post(semWrite);
    }

    sem_unlink(SEM_READ);
    sem_unlink(SEM_WRITE);
    close(pipe_fd);
    printf("Reading finished\n");
}
