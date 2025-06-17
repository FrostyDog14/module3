#ifndef PHONE_LIST_H_
#define PHONE_LIST_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "person.h"


typedef struct PhoneListNode {
    PersonalInformation person;
    struct PhoneListNode* prev;
    struct PhoneListNode* next;
} PhoneList;

PhoneList* InsertIntoList(PhoneList* head, PersonalInformation newEntry);

PhoneList* SearchInList(PhoneList* head, PersonalInformation target);

PhoneList* RemoveFromList(PhoneList* head, PersonalInformation target);

PhoneList* UpdateInList(PhoneList* head, PersonalInformation target);

void DisplayList(PhoneList* head);

#endif 
