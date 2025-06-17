#ifndef ANTHROPONYM_COMPARISON_H
#define ANTHROPONYM_COMPARISON_H

#include <string.h>

typedef struct {
    char lastName[20];
    char firstName[20];
    char patronymic[20];
} Anthroponym;


int LastnameComparer(Anthroponym firstPerson, Anthroponym secondPerson);

#endif 
