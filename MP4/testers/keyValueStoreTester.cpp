#include <iostream>
#include <string>
#include <ctime>
#include "../headers/keyValueStore.h"
#include "../headers/clt.h"

using namespace std;

void printCommands(vector<string> commands) {
	for(int i =0; i < commands.size(); i++) {
		cout<<commands[i]<<endl;
	}
}

int main() {

	int errCode = 0;

	int machine1ID = 1;
	/*int machine2ID = 2;
	int machine3ID = 3;
	int machine4ID = 4;*/

	ErrorLog *logger1 = new ErrorLog(machine1ID);
	/*ErrorLog *logger2 = new ErrorLog(machine2ID);
	ErrorLog *logger3 = new ErrorLog(machine3ID);
	ErrorLog *logger4 = new ErrorLog(machine4ID);
	*/


	Coordinator *coord1 = new Coordinator();
	/*Coordinator *coord2 = new Coordinator();
	Coordinator *coord3 = new Coordinator();
	Coordinator *coord4 = new Coordinator();*/

	KeyValueStore *keyValueStore1 = new KeyValueStore(machine1ID, logger1, coord1);
	
	/*KeyValueStore *keyValueStore2 = new KeyValueStore(machine2ID, logger2, coord2);
	KeyValueStore *keyValueStore3 = new KeyValueStore(machine3ID, logger3, coord3);
	KeyValueStore *keyValueStore4 = new KeyValueStore(machine4ID, logger4, coord4);
	*/


	string cmd = CommandLineTools::showAndHandlePrompt(machine1ID);

	while(cmd != "exit") {
		KeyValueStoreCommand command = CommandLineTools::parseKeyValueStoreCmd(cmd);

		if(command.isValidCommand()) {
			cout<<"Operation is "<<command.getOperation()<<". Key is "<<command.getKey()<< ". Value is "<<command.getValue()<<endl;
			if(command.getOperation() == INSERT_KEY) {
				keyValueStore1->insertKeyValue(command.getKey(), command.getValue(), &errCode);
			} else if(command.getOperation() == DELETE_KEY) {
				keyValueStore1->deleteKey(command.getKey(), &errCode);
			} else if(command.getOperation() == UPDATE_KEY) {
				keyValueStore1->updateKeyValue(command.getKey(), command.getValue(), &errCode);
			} else if(command.getOperation() == LOOKUP_KEY) {
				cout<<keyValueStore1->lookupKey(command.getKey(), &errCode)<<endl;
			} else if(command.getOperation() == SHOW_KVSTORE) {
				keyValueStore1->show(&errCode);
			} else if(command.getOperation() == FORCE_INSERT_KEY) {
				keyValueStore1->forceInsertKeyValue(command.getKey(), command.getValue(), &errCode);
			} else if(command.getOperation() == FORCE_DELETE_KEY) {
				keyValueStore1->forceDeleteKey(command.getKey(), &errCode);
			} else if(command.getOperation() == FORCE_UPDATE_KEY) {
				keyValueStore1->forceUpdateKeyValue(command.getKey(), command.getValue(), &errCode);
			} else if(command.getOperation() == FORCE_LOOKUP_KEY) {
				cout<<keyValueStore1->forceLookupKey(command.getKey(), &errCode)<<endl;
			}
			
		} else {
			cout<<"Malformed command!"<<endl;
		}

		cmd = CommandLineTools::showAndHandlePrompt(machine1ID);
	}

	/*
	keyValueStore1->insertKeyValue(1, "Lorem", &errCode);
	keyValueStore1->insertKeyValue(2, "Ipsum", &errCode);
	keyValueStore1->insertKeyValue(3, "Ipsum", &errCode);
	keyValueStore2->insertKeyValue(1, "Dolor", &errCode);
	keyValueStore2->insertKeyValue(2, "Sit", &errCode);
	keyValueStore3->insertKeyValue(1, "Amet", &errCode);
	
	keyValueStore1->show(&errCode);
	keyValueStore2->show(&errCode);
	keyValueStore3->show(&errCode);
	keyValueStore4->show(&errCode);

    cout<<endl<<"Changing lorem in key 1 to ipsum"<<endl;
	keyValueStore1->updateKeyValue(1, "Ipsum", &errCode);
	cout<<"The value at machine 1 for key 1 is "<<keyValueStore1->lookupKey(1, &errCode)<<endl;

	cout<<endl<<"Deleting key 1"<<endl;
	keyValueStore1->deleteKey(1, &errCode);	

	keyValueStore1->show(&errCode);
	keyValueStore2->show(&errCode);
	keyValueStore3->show(&errCode);
	keyValueStore4->show(&errCode);

	*/
	return 0;
}