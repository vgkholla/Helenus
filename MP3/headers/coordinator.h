#ifndef AG_COORD
#define AG_COORD

#include <iostream>
#include <string>

//reasons for passing message
#define JOIN "join"

using namespace std;

class Coordinator {

	//coordinator messages between KVStore and MemList
	//for transfer of keys
	int transferKeysToMember;//0 signifies no, 1 signfies that some keys need to be transfered
	string reason; //the reason for transfer. can be join.

	//for join
	int newMemberHash;//if reason is join, the hash of new member;
	int selfHash; //Node's hash

	public:
	Coordinator() {
		transferKeysToMember = 0;
		reason = "";
		newMemberHash = 0;
		selfHash = 0;
	}

	int hasMessage() {
		return transferKeysToMember;//keep adding ors here if there are different coordinator messages
	}

	//getters
	string getReason() {
		return reason;
	}

	int getNewMemberHash() {
		return newMemberHash;
	}

	int getSelfHash() {
		return selfHash;
	}

	//setters
	void setTransferKeysToMember(int status) {
		transferKeysToMember = status;
	}

	void setReason(string reasonString) {
		reason = reasonString;
	}

	void setNewMemberHash(int hash) {
		newMemberHash = hash;
	}

	void setSelfHash(int hash) {
		selfHash = hash;
	}
};

#endif
