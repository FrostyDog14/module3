#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <mqueue.h> 
#include <time.h> 

#define QUEUE_NAME "/my_queue"
#define MSG_SIZE 8
int main()
{
    struct mq_attr queue_attr;
    queue_attr.mq_maxmsg = 1000;
    queue_attr.mq_msgsize = MSG_SIZE;
    mqd_t mq_id = mq_open(QUEUE_NAME, O_CREAT | O_RDWR | O_NONBLOCK , 0600, &queue_attr);

    if (mq_id == (mqd_t)-1)
    {
        perror("Creating queue error");
        exit(EXIT_FAILURE);
    }

    int prio;
    char new_text[MSG_SIZE];
    
    for (int i = 0;  ; i++)
    {
        if (mq_receive(mq_id, new_text, MSG_SIZE, &prio) == -1){
            perror("cannot receive");
            return -1;
        }
        printf("Received :%s\n", new_text);
        if(strcmp(new_text, "255") == 0)
            break;
        sleep(1);
    }

    if (mq_close(mq_id) == -1)
        perror("Closing queue error");
        
    mq_unlink(QUEUE_NAME);
    return 0;
}