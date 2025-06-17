#include "list.h"

PhoneList* InsertIntoList(PhoneList* head, PersonalInformation newEntry) {
    PhoneList* newNode = (PhoneList*)malloc(sizeof(PhoneList));
    if (newNode == NULL) {
        perror("Failed to allocate memory");
        return head; 
    }

    newNode->next = NULL;
    newNode->prev = NULL;

    
    strncpy(newNode->person.anthroponym.firstName, newEntry.anthroponym.firstName, sizeof(newNode->person.anthroponym.firstName) - 1);
    strncpy(newNode->person.anthroponym.lastName, newEntry.anthroponym.lastName, sizeof(newNode->person.anthroponym.lastName) - 1);
    strncpy(newNode->person.anthroponym.patronymic, newEntry.anthroponym.patronymic, sizeof(newNode->person.anthroponym.patronymic) - 1);
    strncpy(newNode->person.phoneNumber, newEntry.phoneNumber, sizeof(newNode->person.phoneNumber) - 1);
    strncpy(newNode->person.workingEmail, newEntry.workingEmail, sizeof(newNode->person.workingEmail) - 1);
    strncpy(newNode->person.work.workName, newEntry.work.workName, sizeof(newNode->person.work.workName) - 1);
    strncpy(newNode->person.work.workingPosition, newEntry.work.workingPosition, sizeof(newNode->person.work.workingPosition) - 1);

    if (head == NULL) {
        return newNode; 
    }

    PhoneList* current = head;
    PhoneList* previous = NULL;

    while (current != NULL) {
        if (LastnameComparer(newNode->person.anthroponym, current->person.anthroponym)) {
            if (current == head) {
                newNode->next = current;
                current->prev = newNode;
                return newNode; 
            } else {
                newNode->next = current;
                newNode->prev = previous;
                current->prev = newNode;
                previous->next = newNode;
                return head; 
            }
        }
        previous = current;
        current = current->next;
    }

    previous->next = newNode;
    newNode->prev = previous;
    return head; 
}

PhoneList* SearchInList(PhoneList* head, PersonalInformation target) {
    PhoneList* current = head;
    while (current != NULL) {
        if (strcmp(target.anthroponym.lastName, current->person.anthroponym.lastName) == 0 &&
            strcmp(target.anthroponym.firstName, current->person.anthroponym.firstName) == 0 &&
            strcmp(target.anthroponym.patronymic, current->person.anthroponym.patronymic) == 0) {
            return current; 
        }
        current = current->next;
    }
    return NULL; 
}

PhoneList* RemoveFromList(PhoneList* head, PersonalInformation target) {
    PhoneList* nodeToRemove = SearchInList(head, target);

    if (nodeToRemove != NULL) {
        if (head == nodeToRemove) {
            head = head->next;
            if (head != NULL) {
                head->prev = NULL; 
            }
            free(nodeToRemove);
            return head; 
        } else {
            if (nodeToRemove->next != NULL) {
                nodeToRemove->prev->next = nodeToRemove->next;
                nodeToRemove->next->prev = nodeToRemove->prev;
            } else {
                nodeToRemove->prev->next = NULL; 
            }
            free(nodeToRemove);
            return head; 
        }
    }
    return head; 
}

PhoneList* UpdateInList(PhoneList* head, PersonalInformation target) {
    PhoneList* nodeToUpdate = SearchInList(head, target);
    if (nodeToUpdate != NULL) {
        char buffer[20];
        printf("Введите новое имя.\n");
        scanf("%s", buffer);
        strncpy(nodeToUpdate->person.anthroponym.firstName, buffer, sizeof(nodeToUpdate->person.anthroponym.firstName) - 1);

        printf("Введите новую фамилию.\n");
        scanf("%s", buffer);
        strncpy(nodeToUpdate->person.anthroponym.lastName, buffer, sizeof(nodeToUpdate->person.anthroponym.lastName) - 1);

        printf("Введите новое отчество.\n");
        scanf("%s", buffer);
        strncpy(nodeToUpdate->person.anthroponym.patronymic, buffer, sizeof(nodeToUpdate->person.anthroponym.patronymic) - 1);

        printf("Введите новый номер телефона.\n");
        scanf("%s", buffer);
        strncpy(nodeToUpdate->person.phoneNumber, buffer, sizeof(nodeToUpdate->person.phoneNumber) - 1);
        return head; 
    }
    return head; 
}

void DisplayList(PhoneList* head) {
    PhoneList* current = head;
    for (int index = 1; current != NULL; index++) {
        printf("%d) Имя: %s %s %s ", index, current->person.anthroponym.lastName, current->person.anthroponym.firstName, current->person.anthroponym.patronymic);
        printf("Телефон: %s\n", current->person.phoneNumber);
        current = current->next;
    }
    printf("\n");
}