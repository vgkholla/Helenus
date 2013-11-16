#ifndef AG_COORD
#define AG_COORD

#include <iostream>
#include <string>
#include <vector>

//reasons for passing message
#define REASON_JOIN "join"
#define REASON_FAILURE "failure"

using namespace std;

class Message {
	//a class for passing messages between the membership list and the key value store

	string reason; //the reason for transfer. can be join or failure

	//for join
	int newMemberHash;//if reason is join, the hash of new member
	
	//for failure
	int failedMemberHash;//if reason is failure, the hash of the machine that failed

	int selfHash; //Node's hash

	public:
	Message() {
		reason = "";
		selfHash = -1;

		newMemberHash = -1;
		failedMemberHash = -1;
	}

	//getters
	string getReason() {
		return reason;
	}

	int getSelfHash() {
		return selfHash;
	}

	int getNewMemberHash() {
		return newMemberHash;
	}

	int getFailedMemberHash() {
		return failedMemberHash;
	}

	//setters
	void setReason(string reasonString) {
		reason = reasonString;
	}

	void setSelfHash(int hash) {
		selfHash = hash;
	}

	void setNewMemberHash(int hash) {
		newMemberHash = hash;
	}

	void setFailedMemberHash(int hash) {
		failedMemberHash = hash;
	}

};


class Coordinator {

	vector <Message> messages;

	public:
	Coordinator() {
	}

	/**
	 * [checking whether any message has to be read]
	 * @return [returns whether a message is available]
	 */
	int hasMessage() {
		return messages.size();
	}

	void pushMessage(Message message) {
		messages.push_back(message);
	}

	Message popMessage() {
		Message message = messages.back();
		messages.pop_back();
		return message;
	}
};

#endif
