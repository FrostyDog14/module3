#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

void compute_square(int index, char *args[]) {
    double side_length = atof(args[index]);
    printf("Процесс %d: Площадь квадрата со стороной %.3lf равна %.3lf\n", getpid(), side_length, side_length * side_length);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Ошибка: Необходимо указать длины сторон квадратов.\n");
        return EXIT_FAILURE;
    }

    pid_t child_pid;

    switch (child_pid = fork()) {
        case -1: 
            perror("Ошибка при создании процесса");
            exit(EXIT_SUCCESS);

        case 0: 
            for (int i = 1; i < argc; i += 2) {
                compute_square(i, argv);
            }
            exit(EXIT_SUCCESS);

        default: 
            wait(NULL); 
            for (int i = 2; i < argc; i += 2) {
                compute_square(i, argv);
            }
            exit(EXIT_SUCCESS);
    }
}