#include "work.h"
#include <string.h>

void ClearWork(Work* work)
{
	memset(work->workName, '\0', 20);
	memset(work->workingPosition, '\0', 20);
}