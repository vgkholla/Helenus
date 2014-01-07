#include <iostream>
#include <string>
#include <ctime>
#include "../../headers/logger.h"
#include "../../headers/errorCodes.h"

using namespace std;

int main(int argc, char *argv[]) {

	if(argc < 4) {
		cout<<"Incorrect number of arguments provided. Exiting.."<<endl;
		exit(1);
	}

	int machineID = atoi(argv[1]);
	float size = atof(argv[2]);
	char multiplier = *argv[3];


	ErrorLog *logger = new ErrorLog();
	LogFileCreationDetails *details = new LogFileCreationDetails();
	logger->createLogFile(machineID, size, multiplier, details);

	//error handling
	if(details->returnStatus == -1) {
		cout<<endl<<"****ERROR****: ";
		switch(details->errCode) {
			case ERROR_NO_OPEN : 
					cout<<"Error opening file for logging"<<endl;
					break;
			case ERROR_NO_WRITE : 
					cout<<"Error writing to log file"<<endl;
					break;
			case ERROR_IO_WRITE : 
					cout<<"Error in I/O device write"<<endl;
					break;
			case ERROR_IO_LOGIC : 
					cout<<"Error in I/O device logic"<<endl;
					break;
			case 0 :
			default: 
					cout<<"Unknown error while trying to create/write log file"<<endl;
					break;
		}
		cout<<endl;
	} else if (details->returnStatus == 0) {
		//cout<<"Succesfully wrote "<<details->noOfLines<<" lines in the log file"<<endl;
	} else {
		cout<<"Unknown return status ["<<details->returnStatus<<"] while trying to create log file. Possible logic error in logger.h";
	}

	return 0;
}