#ifndef AG_LOGGER
#define AG_LOGGER
//don't want multiple declarations

//include the boys
#include <string>
#include <ctime>
#include <fstream>
#include "utility.h"
#include "errorCodes.h"
#include "fileHandler.h"

//only include std I/O statements for debugging or task progress reports. Never throw errors here. Return error codes
#include <iostream>

using namespace std;

//other constants (names are usually self explanatory)
#define LOG_NAME_PREFIX "machine."
#define LOG_NAME_SUFFIX ".log"
#define KEY_VALUE_SEPERATOR ":"
#define TIMESTAMP_MSG_SEPERATOR "-"
#define LOG_FILE_BASE_PATH "/tmp/ag/"

class LogFileCreationDetails {
	
	public:
	int returnStatus;
	int errCode;
	long int bytes;
	int noOfLines;

	LogFileCreationDetails() {
		returnStatus = -1;
		errCode = 0;
		bytes = 0;
		noOfLines = 0;
	}
};

class ErrorLog {

	//-----------------------------------------------------------//
	//--------------LOGGING HELPER FUNCTIONS---------------------//
	//-----------------------------------------------------------//
	/**
	 * [utility function to convert error type into a string]
	 * @param  errType integer specifying error type
	 * @return the error type converted to string
	 */
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
	
	/**
	 * [constructs the log msg]
	 * @param  errType integer whoch holds error type
	 * @param  msg the log message
	 * @return the fully constructed log message
	 */
	string buildLogMsg(int errType, string msg) {
		
		time_t timer;

		//make the error type the key
		string logMsg(errorTypeToErrorString(errType));

		//add the seperator
		logMsg += KEY_VALUE_SEPERATOR;
		
		//construct and append the timestamp
		time(&timer);
		logMsg += Utility::intToString((int)timer);
		
		//add the seperator
		logMsg += TIMESTAMP_MSG_SEPERATOR;
		
		//append msg
		logMsg += msg;

		return logMsg;
	}

	
	//-----------------------------------------------------------//
	//--------------LOG CREATOR HELPER FUNCTIONS-----------------//
	//-----------------------------------------------------------//

	/**
	 * [get size of log file in number or bytes]
	 * @param  size size of log required
	 * @param  multiplierRep the multiplier K,M or G
	 * @return the number of bytes
	 */
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

	/**
	 * [Cap the log size to 2G]
	 * @param  size the size of log
	 * @param  multiplier the multiplier that is used. K, M or G
	 * @return the capped size
	 */
	float capSize(float size, char multiplier) {
		//cap at 2 GB
		if (multiplier == 'G' || multiplier == 'g') {
			if(size > 2.0) {
				//cout<<endl<<"Capping the log file size to 2G"<<endl;
				size = 2.0;
			}
		}
		return size;
	}

	/**
	 * [Fills the log with messages based on a probability distribution]
	 * @param  machineID the id of the machine
	 * @param  noOfLines the no of lines in the log
	 * @param  errCode pointer to store error code if any
	 * @return the success status
	 */
	int randomWriteLogFile(int machineID, int noOfLines, int *errCode) {

		int probablities[] = {0, 10, 25, 50};
		int random;
		int i = 0;
		int ret;

		srand(time(0));
		for (i =0; i < noOfLines; i++) {
			random = rand() % 100 + 1;
			if( random <= probablities[1] ) {
				ret = this->logError(machineID, 2, "Unable to connect to network. Check the connection", errCode);
			} else if ( random <= probablities[2]) {
				ret = this->logError(machineID, 3, "Failed to authenticate the peer/client. Check the HS", errCode);
			} else if ( random <= probablities[3]) {
				ret = this->logError(machineID, 4, "Could not find specified destination. Check IP and port", errCode);
			} else {
				ret = this->logError(machineID, 5, "Tried to perform an invalid operation. Check the code", errCode);
			}

			if(ret == -1)
				return ret;
		}
		return ret;
	}

	/**
	 * [constructs the log file name]
	 * @param  machineID id of machine
	 * @return log filename
	 */
	static string getLogName(int machineID) {
		//build the name, put the machine id in the middle. Seperate into three parts with '.'
		string logFileName(LOG_NAME_PREFIX);
		logFileName += Utility::intToString(machineID);
		logFileName += LOG_NAME_SUFFIX;

		return logFileName;
	}


	public:
	//-----------------------------------------------------------//
	//----------------------LOGGING FUNCTION---------------------//
	//-----------------------------------------------------------//

	/**
	 * [request to log an error]
	 * @param  machineID the machine ID
	 * @param  errType the error type
	 * @param  msg the message that has to go into the log
	 * @param  errCode space to return error code if any
	 * @return success status
	 */
	int logError(int machineID, int errType, string msg, int *errCode) {

		//get the log msg
		string logMsg = buildLogMsg(errType, msg);

		//get the log file path
		string logFilePath = getLogPath(machineID);

		//log error to file
		int success = FileHandler::writeToFile(logFilePath, logMsg, errCode);

		if( success == FAILURE) //there was file I/O error. so no point doing anything else. Just return
			return success;

		//handle any other errors other than file I/O errors here (those are handled in logToFile())

		//return success status
		return success;
	} 

	/**
	 * [constructs the log file name]
	 * @param  machineID id of machine
	 * @return log filename
	 */
	static string getLogPath(int machineID) {
		string logFileName = getLogName(machineID);

		//get the path
		string logFilePath = LOG_FILE_BASE_PATH;
		logFilePath += logFileName;

		return logFilePath;
	}

	//-----------------------------------------------------------//
	//---------------------LOG CREATOR FUNCTION------------------//
	//-----------------------------------------------------------//

	/**
	 * [create log file of specified size]
	 * @param machineID the id of the machine
	 * @param size the size of log
	 * @param multiplier the multiplie (K, M or G)
	 * @param details space to return the details about creation
	 */
	void createLogFile(int machineID, float size, char multiplier, LogFileCreationDetails *details) {
		//cap size
		size = capSize(size, multiplier);

		//get number of bytes
		details->bytes = getBytes(size, multiplier);

		//assume each line is 80 bytes
		details->noOfLines = details->bytes/80;

		//random write into log file
		details->returnStatus = randomWriteLogFile(machineID, details->noOfLines, &details->errCode);
	}

};

//end of header file
#endif