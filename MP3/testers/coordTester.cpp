#include <iostream>
#include <string>
#include <ctime>
#include "../headers/coordinator.h"

using namespace std;

int main() {

	Coordinator coord;

	if(!coord.hasMessage()) {
		coord.setTransferKeysToMember(1);
		coord.setReason("join");
		coord.setNewMemberHash(1);
	}

	cout<<"Has message: " <<coord.hasMessage()<<endl;
	cout<<"Reason: "<<coord.getReason()<<endl;
	cout<<"New member hash: "<<coord.getNewMemberHash()<<endl;
	return 0;
}