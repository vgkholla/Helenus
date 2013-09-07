#ifndef AG_FILE_HANDLER
#define AG_FILE_HANDLER

#include <string>
#include <ctime>
#include <fstream>
#include <iostream>
#include "utility.h"
#include "errorCodes.h"

using namespace std;

class FileHandler {

	public:
	/**
	 * [reads the output from a given file]
	 * @param  filePath the path of the file to be read
	 * @param  retStatus space for storing return status
	 * @param  errCode space for storing errors
	 * @return the output
	 */
	static string readFromFile(string filePath, int *retStatus, int *errCode) {
		//create an input stream
		ifstream outputFileReader(filePath.c_str());

		//some error handling
		if(!outputFileReader.is_open()) {
			//file did not open. Dunno why. Maybe perms?
			*errCode = ERROR_NO_OPEN;
			*retStatus = FAILURE;
			return "";
		}

		string line;
		string output;

		while(getline(outputFileReader, line)) { //read lines one by one
			//error handling
			if(!outputFileReader.good()) {
				outputFileReader.close();
				*errCode = ERROR_NO_READ;
				*retStatus = FAILURE;
				return "";
			}

			output += line + "\n";
		}
		//close the file
		outputFileReader.close();

		//everything was ok, so return success
		*errCode = NO_ERROR;
		*retStatus = SUCCESS;
		return output;
	}

	/**
	 * [writes the  message into the file]
	 * @param  filePath path of the file
	 * @param  msg message to be put in the file
	 * @param  errCode pointer to store error code if any
	 * @return success status
	 */
	static int writeToFile(string filePath, string msg, int *errCode) {

		//open the file
		ofstream inputFileWriter;
		inputFileWriter.open(filePath.c_str(), ios::out | ios::app);

		//some error handling
		if(!inputFileWriter.is_open()) {
			//file did not open. Dunno why. Maybe perms?
			*errCode = ERROR_NO_OPEN;
			return FAILURE;
		}

		//write to the file
		inputFileWriter<<msg<<endl;

		//error handling
		if(!inputFileWriter.good()) {
			//write failed for some reason
			if(inputFileWriter.fail() && inputFileWriter.bad()){
				//write failed because the I/O operation failed (bad and fail bit set)
				*errCode = ERROR_IO_WRITE;
				inputFileWriter.close();
				return FAILURE;
			} else if(inputFileWriter.fail()) {
				//write failed because there was a logical I/O error (only fail bit set)
				*errCode = ERROR_IO_LOGIC;
				inputFileWriter.close();
				return FAILURE;
			} else {
				//write failed. dunno exact reason
				*errCode = ERROR_NO_WRITE;
				inputFileWriter.close();
				return FAILURE;
			}
		}
		
		//close the file
		inputFileWriter.close();

		//everything was ok, so return success
		*errCode = NO_ERROR;
		return SUCCESS;

	}
};
#endif