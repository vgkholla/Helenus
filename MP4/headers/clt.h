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
#include "logger.h"

using namespace std;

//error codes go here
#define COMMAND_PROCESSOR_NOT_AVAILABLE 1

//other constants
#define PROMPT ">>"
#define O_FILE_NAME_PREFIX "cmd."
#define O_FILE_NAME_SUFFIX ".out"
#define OUTPUT_FILE_BASE_PATH "/tmp/ag/"
#define GREP_KEY_SPEC "--key"
#define GREP_KEY_LENGTH 5
#define GREP_VAL_SPEC "--value"
#define GREP_VAL_LENGTH 7
#define END_ID "$"
#define START_ID "^"

//key value store commands
//normal commands
#define INSERT_KEY "insert"
#define DELETE_KEY "delete"
#define UPDATE_KEY "update"
#define LOOKUP_KEY "lookup"

//force commands (for replication)
#define FORCE_INSERT_KEY "finsert"
#define FORCE_DELETE_KEY "fdelete"
#define FORCE_UPDATE_KEY "fupdate"
#define FORCE_LOOKUP_KEY "flookup"

//other commands
#define SHOW_KVSTORE "show"

//consistency levels
#define CONSISTENCY_ONE 'o'
#define CONSISTENCY_QUORUM 'q'
#define CONSISTENCY_ALL 'a'



class KeyValueStoreCommand{

	//class to store the command for easy access
	//
	string operation;
	string key;
	string value;
	char consistencyLevel;

	public:

	/**
	 * constructor
	 */
	KeyValueStoreCommand(string op, string k, string valueString, char consistency) {
		operation = op;
		key = k;
		value = valueString;
		consistencyLevel = consistency;
	}

	string getOperation() {
		return operation;
	}

	string getKey() {
		return key;
	}

	string getValue() {
		return value;
	}

	char getConsistencyLevel() {
		return consistencyLevel;
	}

	int isForceOperation() {
		return operation == FORCE_DELETE_KEY || operation == FORCE_INSERT_KEY || operation == FORCE_UPDATE_KEY || operation == FORCE_LOOKUP_KEY;
	}

	int isNormalOperation() {
		return !isForceOperation();
	}

	int isValidCommand() {
		int commandValid = 0;
		
		//for insert and update, we need a valid key and valid value
		if(operation == INSERT_KEY || operation == UPDATE_KEY || operation == FORCE_INSERT_KEY || operation == FORCE_UPDATE_KEY) {
			commandValid = key != "" && value != ""; 
		//for lookup and delete, we need a valid key
		} else if(operation == LOOKUP_KEY || operation == DELETE_KEY || operation == FORCE_LOOKUP_KEY || operation == FORCE_DELETE_KEY) {
			commandValid = key != "";
		//nothing required for show
		} else if(operation == SHOW_KVSTORE) {
			commandValid = 1;
		}

		return commandValid;
	}
};


class CommandResultDetails{

	public:

	int returnStatus;
	int errCode;

	CommandResultDetails() {
		reset();
	}

	void reset() {
		returnStatus = FAILURE;
		errCode = NO_ERROR;
	}
};

class CommandLineTools {

	
	/**
	 * [tags the file to say which machine output came from]
	 * @param  machineID 
	 * @param  filePath
	 * @param  details Command result details like return status and error codes
	 * @return command exit status
	 */
	static void tagFile(int machineID, string filePath, CommandResultDetails *details) {
		//first build the tag command for tagging the file
		string tagCmd = "echo **Output of command from machine [" + Utility::intToString(machineID) + "]** > " + filePath + " 2>/dev/null";
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
		return showAndHandlePrompt(Utility::intToString(machineID));
	}

	/**
	 * [Shows the prompt and takes the input]
	 * @param  prompt [the prompt]
	 * @return [the cmd entered by the user]
	 */
	static string showAndHandlePrompt(string prompt) {
		string cmd;

		//can customize prompt based on machine id if we want
		cout<<prompt<<PROMPT;

		//take user input
		getline(cin, cmd);

		//find a way of santizing command. A white list perhaps?

		//return the command
		return cmd;
	}

