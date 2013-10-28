#include <iostream>
#include <string>
#include <ctime>
#include "../headers/keyValueStore.h"

using namespace std;


int main() {

	int errCode = 0;

	int machine1ID = 1;
	int machine2ID = 2;
	int machine3ID = 3;
	int machine4ID = 4;

	ErrorLog *logger1 = new ErrorLog(machine1ID);
	ErrorLog *logger2 = new ErrorLog(machine2ID);
	ErrorLog *logger3 = new ErrorLog(machine3ID);
	ErrorLog *logger4 = new ErrorLog(machine4ID);


	KeyValueStore *keyValueStore1 = new KeyValueStore(machine1ID, logger1);
	KeyValueStore *keyValueStore2 = new KeyValueStore(machine2ID, logger2);
	KeyValueStore *keyValueStore3 = new KeyValueStore(machine3ID, logger3);
	KeyValueStore *keyValueStore4 = new KeyValueStore(machine4ID, logger4);

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



	return 0;
}