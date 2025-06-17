#include <stdlib.h>
#include <stdio.h>
#include <wait.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>

volatile int canRead = 0;

void Wait(int sig) {
    canRead = 0;
}

void Start(int sig) {
    canRead = 1;
}

void Child() {
    int fd = open("numbers.txt", O_RDONLY);
    char strNum[16];
    ssize_t bytesRead;

    while (1) {
        if (canRead == 1) {
            bytesRead = read(fd, strNum, sizeof(strNum) - 1);
            if (bytesRead > 0) {
                strNum[bytesRead] = '\0';
                printf("Child: %s", strNum);
            }
            canRead = 0;
        }
    }
}

void Parent(pid_t childPid, int n) {
    int fd = open("numbers.txt", O_WRONLY | O_APPEND | O_CREAT, 0644);
    char strNum[16];
    int randNum;

    srand(time(NULL));

    for (int i = 0; i < n; i++) {
        kill(childPid, SIGUSR1);
        randNum = rand() % 1000;
        snprintf(strNum, sizeof(strNum), "%d\n", randNum);
        write(fd, strNum, strlen(strNum));
        kill(childPid, SIGUSR2);
        usleep(100000); 
    }

    usleep(100000);
    kill(childPid, SIGKILL);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Неправильное кол во аргументов\n");
        exit(EXIT_FAILURE);
    }

    signal(SIGUSR1, Wait);
    signal(SIGUSR2, Start);

    int n = atoi(argv[1]);
    pid_t pid;

    switch (pid = fork()) {
    case -1:
        perror("Ошибка в fork");
        exit(EXIT_FAILURE);
        break;

    case 0: // Child
        Child();
        exit(EXIT_SUCCESS);

    default: // Parent
        Parent(pid, n);
        wait(NULL);
        printf("End\n");
        exit(EXIT_SUCCESS);
    }
}