#ifndef AG_MEM_LIST
#define AG_MEM_LIST

#include <iostream>
#include <string>
#include <ctime>
#include <vector>
#include <algorithm>
#include "logger.h"
#include "errorCodes.h"
#include "utility.h"

using namespace std;

class MembershipDetails {

	private:

	public:
	//the network id. different on different incarnations
	string id;
	//each machine increments it own. this serves as proof of "life"
	int heartbeat;
	//the local timestamp when the heartbeat was last updated
	int localTimestamp;

	//flag to indicate suspected failure
	int failed;
	//flag to indicate intention to leave
	int leaving;

	//timestamp when suspected of failure
	int failureTimestamp;

	MembershipDetails() {
		id = "";
		heartbeat = 0;
		localTimestamp = 0;
		
		failed = 0;
		leaving = 0;

		failureTimestamp=0;
	}
};

class MembershipList {

	private:
	ErrorLog *logger;
	vector<MembershipDetails> memList;
	
	//machine id. never changes between successive reincarnations of the machine
	int machineID;
	//network id. changes between successive reincarnations
	string networkID;
	//the time to failure
	int timeToFailure; //in milliseconds
	//the time to cleanup
	int timeToCleanup; //in milliseconds

	/**
	 * [removes a member from the list]
	 * @param entry [the entry to be removed]
	 */
	void removeFromList(MembershipDetails entry) {
		//lookup by network id and delete
		for(int i =0 ; i < memList.size(); i++ ) {
			if(entry.id == memList[i].id) {
				removeFromList(i);
				break;
			}
		}

	}

	/**
	 * [removes a member from the list]
	 * @param index [the index to be removed]
	 */
	void removeFromList(int i) {
		memList.erase(memList.begin() + i);	
	}

	/**
	 * [updates time to failure to scale with number of machines/processes]
	 */
	void updateTimeToFailure() {
		//implement something that takes into account the current number of processes. Or just go static, anything is fine 
		timeToFailure = 1000;
	}

	/**
	 * [updates time to cleanup to scale with number of machines/processors]
	 */
	void updateTimeToCleanup() {
		//implement something that takes into account the current number of processes. Or just go static, anything is fine 
		timeToCleanup = 5000;
	}

	/**
	 * [returns to time to failure in seconds]
	 * @return [TTF in s]
	 */
	int timeToFailureInSeconds() {
		return timeToFailure/1000;
	}

	/**
	 * [returns time to cleanup in seconds]
	 * @return [TTC in seconds]
	 */
	int  timeToCleanupInSeconds() {
		return timeToCleanup/1000;
	}

	/**
	 * [merges the data on two membership details entries]
	 * @param localEntry  [the local entry]
	 * @param remoteEntry [the remote entry]
	 */
	void mergeEntries(MembershipDetails *localEntry, MembershipDetails remoteEntry) {
		if((remoteEntry.heartbeat > localEntry->heartbeat) && localEntry->leaving != 1) { 
			//make sure there is an increment in the heartbeat and that the machine has not put in a request for leaving 
			localEntry->heartbeat = remoteEntry.heartbeat; //update heartbeat
			localEntry->localTimestamp = time(0);//timestamp right now
			localEntry->failed = 0;//reset failure if applicable
			localEntry->leaving = remoteEntry.leaving;//does the guy want to leave now?
		}
	}

	/**
	 * [replace an entry in the membership list. Lookup by entry]
	 * @param oldEntry [the old entry]
	 * @param newEntry [the new entry]
	 */
	void replaceElementInList(MembershipDetails oldEntry, MembershipDetails newEntry) {
		removeFromList(oldEntry);
		addToList(newEntry);
	}

	/**
	 * [replace an entry in the membership list. Lookup by index]
	 * @param i        [the index]
	 * @param newEntry [the new entry]
	 */
	void replaceElementInList(int i, MembershipDetails newEntry) {
		removeFromList(i);
		addToList(newEntry);
	}

	/**
	 * [updates an entry in the membership table]
	 * @param  entryToProcess [the remote entry]
	 * @param  errCode        [space to store error code]
	 * @return                [status of request]
	 */
	int updateEntry(MembershipDetails entryToProcess, int *errCode) {
		//don't continue if there is an error already
		if(*errCode != NO_ERROR) {
			string msg = "Unable to execute updateEntry(). A previous error with error code: " + Utility::intToString(*errCode) + " exists";
			logger->logError(ERROR_ALREADY_EXISTS, msg , errCode);
			return FAILURE;
		}
		int status = SUCCESS;

		int i = 0;
		int matchFound = 0;
		//search through our membership list to see if we have an entry with an id that matches this one. If there is, then merge
		for(i = 0; i < memList.size(); i++) {
			try {
				MembershipDetails *localEntry = &memList.at(i);
				if(localEntry->id == entryToProcess.id){
					matchFound = 1;
					mergeEntries(localEntry, entryToProcess);
					//one match is all we will have, break
					break;
				}
			}
			catch (exception& e){
				//the index is probably not available. Log it
				string msg = "Accessing index in vector failed in updateEntry(). Failed index: " + Utility::intToString(i);
				logger->logError(INDEX_ACCESS_FAILED, msg , errCode);
				

				//whether the logger failed or not, the caller of this function only needs to know that accessing an index failed
				*errCode = INDEX_ACCESS_FAILED;
				status = FAILURE;
				break;
			}
		}

		//if we haven't had an index failure, and the flags for leaving and suspected failure are not set and a we don't have this entry in our list
		if(status != FAILURE && entryToProcess.leaving != 1 && entryToProcess.failed != 1 && matchFound == 0) {
			string msg = "New member in the network with network ID: " + entryToProcess.id;
			logger->logDebug(MEMBER_JOINED, msg, errCode);
			addToList(entryToProcess);
		}

		return status;
	}

