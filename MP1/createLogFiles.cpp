#include <iostream>
#include <string>
#include <ctime>
#include "headers/logger.h"

using namespace std;

int main() {

	int machineID;
	float size;
	char multiplier;

	cout<<endl<<"Enter machine ID: ";
	cin>>machineID;

	cout<<endl<<"Enter size of log. Enter the number and the multiplier seperated by a space (e.g 1 G is 1 Giga byte)"<<endl;
	cout<<"Any floating point number accepted. Multipliers are K, M, G. Cap is 2 G: ";

	cin>>size>>multiplier;

	ErrorLog *logger = new ErrorLog();
	LogFileCreationDetails *details = new LogFileCreationDetails();

	cout<<endl<<"Creating log file. Please be patient. Bigger files take a longer time"<<endl;
	logger->createLogFile(machineID, size, multiplier, details);

	//error handling
	if(details->returnStatus == -1) {
		cout<<endl<<"****ERROR****: ";
		switch(details->errCode) {
			case 1 : 
					cout<<"Error opening file for logging"<<endl;
					break;
			case 2 : 
					cout<<"Error writing to log file"<<endl;
					break;
			case 3 : 
					cout<<"Error in I/O device write"<<endl;
					break;
			case 4 : 
					cout<<"Error in I/O device logic"<<endl;
					break;
			case 0 :
			default: 
					cout<<"Unknown error while trying to create/write log file"<<endl;
					break;
		}
		cout<<endl;
	}
	else if (details->returnStatus == 0) {
		cout<<endl<<"Successfully created log file of size "<<details->bytes<<" bytes and containing "<<details->noOfLines<<" lines"<<endl;
	}
	else {
		cout<<"Unknown return status ["<<details->returnStatus<<"] while trying to create log file. Possible logic error in logger.h";
	}

	return 0;
}