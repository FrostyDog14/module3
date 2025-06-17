#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <wait.h>
#include <limits.h>
#include <signal.h>

#define PATHNAME "/tmp"
#define SHMEM_MAXMSG 128
#define MSG_SIZE 5

volatile pid_t pid;
volatile int data_sets_processed = 0; 

typedef struct shared_memory {
    int buf [SHMEM_MAXMSG];
    int n_msg;
    int pos_minmax;
} shared_memory;

void errorHandler(char* msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

void handler_SIGINT(int sig) {
    printf("Parent process received SIGINT\n");
    printf("Total data sets processed: %d\n", data_sets_processed);
    exit(EXIT_SUCCESS);
}

void Parent(shared_memory *p_shmem)
{
    signal(SIGINT, handler_SIGINT);
    srand((unsigned int)time(NULL));
    int randNum;

    while(1)
    {
        for (int i = 0; i < 10; i++)
        {
            randNum = rand()%1000;
            p_shmem->buf[p_shmem->n_msg] = randNum;
            printf("Parent get: %d\n", p_shmem->buf[p_shmem->n_msg]);
            p_shmem->n_msg++;
        }

        sleep(2);

        printf("MIN = %d\n", p_shmem->buf[p_shmem->n_msg - 2]);
        printf("MAX = %d\n\n", p_shmem->buf[p_shmem->n_msg - 1]);

        data_sets_processed++;
    }
}

void Child(shared_memory *p_shmem)
{   
    int min = INT_MAX, max = INT_MIN;

    while(1)
    {
        sleep(2);

        for (int i = p_shmem->pos_minmax; i < p_shmem->n_msg; i++)
        {
            int curValue = p_shmem->buf[i];

            if(curValue > max)
                max = curValue;
            if(curValue < min)
                min = curValue;
        }

        p_shmem->pos_minmax = p_shmem->n_msg;

        p_shmem->buf[p_shmem->n_msg] = min;
        p_shmem->n_msg++;

        p_shmem->buf[p_shmem->n_msg] = max;
        p_shmem->n_msg++;

        data_sets_processed++;
    }
}

int main()
{
    
    int shm_id = shm_open(PATHNAME, O_CREAT | O_RDWR, 0666);
    if (shm_id == -1)
        errorHandler("shm_open");

    if(ftruncate (shm_id, sizeof(shared_memory)) == -1)
        errorHandler("ftruncate");

    shared_memory *p_shmem = mmap(NULL, sizeof(shared_memory), PROT_READ | PROT_WRITE, MAP_SHARED, shm_id, 0);
    if(p_shmem == NULL)
    {
        shm_unlink(PATHNAME);
        errorHandler("mmap");
    }

    p_shmem->n_msg = 0;
    p_shmem->pos_minmax = 0;
    

    switch (pid = fork())
    {
    case  -1:
        errorHandler("fork");
        break;
    
    case 0:
        Child(p_shmem);
        break;

    default:
        Parent(p_shmem);
        wait(NULL);
        break;
    }
    shm_unlink(PATHNAME);
}
