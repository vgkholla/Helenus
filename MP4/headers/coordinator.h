#ifndef AG_COORD
#define AG_COORD

#include <iostream>
#include <string>
#include <vector>

//reasons for passing message
#define REASON_JOIN "join"
#define REASON_FAILURE "failure"

//roles
#define ROLE_SUCCESSOR "successor"
#define ROLE_PREDECESSOR "predecessor"

using namespace std;

class Message {
	//a class for passing messages between the membership list and the key value store

	string reason; //the reason for transfer. can be join or failure
	string role; //what is the relation of the event to this machine

	//for join
	int newMemberHash;//if reason is join, the hash of new member
	int newMachineOwnedRangeStart; //the start of the owned range for the new machine
	int deleteKeysRangeStart;//the start of the range for which replicated keys in this machine have to be deleted
	int deleteKeysRangeEnd;//the end of the range for which replicated keys in this machine have to be deleted
	
	//for failure
	int failedMemberHash;//if reason is failure, the hash of the machine that failed

	public:
	Message() {
		reason = "";

		newMemberHash = -1;
		newMachineOwnedRangeStart = -1;

		failedMemberHash = -1;
	}

	//getters
	//general
	string getReason() {
		return reason;
	}

	string getRole() {
		return role;
	}

	//join
	int getNewMemberHash() {
		return newMemberHash;
	}

	int getNewMachineOwnedRangeStart() {
		return newMachineOwnedRangeStart;
	}

	int getNewMachineOwnedRangeEnd() {
		return getNewMemberHash();
	}

	int getDeleteKeysRangeStart() {
		return deleteKeysRangeStart;
	}

	int getDeleteKeysRangeEnd() {
		return deleteKeysRangeEnd;
	}

	//fail
	int getFailedMemberHash() {
		return failedMemberHash;
	}

	//setters
	//general
	void setReason(string reasonString) {
		reason = reasonString;
	}

	void setRole(string roleString) {
		role = roleString;
	}

	//join
	void setNewMemberHash(int hash) {
		newMemberHash = hash;
	}

	void setNewMachineOwnedRangeStart(int hash) {
		newMachineOwnedRangeStart = hash;
	}

	void setDeleteKeysRangeStart(int hash) {
		deleteKeysRangeStart = hash;
	}

	void setDeleteKeysRangeEnd(int hash) {
		deleteKeysRangeEnd = hash;
	}

	//failed
	void setFailedMemberHash(int hash) {
		failedMemberHash = hash;
	}

};


class Coordinator {

	//the list of messages
	vector <Message> messages;
	//lock for operations
	pthread_mutex_t mutexsum;

	public:
	Coordinator() {
		/* Initialize the mutex */
		pthread_mutex_init(&mutexsum, NULL);
	}

	/**
	 * [checking whether any message has to be read]
	 * @return [returns whether a message is available]
	 */
	int hasMessage() {
		return messages.size();
	}

	void pushMessage(Message message) {
		pthread_mutex_lock (&mutexsum);
		messages.push_back(message);
		pthread_mutex_unlock (&mutexsum);
	}

	Message popMessage() {
		pthread_mutex_lock (&mutexsum);
		Message message = messages.front();
		messages.erase(messages.begin());
		pthread_mutex_unlock (&mutexsum);
		return message;
	}
};

#endif
