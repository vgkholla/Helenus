#ifndef AG_CLT
#define AG_CLT
//don't want multiple declarations

#include <iostream>
#include <string>
#include <stdio.h>
#include <fstream>
#include <sstream>
#include "utility.h"
#include "errorCodes.h"

using namespace std;

//error codes go here
#define NO_ERROR 0
#define COMMAND_PROCESSOR_NOT_AVAILABLE 1

//other constants
#define PROMPT ">>"
#define O_FILE_NAME_PREFIX "cmd."
#define O_FILE_NAME_SUFFIX ".out"
#define OUTPUT_FILE_BASE_PATH "/tmp/"

class CommandResultDetails{

	public:

	int returnStatus;
	int errCode;

	CommandResultDetails() {
		returnStatus = -1;
		errCode = 0;
	}
};

class CommandLineTools {

	/**
	 * [tags the file to say which machine output came from]
	 * @param  machineID 
	 * @param  filePath
	 * @return command exit status
	 * IMP: before you call this function, make sure the command processor is available. (a call to system(NULL)). For an example, check CommandLineTools::executeCmd.
	 */
	static void tagFile(int machineID, string filePath, CommandResultDetails *details) {
		//first build the tag command for tagging the file
		string tagCmd = "echo **Output of command from machine [" + Utility::intToString(machineID) + "]** > " + filePath + " 2>&1";
		//tag it!!
		executeCmd(tagCmd, details);
	}

	public:
	/**
	 * [Shows the prompt and takes the input]
	 * @param  machineID the machine ID
	 * @return the cmd entered by the user
	 */
	static string showAndHandlePrompt(int machineID) {
		
		string cmd;

		//can customize prompt based on machine id if we want
		cout<<machineID<<PROMPT;

		//take user input
		getline(cin, cmd);

		//find a way of santizing command. A white list perhaps?

		//return the command
		return cmd;
	}

	/**
	 * [Execute the given command and write the output to a file]
	 * @param  machineID the machine ID
	 * @param  cmd the command to be executed
	 * @param  details pointer to where details of command execution can be stored
	 * @return the file path of the file containing the output
	 */
	static string tagAndExecuteCmd(int machineID, string cmd, CommandResultDetails *details) {

		//redirect output to a file
		string fileName = O_FILE_NAME_PREFIX + Utility::intToString(machineID) + O_FILE_NAME_SUFFIX;
		string filePath = OUTPUT_FILE_BASE_PATH + fileName;

		cmd +=" >> " + filePath; //append sign here because the tag command will take care of stripping out the old contents    

		//add 2>&1 to command to stream errors to the file too
		cmd += " 2>&1";
		//cmd has been totally built now

		tagFile(machineID, filePath, details);

		if(details->returnStatus == 0) {
			executeCmd(cmd, details);
		}

		return filePath;
	}

	static string mergeFileOutputs(string *files, int noOfFiles, CommandResultDetails *details, int stripTags) {
		//add support for strip tags later if required

		//merge file name
		string mergeFileName = "cmdMergedOutput.out";
		string mergeFilePath = OUTPUT_FILE_BASE_PATH + mergeFileName;

		//going to resort to UNIX commands to do this
		string cmdToMerge = "cat ";
		int i = 0;

		for (i =0; i < noOfFiles; i++) {
			cmdToMerge += files[i] + " ";
		}

		cmdToMerge += " > " + mergeFilePath + " 2>&1";
		cout<<endl<<"The merge command is: "<<cmdToMerge<<endl;
		executeCmd(cmdToMerge, details);
		return mergeFilePath;

	}

	static void executeCmd(string cmd, CommandResultDetails *details) {
		//check if a command processor is available
		if(system(NULL)) { 
			details->returnStatus = system(cmd.c_str());
		} else {
			details->returnStatus = -1;
			details->errCode = COMMAND_PROCESSOR_NOT_AVAILABLE;
		}
	}
};

#endif