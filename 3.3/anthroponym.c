#include "anthroponym.h"


int LastnameComparer(Anthroponym firstPerson, Anthroponym secondPerson) {
    int firstLength = strlen(firstPerson.lastName);
    int secondLength = strlen(secondPerson.lastName);

    
    if (firstLength == secondLength) {
        for (int i = 0; i < firstLength; i++) {
            if (firstPerson.lastName[i] < secondPerson.lastName[i]) {
                return 1; 
            } 
            else if (firstPerson.lastName[i] > secondPerson.lastName[i]) {
                return 0; 
            }
        }
        return 0; 
    } 
    else {
        return (firstLength < secondLength) ? 1 : 0;
    }
}
