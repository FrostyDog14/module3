#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>

#define MSG_SIZE 100
#define MSG_TYPE 1
#define END_MSG_TYPE 2

struct msgbuf {
    long mtype;
    char mtext[MSG_SIZE];
};

int main() {
    key_t key = ftok("progfile", 65);
    int msqid = msgget(key, 0666 | IPC_CREAT);
    struct msgbuf message;
    strcpy(message.mtext, "Привет от Receiver!");
    msgsnd(msqid, &message, sizeof(message), 0);

    while (1) {
        message.mtype = MSG_TYPE;
        msgsnd(msqid, &message, sizeof(message), 0);
        msgrcv(msqid, &message, sizeof(message), MSG_TYPE, 0);
        printf("Получено от Sender: %s\n", message.mtext);
        if (strcmp(message.mtext, "exit\n") == 0) {
            break;
        }
    }

    msgctl(msqid, IPC_RMID, NULL);
    return 0;
}
