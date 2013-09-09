#include <iostream>
#include "headers/clt.h"
#include "headers/fileHandler.h"
#include "headers/actions.h"

using namespace std;

string executeCommandOnMachine(int machineID, string cmd) {
	//the details object
	CommandResultDetails *details = new CommandResultDetails();

	string outputFilePath = CommandLineTools::tagAndExecuteCmd(machineID, cmd, details);

	/*cout<<endl<<"The output of the command is available at: "<<outputFilePath<<endl;
	cout<<endl<<"******Contents of the output file follow******"<<endl<<endl;

	int errCode = 0; //for holding errors
	int retStatus = 0;
	string output = FileHandler::readFromFile(outputFilePath, &retStatus, &errCode);

	cout<<output<<endl;*/

	return outputFilePath;
}

void testMerge(string cmd) {
	/*****MERGING TESTER*********/

	//execute the command on two machines
	string files[2];
	
	files[0] = executeCommandOnMachine(1, cmd);
	files[1] = executeCommandOnMachine(2, cmd);

	CommandResultDetails *details = new CommandResultDetails();
	CommandLineTools::mergeFileOutputs(files, 2, details, 0);
	
}

void testScp(string cmd) {
	/*********SCP TESTER********/
	string replyTo = "127.0.0.1";
	int errCode;
	int success = Actions::executeAndReturnResultViaScp(2, cmd, replyTo, &errCode);
	if(success == 0) {
		cout<<"The command was executed and the file was sent"<<endl;
	} else {
		cout<<"There was an error executing the command. It returned with error code ["<<errCode<<"]"<<endl;
	}

}

void testStreamOutput(string cmd) {
	int errCode;
	int returnStatus;

	string output = Actions::executeAndReturnResult(2, cmd, &returnStatus, &errCode);
	cout<<output<<endl;

}

int main() {

	//show the prompt
	string cmd = CommandLineTools::showAndHandlePrompt(1);

	//do any string manipulation on command here

	//testMerge(cmd);
	
	//testScp(cmd);
	
	testStreamOutput(cmd);

	return 0;
}