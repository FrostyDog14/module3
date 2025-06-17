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
    mq_unlink(QUEUE_NAME);
    mqd_t mq_id = mq_open(QUEUE_NAME, O_CREAT | O_RDWR | O_NONBLOCK , 0600, &queue_attr);

    if (mq_id == (mqd_t)-1)
    {
        perror("Creating queue error");
        exit(EXIT_FAILURE);
    }

    srand(time(NULL));
    unsigned int randNum;
    char strNum[MSG_SIZE];
    
    for (int i = 0; randNum != 255; i++)
    {
        randNum = (unsigned int)rand()%255 + 1;
        sprintf(strNum, "%d", randNum);
        if (mq_send(mq_id, strNum, MSG_SIZE, randNum) == -1)
        {
            printf("%d\n", errno);
            perror("Sending message error");
            mq_close(mq_id);
            exit(EXIT_FAILURE);
            
        }
        printf("Send to queue: %s\n", strNum);
        sleep(1);
    }

    if (mq_close(mq_id) == -1)
        perror("Closing queue error");
        
    mq_unlink(QUEUE_NAME);
    return 0;
}