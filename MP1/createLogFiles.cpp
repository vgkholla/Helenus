#include <iostream>
#include <string>
#include <ctime>
#include "headers/logger.h"

using namespace std;

long int getBytes(float size, char multiplierRep) {
	long int multiplier = 0;
	switch(multiplierRep) {
		case 'K':
		case 'k':
			multiplier = 1024;
			break;
		case 'M':
		case 'm':
			multiplier = 1024 * 1024;
			break;
		case 'G':
		case 'g':
			multiplier = 1024 * 1024 * 1024;
			break;
		default:
			multiplier = 0;
			break;
	}
	//cout<<endl<<"The multiplier is "<<multiplier<<endl;
	return (long int)(size * multiplier); 
}

int main() {

	int machineID;
	float size;
	char multiplier;

	cout<<endl<<"Enter machine ID: ";
	cin>>machineID;

	cout<<endl<<"Enter size of log. Enter the number and the multiplier seperated by a space (e.g 1 G is 1 Giga byte)"<<endl;
	cout<<"Any floating point number accepted. Multipliers are K, M, G: ";

	cin>>size>>multiplier;

	//cap at 2 GB
	if (multiplier == 'G' || multiplier == 'g') {
		if(size > 2) {
			cout<<endl<<"Capping the log file size to 2G"<<endl;
			size = 2;
		}
	}

	cout<<endl<<"Creating a log file with size "<<size<<multiplier<<endl;
	long int bytes = getBytes(size, multiplier);

	cout<<endl<<"Size of file in bytes = "<<bytes<<" bytes"<<endl;;
	//assume each line is 40 bytes
	int noOfLines = bytes/40;

	cout<<endl<<"Number of lines in the log file will be "<<noOfLines<<endl;

	int probablities[] = {0, 10, 20, 25, 45};
	int random;
	int i = 0;
	
	ErrorLog *logger = new ErrorLog();
	int errCode;
	int ret;

	char answer;

	srand(time(0));
	for (i =0; i < noOfLines; i++) {
		random = rand() % 100 + 1;
		if( random <= probablities[1] ) {
			ret = logger->logError(machineID, 2, "Unable to connect to network. Check the connection", &errCode);
		}
		else if ( random <= probablities[2]) {
			ret = logger->logError(machineID, 3, "Failed to authenticate the peer/client. Check the HS", &errCode);
		}
		else if ( random <= probablities[3]) {
			ret = logger->logError(machineID, 4, "Could not find specified destination. Check IP and port", &errCode);
		}
		else if ( random <= probablities[4]) {
			ret = logger->logError(machineID, 5, "Tried to perform an invalid operation. Check the code", &errCode);
		}
		if(ret == -1) {
			//there was an error with logging
			cout<<"Logging failed with error code =["<<errCode<<"] . Do you want to continue? (y or n): ";
			cin>>answer;

			if(answer != 'y' && answer != 'Y')
				break;
		}
	}
	if( i == noOfLines) {
		cout<<"Log file for machine with machineID ["<<machineID<<"] created"<<endl;
	}
	else {
		cout<<"Creation of log file for machine with machineID ["<<machineID<<"] failed"<<endl;
	}



	return 0;
}