#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/timerfd.h>
#include <mqueue.h>
#include <poll.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include <stdint.h>

#define MAX_DRIVERS 100
#define MQ_NAME_LEN 64

typedef enum
{
    MSG_STATUS,
    MSG_TASK,
    MSG_BUSY,
    MSG_AVAILABLE,
    MSG_ERROR_BUSY,
    MSG_EXIT
} msg_type_t;

typedef struct
{
    msg_type_t type;
    int task_time;
} driver_msg_t;

typedef enum
{
    DRIVER_AVAILABLE,
    DRIVER_BUSY
} driver_status_t;

typedef struct
{
    pid_t pid;
    char mq_name_cmd[MQ_NAME_LEN];
    char mq_name_resp[MQ_NAME_LEN];
    driver_status_t status;
    int busy_time_left;
    mqd_t mq_cmd;
    mqd_t mq_resp;
} driver_info_t;

driver_info_t drivers[MAX_DRIVERS];
int driver_count = 0;

// Найти драйвера по pid
driver_info_t *find_driver(pid_t pid)
{
    for (int i = 0; i < driver_count; ++i)
        if (drivers[i].pid == pid)
            return &drivers[i];
    return NULL;
}

// Добавить драйвера в список
void add_driver(pid_t pid)
{
    if (driver_count >= MAX_DRIVERS)
    {
        printf("Максимальное количество драйверов достигнуто\n");
        return;
    }
    driver_info_t *d = &drivers[driver_count++];
    d->pid = pid;
    snprintf(d->mq_name_cmd, MQ_NAME_LEN, "/driver_cmd_%d", pid);
    snprintf(d->mq_name_resp, MQ_NAME_LEN, "/driver_resp_%d", pid);
    d->status = DRIVER_AVAILABLE;
    d->busy_time_left = 0;
}

// Открыть очереди драйвера
int open_driver_queues(driver_info_t *d)
{
    d->mq_cmd = mq_open(d->mq_name_cmd, O_WRONLY);
    if (d->mq_cmd == (mqd_t)-1) return -1;
    d->mq_resp = mq_open(d->mq_name_resp, O_RDONLY);
    if (d->mq_resp == (mqd_t)-1) {
        mq_close(d->mq_cmd);
        return -1;
    }
    return 0;
}

// Закрыть очереди драйвера
void close_driver_queues(driver_info_t *d)
{
    if (d->mq_cmd != (mqd_t)-1) mq_close(d->mq_cmd);
    if (d->mq_resp != (mqd_t)-1) mq_close(d->mq_resp);
}

// Обновить статус драйвера
void update_driver_status(pid_t pid)
{
    driver_info_t *d = find_driver(pid);
    if (!d)
    {
        printf("Драйвер с PID %d не найден\n", pid);
        return;
    }

    if (open_driver_queues(d) == -1)
        return;

    driver_msg_t msg;
    msg.type = MSG_STATUS;

    if (mq_send(d->mq_cmd, (char *)&msg, sizeof(msg), 0) == -1)
    {
        perror("mq_send");
        close_driver_queues(d);
        return;
    }

    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += 2; // тайм-аут 2 секунды

    driver_msg_t reply;
    ssize_t n = mq_timedreceive(d->mq_resp, (char *)&reply, sizeof(reply), NULL, &ts);

    if (n == -1)
    {
        perror("mq_timedreceive");
        close_driver_queues(d);
        return;
    }

    if (reply.type == MSG_BUSY)
    {
        d->status = DRIVER_BUSY;
        d->busy_time_left = reply.task_time;
    }

    else if (reply.type == MSG_AVAILABLE)
    {
        d->status = DRIVER_AVAILABLE;
        d->busy_time_left = 0;
    }
}

// Получить статус драйвера
int get_status(pid_t pid)
{
    update_driver_status(pid);
    driver_info_t *d = find_driver(pid);
    
    if (!d)
    {
        printf("Драйвер с PID %d не найден\n", pid);
        return -1;
    }

    if (d->status == DRIVER_BUSY)
    {
        printf("Driver %d: Busy (%d seconds left)\n", pid, d->busy_time_left);
    }
    else if (d->status == DRIVER_AVAILABLE)
    {
        printf("Driver %d: Available\n", pid);
    }
    else
    {
        printf("Неизвестный ответ от драйвера %d\n", pid);
    }

    return 0;
}

// Отправить задачу драйверу
int send_task(pid_t pid, int task_time)
{
    update_driver_status(pid);
    driver_info_t *d = find_driver(pid);
    if (!d)
    {
        printf("Драйвер с PID %d не найден\n", pid);
        return -1;
    }

    if (d->status == DRIVER_BUSY)
    {
        printf("Driver %d занят, нельзя отправить новую задачу\n", pid);
        return -1;
    }

    if (open_driver_queues(d) == -1)
        return -1;

    driver_msg_t msg;
    msg.type = MSG_TASK; 
    msg.task_time = task_time;

    if (mq_send(d->mq_cmd, (char *)&msg, sizeof(msg), 0) == -1)
    {
        perror("mq_send task");
        close_driver_queues(d);
        return -1;
    }
    else
    {
        printf("Driver %d начал задачу на %d секунд\n", pid, task_time);
    }

    close_driver_queues(d);
    return 0;
}

// Показать статусы всех драйверов
void get_drivers()
{
    printf("Список драйверов:\n");
    for (int i = 0; i < driver_count; ++i)
    {
        get_status(drivers[i].pid);
    }
}

