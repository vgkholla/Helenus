//don't want multiple declarations
#ifndef AG_LOGGER
#define AG_LOGGER

//include the boys
#include <iomanip>
#include <locale>
#include <sstream>
#include <string>
#include <ctime>
#include <fstream>

//only include std I/O statements for debugging. Never throw errors here. Return error codes
#include <iostream>

using namespace std;

//error types go here
#define IO_ERROR 1
#define NETWORK_ERROR 2
#define HANDSHAKE_FAILURE 3
#define DEST_NOT_FOUND 4
#define INVALID_OPERATION 5

//error codes go here
#define NO_ERROR 0
#define ERROR_NO_OPEN 1
#define ERROR_NO_WRITE 2
#define ERROR_IO_WRITE 3
#define ERROR_IO_LOGIC 4

//other constants (names are usually self explanatory)
#define LOG_NAME_PREFIX "machine."
#define LOG_NAME_SUFFIX ".log"
#define KEY_VALUE_SEPERATOR ":"
#define TIMESTAMP_MSG_SEPERATOR "-"
#define LOG_FILE_BASE_PATH "/tmp/"
#define SUCCESS 0 //code for success
#define FAILURE -1//code for fail

class ErrorLog {
	//utility function to convert an integer to a string
	string intToString(int num) {
		return static_cast<ostringstream*>( &(ostringstream() << num) )->str();
	} 

	//utility function to convert error type into a string
	string errorTypeToErrorString(int errType) {
		switch (errType) {
			case 1: 
				return "IO_ERROR";
				break;
			case 2:
				return "NETWORK_ERROR";
				break;
			case 3:
				return "HANDSHAKE_FAILURE";
				break;
			case 4:
				return "DEST_NOT_FOUND";
				break;
			case 5:
				return "INVALID_OPERATION";
				break;
			default:
				return "UNKNOWN_ERROR";
				break;
		}
	}

	//constructs the log file name
	string getLogName(int machineID) {

		//build the name, put the machine id in the middle. Seperate into three parts with '.'
		string logFileName(LOG_NAME_PREFIX);
		logFileName += intToString(machineID);		
		logFileName += LOG_NAME_SUFFIX;

		return logFileName;
	}

	//constructs the log msg
	string buildLogMsg(int errType, string msg) {
		
		time_t timer;

		//make the error type the key
		string logMsg(errorTypeToErrorString(errType));

		//add the seperator
		logMsg += KEY_VALUE_SEPERATOR;
		
		//construct and append the timestamp
		time(&timer);
		logMsg += intToString((int)timer);
		
		//add the seperator
		logMsg += TIMESTAMP_MSG_SEPERATOR;
		
		//append msg
		logMsg += msg;

		return logMsg;
	}

	int logToFile(string logFileName, string logMsg, int *errCode) {
		
		//get the path
		string logFilePath = LOG_FILE_BASE_PATH;
		logFilePath += logFileName;

		//open the file
		ofstream logFileWriter;
		logFileWriter.open(logFilePath.c_str(), ios::out | ios::app);

		//some error handling
		if(!logFileWriter.is_open()) {
			//file did not open. Dunno why. Maybe perms?
			*errCode = ERROR_NO_OPEN;
			return FAILURE;
		}

		//write to the file
		logFileWriter<<logMsg<<endl;

		//error handling
		if(!logFileWriter.good()) {
			//write failed for some reason
			if(logFileWriter.fail() && logFileWriter.bad()){
				//write failed because the I/O operation failed (bad and fail bit set)
				*errCode = ERROR_IO_WRITE;
				return FAILURE;
			}
			else if(logFileWriter.fail()) {
				//write failed because there was a logical I/O error (only fail bit set)
				*errCode = ERROR_IO_LOGIC;
				return FAILURE;
			}
			else {
				//write failed. dunno exact reason
				*errCode = ERROR_NO_WRITE;
				return FAILURE;
			}
		}
		
		//close the file
		logFileWriter.close();

		//everything was ok, so return success
		*errCode = NO_ERROR;
		return SUCCESS;

	}

	public:
	int logError(int machineID, int errType, string msg, int *errCode) {

		//get the log msg
		string logMsg = buildLogMsg(errType, msg);

		//get the log file name
		string logFile = getLogName(machineID);

		//log error to file
		int success = logToFile(logFile, logMsg, errCode);

		if( success == FAILURE) //there was file I/O error. so no point doing anything else. Just return
			return success;

		//handle any other errors other than file I/O errors here (those are handled in logToFile())

		//return success status
		return success;
	} 
};

//end of header file
#endif