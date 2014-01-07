#include <iostream>
#include "headers/logger.h"

using namespace std;

int main() {

	ErrorLog *logger = new ErrorLog();
	int errCode;
	int ret = logger->logError(1, 1, "testing", &errCode);
	ret = logger->logError(1, 2, "testing", &errCode);
	return 0;
}