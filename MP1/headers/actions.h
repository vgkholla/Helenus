#ifndef AG_ACTIONS
#define AG_ACTIONS

#include <string>
#include <iostream>
#include "clt.h"
#include "errorCodes.h"
#include "fileHandler.h"

using namespace std;

class Actions {

	public:

	/**
	 * [executes the command and returns the output as a string]
	 * @param  machineID    [the machine ID]
	 * @param  cmd          [the command]
	 * @param  returnStatus [return status]
	 * @param  errCode      [error code]
	 * @return              [the output of command]
	 */
	static string executeAndReturnResult(int machineID, string cmd, int *returnStatus, int *errCode) {

		//make the details object
		CommandResultDetails *details = new CommandResultDetails();

		//this is coming from a peer, so just execute the command
		string outputFilePath = CommandLineTools::tagAndExecuteCmd(machineID, cmd, details);
		if(details->returnStatus != SUCCESS) {
			//might want to log an error here later on
			*errCode = COMMAND_FAILED_TO_EXEC_AT_PEER;
			*returnStatus = details->returnStatus;
			return "";
		}
		//command was successfully executed. Output file is available and tagged.
		
		return FileHandler::readFromFile(outputFilePath, returnStatus, errCode);
	}

	/**
	 * Probably not going the scp way because it needs a password
	 */
	/**
	 * [executes a command ans scps the output file]
	 * @param  machineID [the machine ID]
	 * @param  cmd       [the command]
	 * @param  replyTo   [the destination]
	 * @param  errCode   [error code]
	 * @return           [return status]
	 */
	static int executeAndReturnResultViaScp(int machineID, string cmd, string replyTo, int *errCode) {

		*errCode = NO_ERROR;
		//make the details object
		CommandResultDetails *details = new CommandResultDetails();

		//this is coming from a peer, so just execute the command
		string outputFilePath = CommandLineTools::tagAndExecuteCmd(machineID, cmd, details);
		if(details->returnStatus != SUCCESS) {
			//might want to log an error here later on
			*errCode = COMMAND_FAILED_TO_EXEC_AT_PEER;
			return details->returnStatus;
		}
		//command was successfully executed. Output file is available and tagged.
		
		//since we are storing all our files in /tmp, we can send that over and store it in the same path in the peer
		string remoteFilePath = outputFilePath;
		remoteFilePath = "/tmp/scped.out";

		//reset the details object
		details->reset();

		//now scp the file to the peer who asked for the command to be executed
		CommandLineTools::scpFile(outputFilePath, replyTo, remoteFilePath, details);
		if(details->returnStatus != SUCCESS) {
			//might want to log an error here later on
			*errCode = FILE_TRANSFER_FAILED;
			return details->returnStatus;
		}
		//the scp was successful. Let the caller know that the command was executed and the result is now available with the asking peer!

		return SUCCESS;
	}
};

#endif