	/**
	 * [processes the key part of the grep]
	 * @param  cmd [the command]
	 * @return     [processed return]
	 */
	static string processKeyGrep(string cmd) {
		//find the identifier
		size_t identifierPos = cmd.find(GREP_KEY_SPEC);
		
		if(identifierPos != string::npos) {
			//rip it out
			cmd = cmd.substr(0, identifierPos) + cmd.substr(identifierPos + GREP_KEY_LENGTH + 1, cmd.length());
		}

		//cout<<"Commmand without custom identifiers is: "<<cmd<<endl;
		//process it
		string suffix = "";
		//find if we have the $
		identifierPos = cmd.find(END_ID);
		if(identifierPos != string::npos && !Utility::isEscaped(cmd, identifierPos)) {
			//rip the $ out if its not escaped
			cmd = cmd.substr(0, identifierPos) + cmd.substr(identifierPos + 1, cmd.length());
			suffix = ":";
		} else {
			suffix = ".*:";
		}

		return cmd + suffix;
	}

	/**
	 * [Process the value part of the grep]
	 * @param  cmd [the command]
	 * @return     [the processed command]
	 */
	static string processValueGrep(string cmd) {
		//find the identifier
		size_t identifierPos = cmd.find(GREP_VAL_SPEC);
		
		//rip it out
		cmd = cmd.substr(0, identifierPos) + cmd.substr(identifierPos + GREP_VAL_LENGTH + 1, cmd.length());
	
		//cout<<"Commmand without custom identifiers is: "<<cmd<<endl;
		//process it
		string prefix = "";
		//find if we have the ^
		identifierPos = cmd.find(START_ID);
		if(identifierPos != string::npos && !Utility::isEscaped(cmd, identifierPos) ) {
			//rip the ^ out if its not escaped
			cmd = cmd.substr(0, identifierPos) + cmd.substr(identifierPos + 1, cmd.length());
			prefix = ":";
		} else {
			prefix = ":.*";
		}

		//find the last space
		size_t lastSpace = cmd.find_last_of(' ');
		//insert prefix just before the search text starts
		cmd = cmd.substr(0, lastSpace + 1) + prefix + cmd.substr(lastSpace + 1, cmd.length());

		return cmd;
	}

	/**
	 * [rips out the quotes]
	 * @param  str [the string]
	 * @return     [string without quotes]
	 */
	static string ripQuotes(string str) {
		//cout<<"Position of quote: " << str.find('\"') <<endl;
		//successively search for double quotes and rip them out
		size_t pos = str.find('\"');
		while(  pos != string::npos ) {
			//if(!Utility::isEscaped(str, pos)) {
				if(pos + 1 < str.length()) {
					str = str.substr(0, pos) + str.substr(pos + 1, str.length());
				} else {
					str = str.substr(0, pos);
				}
			//}
			pos = str.find('\"');
		}
		//cout<<"Position of quote: " << str.find('\'') <<endl;
		//successively search for quotes and rip them out
		pos = str.find('\'');
		while(pos != string::npos ) {
			//if(!Utility::isEscaped(str, pos)) {
				if(pos + 1 < str.length()) {
					str = str.substr(0, pos) + str.substr(pos + 1, str.length());
				} else {
					str = str.substr(0, pos);
				}
			//}
			pos = str.find('\'');
		}

		return str;
	}

	/**
	 * [add quotes to the commands last word]
	 * @param  str [the string]
	 * @return     [string with quotes on the last word]
	 */
	static string addQuotes(string str) {
		size_t pos = str.find_last_of(" ");
		str = str.substr(0, pos + 1) + '\"' + str.substr(pos + 1, str.length()) + '\"'; 
		return str;
	}

	/**
	 * [processes the command based on whether it has key or value]
	 * @param  cmd [the command]
	 * @return     [the processed command]
	 */
	static string processKeyOrValue(string cmd) {
		//trim the cmd
		cmd = Utility::trimTrailingSpaces(cmd);
		//rip out the quotes
		cmd = ripQuotes(cmd);

		//cout<<"Trimmed and ripped command is: "<<cmd<<endl;

		//find if the value identifier is there
		size_t identifierPos = cmd.find(GREP_VAL_SPEC);
		if(identifierPos != string::npos) {
			//there is value identifier
			return addQuotes(processValueGrep(cmd));
		}
		//else its a key
		return addQuotes(processKeyGrep(cmd));
	
	}
	
