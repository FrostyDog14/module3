#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#include"person.h"
#include "list.h"


PhoneList* ReadFromFile(PhoneList* head)
{
    char file[] = "database.txt";
    int fd = open(file, O_RDONLY);
    ssize_t eof = 10;
    PersonalInformation p;
    eof = read(fd, p.anthroponym.firstName, 20);
    for(; eof != 0; )
    { 
        eof = read(fd, p.anthroponym.lastName, 20);
        eof = read(fd, p.anthroponym.patronymic, 20);
        eof = read(fd, p.phoneNumber, 12);
        head = InsertIntoList(head, p);
        eof = read(fd, p.anthroponym.firstName, 20);
    }
    
    close(fd);

    return head;
}

void WriteInFile(PhoneList* head)
{
    char file[] = "database.txt";
    int fd = open(file, O_WRONLY | O_CREAT);
    if(head == NULL)
        return;

    PhoneList* curElem = head;
    for(;curElem != NULL; curElem = curElem->next)
    {
        write(fd, curElem->person.anthroponym.firstName, 20);
        write(fd, curElem->person.anthroponym.lastName, 20);
        write(fd, curElem->person.anthroponym.patronymic, 20);
        write(fd, curElem->person.phoneNumber, 12);
    }
    
    close(fd);
}

void Interface()
{
    PhoneList* head = NULL;
    PhoneList* tmpHead = NULL;
    int selectedOption;
    PersonalInformation p;
    for (int statusExit = 0; statusExit == 0; )
    {
        printf("Выберите пункт меню:\n");
        printf("\t1) Добавить контакт в буфер.\n");
        printf("\t2) Прочитать базу данных файле.\n");
        printf("\t3) Изменить контакт в базе данных.\n");
        printf("\t4) Удалить контакт из базы данных.\n");
        printf("\t5) Вывести базу данных в буфере.\n");
        printf("\t6) Выход.\n");


        scanf("%d", &selectedOption);
        switch (selectedOption)
        {
        case 1:
            p = ReadingData();
            head = InsertIntoList(head, p);
            printf("Contact added!\n");
            break;
        case 2:
            head = ReadFromFile(head);
            break;
        case 3:
            p = ReadAnthroponym();
            tmpHead = UpdateInList(head, p);
            if (tmpHead != NULL)
            {
                head = tmpHead;
                printf("Contact redacted!\n");
            }
            else
                printf("Contact not found!\n");
               
            break;
        case 4:
            p = ReadAnthroponym();
            tmpHead = RemoveFromList(head, p);
            if (tmpHead != NULL)
            {
                head = tmpHead;
                printf("Contact deleted!\n");
            }
            else
                printf("Contact not found!\n");
            break;
        case 5:
            DisplayList(head);
            break;
        case 6:
            WriteInFile(head);
            statusExit = 1;
            break;
        default:
            printf("Wrong action.\n");
        }
    }
}

int main()
{
    Interface();
    return 0;
}