	/**
	 * [merges two membership lists entry by entry]
	 * @param  recvList [the received list]
	 * @param  errCode  [space to store error code]
	 * @return          [the status]
	 */
	int mergeList(MembershipList *recvList, int *errCode) {
		//bail if there is already an error in the system
		if(*errCode != NO_ERROR) {
			string msg = "Unable to execute mergeList(). A previous error with error code: " + Utility::intToString(*errCode) + " exists";
			logger->logError(ERROR_ALREADY_EXISTS, msg , errCode);
			return FAILURE;
		}
		int status = SUCCESS;

		//received list is empty, something wrong
		if(recvList->memList.size() == 0) {
			*errCode = EMPTY_LIST;
			return FAILURE;
		}

		int i = 0;
		//go through the received list and match it entry by entry to the local list
		for(i = 0; i < recvList->memList.size(); i++) {
			try {
				MembershipDetails memEntryToProcess = recvList->memList.at(i);
				//update this entry
				status = updateEntry(memEntryToProcess, errCode);
				if(status == FAILURE) {
					break;
				}
			}
			catch (exception& e){
				//the index is probably not available. Log it
				string msg = "Accessing index in vector failed in mergeList(). Failed index: " + Utility::intToString(i) + ". Received list from machine " + Utility::intToString(recvList->machineID);
				logger->logError(INDEX_ACCESS_FAILED, msg , errCode);

				//whether the logger failed or not, the caller of this function only needs to know that accessing an index failed
				*errCode = INDEX_ACCESS_FAILED;
				status = FAILURE;
			}

		}

		return status;
	}

	/**
	 * [processes a single entry]
	 * @param  i               [the index of the entry]
	 * @param  entriesToDelete [an array to store entries which have to be deleted]
	 * @param  errCode         [space for error code]
	 * @return                 [description]
	 */
	int processEntry(int i, vector<MembershipDetails> *entriesToDelete, int *errCode) {
		//bail if there is already an error
		if(*errCode != NO_ERROR) {
			string msg = "Unable to execute processEntry(). A previous error with error code: " + Utility::intToString(*errCode) + " exists";
			logger->logError(ERROR_ALREADY_EXISTS, msg , errCode);
			return FAILURE;
		}
		int status = SUCCESS;

		try {
				MembershipDetails *entryToProcess = &memList.at(i);
				//if we had previously marked this as failed or left and we have advanced cleanup seconds since then, mark for deletion
				if((entryToProcess->failed == 1 || entryToProcess->leaving == 1) && (entryToProcess->localTimestamp + timeToCleanupInSeconds() < time(0))) {
					string msg = "Machine with Network ID: " + entryToProcess->id;
					msg += entryToProcess->failed == 1 ? " has failed " : " has left ";
					msg += " and has been removed from the membership list";

					int debugType = entryToProcess->failed == 1 ? MEMBER_FAILED : MEMBER_LEFT;
					logger->logDebug(debugType, msg, errCode);
					entriesToDelete->push_back(*entryToProcess);
					//removeFromList(*entryToProcess);
				}
				//test the entry for suspected failure
				else if( entryToProcess->failed !=1 && entryToProcess->leaving != 1 && entryToProcess->localTimestamp + timeToFailureInSeconds() < time(0)) {
					entryToProcess->failed = 1;
					entryToProcess->failureTimestamp = time(0);
					string msg = "Suspected machine failure. Network ID: " + entryToProcess->id;
					logger->logDebug(MEMBER_SUSPECTED_FAILED, msg, errCode);
				}

			}
			catch (exception& e){
				//the index is probably not available. Log it
				string msg = "Accessing index in vector failed in processEntry(). Failed index: " + Utility::intToString(i);
				logger->logError(INDEX_ACCESS_FAILED, msg , errCode);
				

				//whether the logger failed or not, the caller of this function only needs to know that accessing an index failed
				*errCode = INDEX_ACCESS_FAILED;
				status = FAILURE;
			}
		return status;
	}

	public:
	