	/**
	 * [processes the command]
	 * @param  cmd [the command]
	 * @return     [processed output]
	 */
	static string processKeyAndValue(string cmd) {
		//see if we have a pipe
		size_t pipePos = cmd.find('|');

		//if we dont, process and return
		if(pipePos == string::npos) {
			//cout<<"Pipe not found"<<endl;
			return processKeyOrValue(cmd);
		}

		//process the first part
		string firstPart = processKeyOrValue(cmd.substr(0, pipePos));
		//cout<<"First Part is: "<<firstPart<<endl;
		//process the second part
		string secondPart = processKeyOrValue(cmd.substr(pipePos + 1, cmd.length()));
		//cout<<"Second Part is: "<<secondPart<<endl;
		//send final command
		return firstPart + " |" + secondPart;

	}

	/**
	 * [parses the grep command and gives a file name to grep from]
	 * @param  machineID [the machine ID]
	 * @param  cmd       [the command]
	 * @return           [the concatenated command]
	 */
	static string parseGrepCmd(int machineID, string cmd) {
		//if not grep return
		if(cmd.find("grep") == string::npos) {
			return cmd;
		}
		//cout<<"Command before processing: "<<cmd<<endl;
		cmd = processKeyAndValue(cmd);
		//cout<<"Returned Command is: "<<cmd<<endl;

		//the log file path
		string logFilePath = ErrorLog::getLogPath(machineID);

		//build the command
		string fullCmd = "cat " + logFilePath + " | " + cmd;

		return fullCmd;
		
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

		//add 2>/dev/null to command to ignore errors
		cmd += " 2>/dev/null";
		//cmd has been totally built now

		//tag the file
		tagFile(machineID, filePath, details);

		//if tagging succeeded, execute command
		if(details->returnStatus == SUCCESS) {
			executeCmd(cmd, details);
		}

		return filePath;
	}

	/**
	 * [merges output from files]
	 * @param  files the file paths
	 * @param  noOfFiles the number of files
	 * @param  details the command result details object
	 * @param  stripTags optional to stripTags. 1 if you want tags stripped (support no added yet)
	 * @return
	 */
	static string mergeFileOutputs(string *files, int noOfFiles, CommandResultDetails *details, int stripTags) {
		//add support for strip tags later if required

		//merge file name
		string mergeFileName = "cmdMergedOutput.out";
		string mergeFilePath = OUTPUT_FILE_BASE_PATH + mergeFileName;

		//going to resort to UNIX commands to do this
		string cmdToMerge = "cat abc.txt ";
		int i = 0;

		for (i =0; i < noOfFiles; i++) {
			cmdToMerge += files[i] + " ";
		}

		cmdToMerge += " > " + mergeFilePath + " 2>/dev/null";
		//cout<<endl<<"The merge command is: "<<cmdToMerge<<endl;
		executeCmd(cmdToMerge, details);
		return mergeFilePath;
	}

	/**
	 * [executes given command]
	 * @param cmd     [the command to be executed]
	 * @param details [return details]
	 */
	static void executeCmd(string cmd, CommandResultDetails *details) {
		//check if a command processor is available
		if(system(NULL)) { 
			details->returnStatus = system(cmd.c_str());
		} else {
			details->returnStatus = FAILURE;
			details->errCode = COMMAND_PROCESSOR_NOT_AVAILABLE;
		}
	}

	/**
	 * [scps a given file to the destination]
	 * @param localFilePath  [path of file locally]
	 * @param destination    [destination]
	 * @param remoteFilePath [path at destination]
	 * @param details        [return details]
	 */
	//probably not going to use this since it requires a password
	static void scpFile(string localFilePath, string destination, string remoteFilePath, CommandResultDetails *details) {

		//make the scp command
		string scpCmd = "scp " + localFilePath + " " + destination + ":" + remoteFilePath + " > /dev/null 2>&1";

		cout<<scpCmd<<endl;
		//execute the command
		executeCmd(scpCmd, details);
	}