void driver_process(const char *mq_name_cmd, const char *mq_name_resp)
{
    struct mq_attr attr = {0};
    struct timespec task_start_time = {0};
    int task_duration = 0; // длительность задачи в секундах

    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(driver_msg_t);

    mq_unlink(mq_name_cmd);
    mq_unlink(mq_name_resp);

    mqd_t mq_cmd = mq_open(mq_name_cmd, O_CREAT | O_RDONLY, 0644, &attr);
    if (mq_cmd == (mqd_t)-1)
    {
        perror("mq_open cmd");
        exit(1);
    }

    mqd_t mq_resp = mq_open(mq_name_resp, O_CREAT | O_WRONLY, 0644, &attr);
    if (mq_resp == (mqd_t)-1)
    {
        perror("mq_open resp");
        mq_close(mq_cmd);
        mq_unlink(mq_name_cmd);
        exit(1);
    }

    driver_status_t status = DRIVER_AVAILABLE;

    int timer_fd = timerfd_create(CLOCK_REALTIME, 0);
    if (timer_fd == -1)
    {
        perror("timerfd_create");
        exit(1);
    }

    struct pollfd pfds[2];
    pfds[0].fd = mq_cmd;
    pfds[0].events = POLLIN;
    pfds[1].fd = timer_fd;
    pfds[1].events = POLLIN;

    while (1)
    {
        int ret = poll(pfds, 2, -1);

        if (ret == -1)
        {
            perror("poll");
            break;
        }

        if (pfds[0].revents & POLLIN)
        {
            driver_msg_t msg;
            ssize_t n = mq_receive(mq_cmd, (char *)&msg, sizeof(msg), NULL);

            if (n > 0)
            {
                if (msg.type == MSG_STATUS)
                {
                    driver_msg_t reply;
                    if (status == DRIVER_BUSY)
                    {
                        struct timespec now;
                        clock_gettime(CLOCK_MONOTONIC, &now);
                        int elapsed = now.tv_sec - task_start_time.tv_sec;
                        int time_left = task_duration - elapsed;

                        if (time_left <= 0)
                        {
                            status = DRIVER_AVAILABLE;
                            reply.type = MSG_AVAILABLE;
                            reply.task_time = 0;
                        }
                        else
                        {
                            reply.type = MSG_BUSY;
                            reply.task_time = time_left;
                        }
                    }
                    else
                    {
                        reply.type = MSG_AVAILABLE;
                        reply.task_time = 0;
                    }
                    mq_send(mq_resp, (char *)&reply, sizeof(reply), 0);
                }
                else if (msg.type == MSG_TASK)
                {
                // Здесь обработка новой задачи
                    status = DRIVER_BUSY;
                    task_duration = msg.task_time;
                    clock_gettime(CLOCK_MONOTONIC, &task_start_time);

                    struct itimerspec its = {0};
                    its.it_value.tv_sec = task_duration;
                    timerfd_settime(timer_fd, 0, &its, NULL);
                }
            }
        }
        else if (pfds[1].revents & POLLIN)
        {
        uint64_t expirations;
        ssize_t s = read(timer_fd, &expirations, sizeof(expirations));
        (void)s;

        status = DRIVER_AVAILABLE;
        task_duration = 0;
        memset(&task_start_time, 0, sizeof(task_start_time));
        }
    }
mq_close(mq_cmd);
mq_close(mq_resp);
mq_unlink(mq_name_cmd);
mq_unlink(mq_name_resp);
}

int main()
{
    printf("Такси система запущена\n");

    char line[128];
    while (1)
    {
        printf("\nВведите команду:create_driver, send_task <pid> <task_time>, get_drivers, send_task <pid> <task_time>, get_status <pid>, get_drivers\n");
        printf("> ");

        if (!fgets(line, sizeof(line), stdin))
            break;

        if (strncmp(line, "create_driver", 13) == 0)
        {
            pid_t child = fork();
            if (child == -1)
            {
                perror("fork");
                continue;
            }
            if (child == 0)
            {
                // В дочернем процессе запускаем драйвер
                pid_t pid = getpid();
                char mq_cmd[MQ_NAME_LEN], mq_resp[MQ_NAME_LEN];
                snprintf(mq_cmd, MQ_NAME_LEN, "/driver_cmd_%d", pid);
                snprintf(mq_resp, MQ_NAME_LEN, "/driver_resp_%d", pid);
                driver_process(mq_cmd, mq_resp);
                exit(0);
            }
            else
            {
                // В родительском процессе добавляем драйвера
                add_driver(child);
                printf("Создан драйвер с PID %d\n", child);
            }
        }
        else if (strncmp(line, "send_task", 9) == 0)
        {
            pid_t pid;
            int time;
            if (sscanf(line + 10, "%d %d", &pid, &time) == 2)
            {
                send_task(pid, time);
            }
            else
            {
                printf("Неверный формат. Используйте: send_task <pid> <task_time>\n");
            }
        }
        else if (strncmp(line, "get_status", 10) == 0)
        {
            pid_t pid;
            if (sscanf(line + 11, "%d", &pid) == 1)
            {
                get_status(pid);
            }
            else
            {
                printf("Неверный формат. Используйте: get_status <pid>\n");
            }
        }
        else if (strncmp(line, "get_drivers", 11) == 0)
        {
            get_drivers();
        }
        else
        {
            printf("Неизвестная команда\n");
        }
    }
    printf("Программа завершена\n");
    return 0;
}