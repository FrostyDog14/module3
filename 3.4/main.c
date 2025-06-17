#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#define BUFFER_SIZE 256

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Использование: %s <количество случайных чисел>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int count = atoi(argv[1]);
    if (count <= 0) {
        fprintf(stderr, "Количество должно быть положительным числом.\n");
        exit(EXIT_FAILURE);
    }

    int pipefd[2];
    if(pipe(pipefd) == -1)
    {
        perror("Ошибка при создании pipe");
        exit(EXIT_FAILURE);
    }

    pid_t pid;
    switch (pid = fork()) {
        case -1:
            perror("Ошибка при создании дочернего процесса");
            exit(EXIT_FAILURE);
        case 0: 
            close(pipefd[0]); 

            srand(time(NULL)); 
            for (int i = 0; i < count; i++) {
                int random_number = rand() % 100; 
                write(pipefd[1], &random_number, sizeof(random_number)); 
            }

            close(pipefd[1]); 
            exit(EXIT_SUCCESS);
        default: 
            close(pipefd[1]); 

            FILE *file = fopen("random_numbers.txt", "w");
            if (file == NULL) {
                perror("Ошибка при открытии файла");
                return EXIT_FAILURE;
            }

            int received_number;
            for (int i = 0; i < count; i++) {
                switch (read(pipefd[0], &received_number, sizeof(received_number))) {
                    case -1:
                        perror("Ошибка при чтении из канала");
                        fclose(file);
                        exit(EXIT_FAILURE);
                    case 0:
                        fprintf(stderr, "Канал закрыт, нет больше данных для чтения.\n");
                        fclose(file);
                        exit(EXIT_FAILURE);
                    default:
                        printf("Получено число: %d\n", received_number);
                        fprintf(file, "%d\n", received_number);
                        break;
                }
            }

            close(pipefd[0]); 
            fclose(file); 
            wait(NULL); 
    }

    return EXIT_SUCCESS;
}