	/*****************************KEY VALUE COMMAND PARSER******************************/

	/**
	 * [checks if an operation is valid]
	 * @param  operation [the operation]
	 * @return           [validity of operation]
	 */
	static int isValidOperation(string operation) {
		return operation == INSERT_KEY || operation == LOOKUP_KEY || operation == UPDATE_KEY || operation == DELETE_KEY ||
				operation == FORCE_INSERT_KEY || operation == FORCE_LOOKUP_KEY || operation == FORCE_UPDATE_KEY || operation == FORCE_DELETE_KEY;
	}

	/**
	 * [parses the command string and creates an object which stores the operation, key and value]
	 * @param  commandString [the command]
	 * @return               [an object with command details]
	 */
	static KeyValueStoreCommand parseKeyValueStoreCmd(string commandString) {
		/* command will be of the form operation(key,value)*/

		int startPos = 0;
		char consistencyLevel = CONSISTENCY_ALL;
		
		//find the first '-''
		size_t firstDashPos = commandString.find_first_of("-");
		//if the client has specified a consistency level, it will be at exactly position 1 (string starts from 0)
		if(firstDashPos == 1) {
			//client has specified a consistency level. Check it is one of the acceptable levels
			if(commandString.at(0) == CONSISTENCY_ONE || commandString.at(0) == CONSISTENCY_QUORUM) { //all is the default anyway. no need to set
				consistencyLevel = commandString.at(0);
			}
			startPos = 2; //update starting position for rest of the commands
		}

		string operation = "";

		//find the first bracket. Operation is just before the first bracket
		size_t firstBracketPos = commandString.find_first_of("(");
		if(firstBracketPos == string::npos) {
			//if there is no first bracket, then the command has to be show, otherwise bail and report malformed command
			if(commandString == SHOW_KVSTORE) {
				operation = SHOW_KVSTORE;
			}
			return KeyValueStoreCommand(operation, "", "", consistencyLevel);
		}

		operation = commandString.substr(startPos, firstBracketPos - startPos);

		//extract and check operation for validity
		if(!isValidOperation(operation)) {
			return KeyValueStoreCommand("", "", "", consistencyLevel);
		}

		//get the first comma position. Key is just after first bracket and just before first comma
		size_t firstCommaPos = commandString.find_first_of(",");
		if(firstCommaPos == string::npos) {
			//if no comma, then the operation has to be lookup or delete, otherwise bail and report malformed command
			if(operation == INSERT_KEY || operation == UPDATE_KEY || operation == FORCE_INSERT_KEY || operation == FORCE_UPDATE_KEY) {
				return KeyValueStoreCommand("", "", "", consistencyLevel);
			} else {
				firstCommaPos = commandString.length() - 1;
			}
		}

		//extract the key
		string key = commandString.substr(firstBracketPos + 1, firstCommaPos - firstBracketPos - 1);

		string value;
		//find the last bracket. value is between first comma and last close bracket
		if(operation == INSERT_KEY || operation == UPDATE_KEY || operation == FORCE_INSERT_KEY || operation == FORCE_UPDATE_KEY) {
			size_t lastBracketPos = commandString.find_last_of(")");
			//if no last bracket, bail and report malformed command
			if(lastBracketPos == string::npos) {
				return KeyValueStoreCommand("", "", "", consistencyLevel);
			}

			//extract the value
			value = commandString.substr(firstCommaPos + 1, lastBracketPos - firstCommaPos - 1);
		} else {
			value = "";
		}

		//return an object with the operation, key and value for easy access
		return KeyValueStoreCommand(operation, key, value, consistencyLevel);
	}


	/***********************************************************************************/

	static string getForceCommand(string commandString) {
		//find the first '-''
		size_t firstDashPos = commandString.find_first_of("-");
		//if the client has specified a consistency level, it will be at exactly position 1 (string starts from 0)
		if(firstDashPos == 1) {
			//client has specified a consistency level. Strip it
			commandString = commandString.substr(2, commandString.length() - 2);
		}

		return "f" + commandString;
	}

};

#endif
