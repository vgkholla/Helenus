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
#define AG_LOG_ERROR 1
#define AG_LOG_DEBUG 2
#define AG_LOG_NOTICE 3

#define LOGGING_LEVEL AG_LOG_DEBUG
#define VERBOSE 0

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

	int machineID;

	//-----------------------------------------------------------//
	//--------------LOG CREATOR HELPER FUNCTIONS-----------------//
	//-----------------------------------------------------------//

	/**
	 * [get size of log file in number or bytes]
	 * @param  size 			[size of log required]
	 * @param  multiplierRep 	[the multiplier K,M or G]
	 * @return [the number of bytes]
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
	 * @param  size 		[the size of log]
	 * @param  multiplier 	[the multiplier that is used. K, M or G]
	 * @return [the capped size]
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
	 * @param  noOfLines 	[the no of lines in the log]
	 * @param  errCode 		[pointer to store error code if any]
	 * @return [the success status]
	 */
	int randomWriteLogFile(int noOfLines, int *errCode) {

		int probablities[] = {0, 10, 25, 50};
		int random;
		int i = 0;
		int ret;

		srand(time(0));
		for (i =0; i < noOfLines; i++) {
			random = rand() % 100 + 1;
			if( random <= probablities[1] ) {
				ret = this->logError(2, "Unable to connect to network. Check the connection", errCode);
			} else if ( random <= probablities[2]) {
				ret = this->logError(3, "Failed to authenticate the peer/client. Check the HS", errCode);
			} else if ( random <= probablities[3]) {
				ret = this->logError(4, "Could not find specified destination. Check IP and port", errCode);
			} else {
				ret = this->logError(5, "Tried to perform an invalid operation. Check the code", errCode);
			}

			if(ret == -1)
				return ret;
		}
		return ret;
	}

	//-----------------------------------------------------------//
	//--------------LOGGING HELPER FUNCTIONS---------------------//
	//-----------------------------------------------------------//
	/**
	 * [utility function to convert error type into a string]
	 * @param  errType [integer specifying error type]
	 * @return [the error type converted to string]
	 */
	string errorTypeToErrorString(int errType) {
		string errMsg;
		switch (errType) {
			case IO_ERROR: 
				errMsg = "IO_ERROR";
				break;
			case NETWORK_ERROR:
				errMsg = "NETWORK_ERROR";
				break;
			case HANDSHAKE_FAILURE:
				errMsg = "HANDSHAKE_FAILURE";
				break;
			case DEST_NOT_FOUND:
				errMsg = "DEST_NOT_FOUND";
				break;
			case INVALID_OPERATION:
				errMsg = "INVALID_OPERATION";
				break;
			case INDEX_ACCESS_FAILED:
				errMsg = "INDEX_ACCESS_FAILED";
				break;
			case EMPTY_LIST:
				errMsg = "EMPTY_LIST";
				break;
			case SELF_ENTRY_NOT_FOUND:
				errMsg = "SELF_ENTRY_NOT_FOUND";
				break;
			case ERROR_ALREADY_EXISTS:
				errMsg = "ERROR_ALREADY_EXISTS";
				break;
            case SERIALIZATION_ERROR:
                errMsg = "SERIALIZATION_ERROR";
                break;
            case SOCKET_ERROR:
                errMsg = "SOCKET_ERROR";
                break;
            case NO_SUCH_KEY:
            	errMsg = "NO_SUCH_KEY";
            	break;
           	case TOO_MANY_KEYS:
           		errMsg = "TOO_MANY_KEYS";
           		break;
           	case KEY_EXISTS:
           		errMsg = "KEY_EXISTS";
           		break;
           	case INSERT_FAILED:
           		errMsg = "INSERT_FAILED";
           		break;
           	case UPDATE_FAILED:
           		errMsg = "UPDATE_FAILED";
           		break;
           	case MALFORMED_COMMAND:
           		errMsg = "MALFORMED_COMMAND";
           		break;
			default:
				errMsg = "UNKNOWN_ERROR";
				break;
		}
		return errMsg;
	}

	/**
	 * [utility function to convert debug type into a string]
	 * @param  debugType [integer specifying debug type]
	 * @return [the debug type converted to string]
	 */
	string debugTypeToDebugString(int debugType) {
		string debugMsg;
		switch (debugType) {
			case MEMBER_JOINED:
				debugMsg = "MEMBER_JOINED";
				break;
			case MEMBER_LEFT:
				debugMsg = "MEMBER_LEFT";
				break;
			case MEMBER_SUSPECTED_FAILED:
				debugMsg = "MEMBER_SUSPECTED_FAILED";
				break;
			case MEMBER_FAILED:
				debugMsg = "MEMBER_FAILED";
				break;
			default:
				debugMsg = "UNKNOWN_DEBUG";
				break;
		}
		return debugMsg;
	}
	
	/**
	 * [constructs the log msg]
	 * @param  msgType 	[integer whoch holds msg type]
	 * @param  type 	[integer whoch holds error/debug type]
	 * @param  msg 		[the log message]
	 * @return [the fully constructed log message]
	 */
	string buildLogMsg(int msgType, int type, string msg) {
		
		time_t timer;
		string logMsg = "";

		if(msgType == AG_LOG_DEBUG) {
			//make the type the key
			logMsg += debugTypeToDebugString(type);

		} else {
			//assume it's an error and make the  type the key
			logMsg += errorTypeToErrorString(type);
		}

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

	/**
	 * [constructs the log file name]
	 * @param  machineID [id of machine]
	 * @return [log filename]
	 */
	static string getLogName(int machineID) {
		//build the name, put the machine id in the middle. Seperate into three parts with '.'
		string logFileName(LOG_NAME_PREFIX);
		logFileName += Utility::intToString(machineID);
		logFileName += LOG_NAME_SUFFIX;

		return logFileName;
	}


	public:

	ErrorLog(int ID) {
		machineID = ID;
	}

	//-----------------------------------------------------------//
	//----------------------LOGGING FUNCTION---------------------//
	//-----------------------------------------------------------//

	/**
	 * [request to log an error]
	 * @param  errType 		[the error type]
	 * @param  msg the 		[message that has to go into the log]
	 * @param  errCode 		[space to return error code if any]
	 * @return success 		[status]
	 */
	int logError(int errType, string msg, int *errCode) {

		if(LOGGING_LEVEL < AG_LOG_ERROR) {
			return SUCCESS;
		}

		//get the log msg
		string logMsg = buildLogMsg(AG_LOG_ERROR,errType, msg);

		//get the log file path
		string logFilePath = getLogPath(machineID);

		//log error to file
		int success = FileHandler::writeToFile(logFilePath, logMsg, errCode);

		if(VERBOSE) {
			cout<<logMsg<<endl;
		}

		if(*errCode != NO_ERROR && *errCode <= HIGHEST_FILE_ERROR_CODE) {//oh dear
				//the logger might throw an exception too. give up and throw an output to terminal in this case
				cout<<"ERRORCEPTION: Error logger threw an error while logging an error. Oh the irony... The log message would have been '" << msg <<"'. Error code for log message failure: "<< *errCode<<endl;
		}

		//handle any other errors other than file I/O errors here (those are handled in logToFile())

		//return success status
		return success;
	}

	/**
	 * [request to log a debug]
	 * @param  debugType 	[the debug type]
	 * @param  msg 			[the message that has to go into the log]
	 * @param  errCode 		[space to return error code if any]
	 * @return success 		[status]
	 */
	int logDebug(int debugType, string msg, int *errCode) {

		if(LOGGING_LEVEL < AG_LOG_DEBUG) {
			return SUCCESS;
		}

		//get the log msg
		string logMsg = buildLogMsg(AG_LOG_DEBUG,debugType, msg);

		//get the log file path
		string logFilePath = getLogPath(machineID);

		//log error to file
		int success = FileHandler::writeToFile(logFilePath, logMsg, errCode);

		if(VERBOSE) {
			cout<<logMsg<<endl;
		}

		if(*errCode != NO_ERROR && *errCode <= HIGHEST_FILE_ERROR_CODE) {//oh dear
				//the logger might throw an exception too. give up and throw an output to terminal in this case
				cout<<"ERROR: Debug logger threw an error while logging a debug. The log message would have been '" << msg <<"'. Error code for log message failure: "<< *errCode<<endl;
		}

		//handle any other errors other than file I/O errors here (those are handled in logToFile())

		//return success status
		return success;
	} 

	/**
	 * [constructs the log file path]
	 * @param  machineID [id of machine]
	 * @return [log filepath]
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
	 * @param size 			[the size of log]
	 * @param multiplier 	[the multiplie (K, M or G)]
	 * @param details 		[space to return the details about creation]
	 */
	void createLogFile(float size, char multiplier, LogFileCreationDetails *details) {
		//cap size
		size = capSize(size, multiplier);

		//get number of bytes
		details->bytes = getBytes(size, multiplier);

		//assume each line is 80 bytes
		details->noOfLines = details->bytes/80;

		//random write into log file
		details->returnStatus = randomWriteLogFile(details->noOfLines, &details->errCode);
	}

};

//end of header file
#endif
