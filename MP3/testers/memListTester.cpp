#include <iostream>
#include <string>
#include <ctime>
#include "../headers/membershipList.h"

using namespace std;

void createList1(MembershipList *memList) {
	
}

void createList2(MembershipList *memList) {

}

void createList3(MembershipList *memList) {
	/*MembershipDetails entry;
	entry.id = Utility::intToString(2);
	entry.heartbeat = 10;
	entry.localTimestamp = time(0);
	memList->addToList(entry);*/
}

void createList4(MembershipList *memList) {
	/*MembershipDetails entry;
	entry.id = Utility::intToString(2);
	entry.heartbeat = 11;
	entry.localTimestamp = time(0);
	memList->addToList(entry);*/
}


int main() {

	//cout<<MembershipList::getNetworkID("192.16.17.11")<<endl;

	int machine1ID = 1;
	int machine2ID = 2;
	int machine3ID = 3;
	int machine4ID = 4;

	ErrorLog *logger1 = new ErrorLog(machine1ID);
	ErrorLog *logger2 = new ErrorLog(machine2ID);
	ErrorLog *logger3 = new ErrorLog(machine3ID);
	ErrorLog *logger4 = new ErrorLog(machine4ID);

	int errCode = 0;

	MembershipList *memList1 = new MembershipList(machine1ID, "127.0.0.1:0", logger1);
	MembershipList *memList2 = new MembershipList(machine2ID, "127.0.0.2:0", logger2);
	MembershipList *memList3 = new MembershipList(machine3ID, "127.0.0.3:0", logger3);
	MembershipList *memList4 = new MembershipList(machine4ID, "127.0.0.4:0", logger4);


	createList1(memList1);
	createList2(memList2);
	createList3(memList3);
	createList4(memList4);

	memList1->incrementHeartbeat(1, &errCode);
	memList2->incrementHeartbeat(2, &errCode);
	memList3->incrementHeartbeat(3, &errCode);
	memList4->incrementHeartbeat(4, &errCode);

	cout<<"List 1: "<<endl;
	memList1->printMemList();
	cout<<endl;

	cout<<"List 2: "<<endl;
	memList2->printMemList();
	cout<<endl;

	cout<<"List 3: "<<endl;
	memList3->printMemList();
	cout<<endl;

	cout<<"List 4: "<<endl;
	memList4->printMemList();
	cout<<endl;

	memList1->updateMembershipList(memList2, &errCode);
	memList1->updateMembershipList(memList3, &errCode);
	memList1->updateMembershipList(memList4, &errCode);
	memList1->processList(&errCode);
		

	cout<<"Current time: "<<time(0)<<endl;
	cout<<"Modified List 1: "<<endl;
	memList1->printMemList();

	//memList1->writeIPsToFile(&errCode);

	/*memList2->updateMembershipList(memList3, &errCode);
	cout<<"have to send to: "<<endl;
	vector<string> ips;
	memList2->getListOfMachinesToSendTo(0.5, &ips, &errCode);
	for(int i=0; i< ips.size(); i++) {
		cout<<ips.at(i)<<endl;
	}
	*/

	/*
	cout<<"have to send to: "<<endl;
	vector<string> ips;
	memList1->getListOfMachinesToSendTo(0.5, &ips, &errCode);
	for(int i=0; i< ips.size(); i++) {
		cout<<ips.at(i)<<endl;
	}
	 */
	
	/*vector<string> ips;
	memList1->readIPsFromFile(&ips, &errCode);
	cout<<"List of ips from the file is"<<endl;
	for(int i=0; i< ips.size(); i++) {
		cout<<ips.at(i)<<endl;
	}*/

	/*vector<string> ips;
	memList1->getListOfMachinesToSendTo(0.5, &ips, &errCode);

	cout<<"The ips are: "<<endl;
	for(int j =0; j < ips.size(); j++) {
		cout<<ips.at(j)<<endl;
	}*/

	/**
	
	memList2->incrementHeartbeat(1, &errCode);
	 
	memList2->requestRetirement(&errCode);


	sleep(3);
	cout<<"Current time: "<<time(0)<<endl;
	memList1->incrementHeartbeat(1, &errCode);

	memList1->updateMembershipList(memList2, &errCode);
	memList1->updateMembershipList(memList3, &errCode);
	memList1->updateMembershipList(memList4, &errCode);
	memList1->processList(&errCode);
	cout<<"Modified List 1: "<<endl;
	memList1->printMemList();

	sleep(3);
	memList1->incrementHeartbeat(1, &errCode);

	memList1->updateMembershipList(memList2, &errCode);
	memList1->updateMembershipList(memList3, &errCode);
	memList1->updateMembershipList(memList4, &errCode);
	memList1->processList(&errCode);

	cout<<"Current time: "<<time(0)<<endl;
	cout<<"Modified List 1: "<<endl;
	memList1->printMemList();

	sleep(3);
	memList1->incrementHeartbeat(1, &errCode);
	memList1->processList(&errCode);

	cout<<"Current time: "<<time(0)<<endl;
	cout<<"Modified List 1: "<<endl;
	memList1->printMemList();

	*/

	return 0;
}