	/**
	 * Constructor which asks for the machineID and the logger object
	 */
	MembershipList(int ID, string netID, ErrorLog *logObject) {
		machineID = ID;
		logger = logObject;
		networkID = netID;

		MembershipDetails entry;
		entry.id = networkID;
		entry.localTimestamp = time(0);
		addToList(entry);

		updateTimeToCleanup();
		updateTimeToFailure();
	}

	//for debugging
	/**
	 * [prints the members in the list]
	 */
	void printMemList() {
		for(int i =0 ; i < memList.size(); i++ ) {
			cout<<"ID: "<< memList[i].id <<" Heartbeat: "<< memList[i].heartbeat<<" Timestamp: "<<memList[i].localTimestamp<<endl;
		}
	}

	/**
	 * [adds a member to the list]
	 * @param entry [the entry to be added]
	 */
	void addToList(MembershipDetails entry) {
		memList.push_back(entry);
	}

	/**
	 * [updates the membership list to reflect joins/leaves/failures/increments when gossip messages are received]
	 * @param  recvList 	[the received list]
	 * @param  errCode 		[space to return error code if any]
	 * @return           	[returns the status of operation - success/faiure]
	 */
	int updateMembershipList(MembershipList *recvList, int *errCode) {
		//bail if already have an error
		if(*errCode != NO_ERROR) {
			string msg = "Unable to execute updateMembershipList(). A previous error with error code: " + Utility::intToString(*errCode) + " exists";
			logger->logError(ERROR_ALREADY_EXISTS, msg , errCode);
			return FAILURE;
		}
		int status = SUCCESS;

		//merge the lists
		status = mergeList(recvList, errCode);
		
		return status;
	}

	/**
	 * [processes own membership list to update flags]
	 * @param  errCode [space to return error code if any]
	 * @return         [status]
	 */
	int processList(int *errCode) {
		//bail if there is already an error
		if(*errCode != NO_ERROR) {
			string msg = "Unable to execute processList(). A previous error with error code: " + Utility::intToString(*errCode) + " exists";
			logger->logError(ERROR_ALREADY_EXISTS, msg , errCode);
			return FAILURE;
		}

		int status = SUCCESS;
		int i =0;

		//this vector holds all entries marked for deletion
		vector<MembershipDetails> entriesToDelete;

		//iterate through the list and process each entry. They may get marked for deletion
		for(i = 0; i < memList.size(); i++){
			status = processEntry(i, &entriesToDelete, errCode);
			if(status == FAILURE) {
				break;
			}
		}

		//delete entries which were marked for deletion
		for(i=0; i < entriesToDelete.size(); i++) {
			try {
				removeFromList(entriesToDelete.at(i));
			}
			catch (exception& e){
				//the index is probably not available. Log it
				string msg = "Accessing index in vector failed in processList() while trying to delete entries. Failed index: " + Utility::intToString(i);
				logger->logError(INDEX_ACCESS_FAILED, msg , errCode);

				//whether the logger failed or not, the caller of this function only needs to know that accessing an index failed
				*errCode = INDEX_ACCESS_FAILED;
				status = FAILURE;
			}
		}

		return status;
	}

	/**
	 * [increments own heartbeat. proof of "life"]
	 * @param  by      [quantity to add to heartbeat]
	 * @param  errCode [space to return error code if any]
	 * @return         [status of request]
	 */
	int incrementHeartbeat(int by, int *errCode) {
		//bail if there is already an error
		if(*errCode != NO_ERROR) {
			string msg = "Unable to execute incrementHeartbeat(). A previous error with error code: " + Utility::intToString(*errCode) + " exists";
			logger->logError(ERROR_ALREADY_EXISTS, msg , errCode);
			return FAILURE;
		}
		int status = SUCCESS;

		int i = 0;
		int found = 0;
		//find yourself and increase your heartbeat
		for(i = 0; i < memList.size(); i++){
			try {
				MembershipDetails *localEntry = &memList.at(i);
				if(networkID.compare(localEntry->id) == 0){
					localEntry->heartbeat += by;
					localEntry->localTimestamp = time(0);//timestamp it too, otherwise you might think you are dying when you process your own list!!
					found = 1;
					break;
				}
			}
			catch (exception& e){
				//the index is probably not available. Log it
				string msg = "Accessing index in vector failed in incrementHeartbeat(). Failed index: " + Utility::intToString(i);
				logger->logError(INDEX_ACCESS_FAILED, msg , errCode);
				
				//whether the logger failed or not, the caller of this function only needs to know that accessing an index failed
				*errCode = INDEX_ACCESS_FAILED;
				status = FAILURE;
				break;
			}
		}

		//we didn't find ourselves!! identity crisis!! NOOOOOOOOOOOOOO!!!!!
		if(status != FAILURE && found == 0) {
			*errCode = SELF_ENTRY_NOT_FOUND;
			status = FAILURE;
		}
		return status;

	}
	
};
#endif