#include <stdlib.h>
#include <stdio.h>
#include <wait.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>


int LaunchProgram()
{
    char str[1024];
    printf(">> ");
    fscanf(stdin, "%1024[^\n]", str);

    char *args[16];
    args[0] = strtok(str, " ");

    int i;
    for(i=1; ; i++)
    {
        args[i] = strtok(NULL, " ");
        if(args[i] == NULL)
            break;
    }
    args[i] = NULL;

    return execvp(args[0], args);
}

int main()
{
    pid_t pid;
    for (int i = 0; ; i++)
    {
        switch (pid=fork())
        {
        case -1:
            perror("fork");
            exit(EXIT_FAILURE);

        case 0:
            if(LaunchProgram() == -1)
                perror("exec");
            exit(EXIT_SUCCESS);
        default:
            wait(NULL);
            break;
        }
    } 
}
