#ifndef PERSONAL_INFORMATION_H_
#define PERSONAL_INFORMATION_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "anthroponym.h"
#include "work.h"

typedef struct PersonalInformation {
    Anthroponym anthroponym; 
    Work work;               
    char workingEmail[20];   
    char phoneNumber[12];    
} PersonalInformation;

PersonalInformation ReadAnthroponym();

PersonalInformation ReadingData();

#endif 
