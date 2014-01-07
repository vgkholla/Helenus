#include <iostream>
#include <string>
#include <ctime>
#include "../headers/membershipList.h"
#include "../headers/Hash.h"

using namespace std;

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

	Coordinator *coord1 = new Coordinator();
	Coordinator *coord2 = new Coordinator();
	Coordinator *coord3 = new Coordinator();
	Coordinator *coord4 = new Coordinator();

	MembershipList *memList1 = new MembershipList(machine1ID, "127.0.0.1:0", logger1, coord1);
	MembershipList *memList2 = new MembershipList(machine2ID, "127.0.0.2:0", logger2, coord2);
	MembershipList *memList3 = new MembershipList(machine3ID, "127.0.0.3:0", logger3, coord3);
	MembershipList *memList4 = new MembershipList(machine4ID, "127.0.0.4:0", logger4, coord4);


	memList1->incrementHeartbeat(1, &errCode);
	memList2->incrementHeartbeat(2, &errCode);
	memList3->incrementHeartbeat(3, &errCode);
	memList4->incrementHeartbeat(4, &errCode);

	/*cout<<"List 1: "<<endl;
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
	cout<<endl;*/

	memList1->updateMembershipList(memList2, &errCode);
	memList1->updateMembershipList(memList3, &errCode);
	memList1->updateMembershipList(memList4, &errCode);

	/*memList2->updateMembershipList(memList4, &errCode);
	memList2->updateMembershipList(memList3, &errCode);

	memList3->updateMembershipList(memList4, &errCode);*/

	memList2->updateMembershipList(memList1, &errCode);
	memList3->updateMembershipList(memList1, &errCode);
	memList4->updateMembershipList(memList1, &errCode);

	//memList1->processList(&errCode);
		



	cout<<endl<<endl<<"Final Details: "<<endl;	
	cout<<"Key to IP map of machine 1 is: "<<endl;
	cout<<memList1->getkeyToIPMapDetails()<<endl;
	int machine1Hash = Hash::calculateNodeHash("127.0.0.1:0");
	cout<<"Machine hash is: "<<machine1Hash<<endl;

	cout<<"Key to IP map of machine 2 is: "<<endl;
	cout<<memList2->getkeyToIPMapDetails()<<endl;
	int machine2Hash = Hash::calculateNodeHash("127.0.0.2:0");
	cout<<"Machine hash is: "<<machine2Hash<<endl;

	cout<<"Key to IP map of machine 3 is: "<<endl;
	cout<<memList3->getkeyToIPMapDetails()<<endl;
	int machine3Hash = Hash::calculateNodeHash("127.0.0.3:0");
	cout<<"Machine hash is: "<<machine3Hash<<endl;

	cout<<"Key to IP map of machine 4 is: "<<endl;
	cout<<memList4->getkeyToIPMapDetails()<<endl;
	int machine4Hash = Hash::calculateNodeHash("127.0.0.4:0");
	cout<<"Machine hash is: "<<machine4Hash<<endl;

	/*cout<<"Machine 1"<<endl;
	cout<<memList1->getIPofFirstReplica()<<endl;
	cout<<memList1->getIPofSecondReplica()<<endl;
	cout<<memList1->getIPofFirstPredecessor()<<endl;
	cout<<memList1->getIPofSecondPredecessor()<<endl;
	

	cout<<"Machine 2"<<endl;
	cout<<memList2->getIPofFirstReplica()<<endl;
	cout<<memList2->getIPofSecondReplica()<<endl;
	cout<<memList2->getIPofFirstPredecessor()<<endl;
	cout<<memList2->getIPofSecondPredecessor()<<endl;

	cout<<"Machine 3"<<endl;
	cout<<memList3->getIPofFirstReplica()<<endl;
	cout<<memList3->getIPofSecondReplica()<<endl;
	cout<<memList3->getIPofFirstPredecessor()<<endl;
	cout<<memList3->getIPofSecondPredecessor()<<endl;

	cout<<"Machine 4"<<endl;
	cout<<memList4->getIPofFirstReplica()<<endl;
	cout<<memList4->getIPofSecondReplica()<<endl;
	cout<<memList4->getIPofFirstPredecessor()<<endl;
	cout<<memList4->getIPofSecondPredecessor()<<endl;*/

	return 0;
}

