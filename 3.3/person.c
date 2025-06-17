#include "person.h"

PersonalInformation ReadAnthroponym() {
    PersonalInformation person;
    char buffer[20];

    printf("Введите фамилию.\n");
    scanf("%s", buffer);
    strncpy(person.anthroponym.lastName, buffer, sizeof(person.anthroponym.lastName) - 1);

    printf("Введите имя.\n");
    scanf("%s", buffer);
    strncpy(person.anthroponym.firstName, buffer, sizeof(person.anthroponym.firstName) - 1);

    printf("Введите отчество.\n");
    scanf("%s", buffer);
    strncpy(person.anthroponym.patronymic, buffer, sizeof(person.anthroponym.patronymic) - 1);
    
    return person;
}

PersonalInformation ReadingData() {
    PersonalInformation p;
    char buffer[256];

    printf("Введите фамилию.\n");
    scanf("%s", buffer);
    strncpy(p.anthroponym.lastName, buffer, sizeof(p.anthroponym.lastName) - 1);

    printf("Введите имя.\n");
    scanf("%s", buffer);
    strncpy(p.anthroponym.firstName, buffer, sizeof(p.anthroponym.firstName) - 1);

    printf("Введите отчество.\n");
    scanf("%s", buffer);
    strncpy(p.anthroponym.patronymic, buffer, sizeof(p.anthroponym.patronymic) - 1);

    printf("Введите номер телефона.\n");
    scanf("%s", buffer);
    strncpy(p.phoneNumber, buffer, sizeof(p.phoneNumber) - 1);

    printf("Хотите ввести дополнительные данные? (y/n)\n");
    scanf("%s", buffer);

    if (buffer[0] == 'y') {
        printf("Введите рабочий email.\n");
        scanf("%s", buffer);
        strncpy(p.workingEmail, buffer, sizeof(p.workingEmail) - 1);

        printf("Введите место работы.\n");
        scanf("%s", buffer);
        strncpy(p.work.workName, buffer, sizeof(p.work.workName) - 1);

        printf("Введите должность на работе.\n");
        scanf("%s", buffer);
        strncpy(p.work.workingPosition, buffer, sizeof(p.work.workingPosition) - 1);
    } 
    else {
        memset(p.workingEmail, '\0', sizeof(p.workingEmail));
        ClearWork(&p.work);
    }

    return p;
}