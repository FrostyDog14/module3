#ifndef WORK_H_
#define WORK_H_

typedef struct Work {
    char workName[20];
    char workingPosition[20];
} Work;

void ClearWork(Work* work);

#endif 
