#ifndef AG_MEM_LIST
#define AG_MEM_LIST

#include <iostream>
#include <string>
#include <ctime>
#include <vector>
#include <map>
#include <algorithm>

#include <boost/serialization/vector.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include "logger.h"
#include "errorCodes.h"
#include "utility.h"
#include "fileHandler.h"
#include "clt.h"
#include "Hash.h"
#include "keyValueStore.h"
#include "coordinator.h"

using namespace std;

#define IP_TIMESTAMP_SEPERATOR ':'
#define IP_LIST_ENTRY_SEPERATOR '|'
#define MASTER_IP "127.0.0.1"

class MembershipDetails {

	private:

 	friend class boost::serialization::access;

    template <typename Archive>
    /**
     * [serializes this calss. required for boost serialization]
     * @param ar      [the archive name]
     * @param version [versioning]
     */
    void serialize(Archive &ar, const unsigned int version) {
        ar & id;
        ar & heartbeat;
        ar & localTimestamp;
        ar & failed;
        ar & leaving;
        ar & failureTimestamp;
        ar & nodeHash;
    }

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
    
    //Hashed value of the node
    int nodeHash;

	MembershipDetails() {
		id = "";
		heartbeat = 0;
		localTimestamp = 0;
		
		failed = 0;
		leaving = 0;

		failureTimestamp=0;
        nodeHash = 0;
	}
};

class MembershipList {

	private:
	//the logger
	ErrorLog *logger;
	//the memberhip list
	vector<MembershipDetails> memList;
	//the key to IP map
	map<int,string> keyToIPMap;
	//the coordinator
	Coordinator *coordinator;
	//hash of the machine that owns this membership list
	int selfHash;
	//lock for operations
	pthread_mutex_t mutexsum;
	
	//machine id. never changes between successive reincarnations of the machine
	int machineID;
	//network id. changes between successive reincarnations
	string networkID;
	//the time to failure
	int timeToFailure; //in milliseconds
	//the time to cleanup
	int timeToCleanup; //in milliseconds
        
    friend class boost::serialization::access; 

    template <typename Archive> 
    /**
     * [function to serialize this class (for boost)]
     * @param ar      [the archive]
     * @param version [versioning]
     */
    void serialize(Archive &ar, const unsigned int version) {
        ar & machineID;
        ar & networkID;
        ar & memList;
    }


    int getNumberOfEntriesInList() {
    	return memList.size();
    }

	/**
	 * [removes a member from the list]
	 * @param entry [the entry to be removed]
	 */
	void removeFromList(MembershipDetails entry) {
		if(keyToIPMap.find(entry.nodeHash) != keyToIPMap.end()) {
			keyToIPMap.erase(entry.nodeHash); 
		}
		//lookup by network id and delete
		for(int i =0 ; i < memList.size(); i++ ) {
			if(entry.id == memList[i].id) {
				removeFromList(i);
                keyToIPMap.erase(entry.nodeHash);
				break;
			}
		}
		printkeyToIPMap();
                       

	}

	void printkeyToIPMap() {
		cout<<"Key to IP map is: "<<endl;
		cout<<getkeyToIPMapDetails();
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
		timeToFailure = 4000;
	}

	/**
	 * [updates time to cleanup to scale with number of machines/processors]
	 */
	void updateTimeToCleanup() {
		//implement something that takes into account the current number of processes. Or just go static, anything is fine 
		timeToCleanup = 4000;
	}

	/**
	 * [merges the data on two membership details entries]
	 * @param localEntry  [the local entry]
	 * @param remoteEntry [the remote entry]
	 */
	void mergeEntries(MembershipDetails *localEntry, MembershipDetails remoteEntry) {
		if((remoteEntry.heartbeat > localEntry->heartbeat) && localEntry->leaving != 1 && remoteEntry.failed != 1) { 
			//make sure there is an increment in the heartbeat and that the machine has not put in a request for leaving 
			localEntry->heartbeat = remoteEntry.heartbeat; //update heartbeat
			localEntry->localTimestamp = time(0);//timestamp right now
			localEntry->failed = 0;//reset failure if applicable
			localEntry->leaving = remoteEntry.leaving;//does the guy want to leave now?
			if(remoteEntry.leaving == 1) {
				if(keyToIPMap.find(remoteEntry.nodeHash) != keyToIPMap.end()) {
					keyToIPMap.erase(remoteEntry.nodeHash); 
				}
			}
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
			entryToProcess.localTimestamp = time(0);
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
				if((entryToProcess->failed == 1 && (entryToProcess->failureTimestamp + timeToCleanupInSeconds() < time(0)))
					|| (entryToProcess->leaving == 1 && (entryToProcess->localTimestamp + timeToCleanupInSeconds() < time(0)))
						) {
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
					//cout<<"Current time: "<<time(0)<<endl;
					//printMemList();
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

	/**
	 * [gets the name of the file which has the list of known ips in the membership list]
	 * @return [the file name]
	 */
	string getIPsBackupFileName() {
		return "ips." + Utility::intToString(machineID) + ".bak";
	}

	/**
	 * [gets the ip from a network id by stripping out the part before the seperator]
	 * @param  id [the network id]
	 * @return    [the ip]
	 */
	string getIPFromNetworkID(string id) {
		return id.substr(0, id.find(IP_TIMESTAMP_SEPERATOR));
	}

	/**
	 * [reads a list of ips from the backup file]
	 * @param  ips     [space to store the ips]
	 * @param  errCode [space to store error code]
	 * @return         [status, success or failure]
	 */
	int readIPsFromFile(vector<string> *ips, int *errCode) {
		//bail if there is already an error
		if(*errCode != NO_ERROR) {
			string msg = "Unable to execute writeIPsToFile(). A previous error with error code: " + Utility::intToString(*errCode) + " exists";
			logger->logError(ERROR_ALREADY_EXISTS, msg , errCode);
			return FAILURE;
		}

		int status = SUCCESS;
		
		//get the filename and path
		string fileName = getIPsBackupFileName();
		string filePath = "./" + fileName;
		string ip = "";

		string ipsString = FileHandler::readFromFile(filePath, &status, errCode);
		if(status != FAILURE) {
			int i = 0;
			while(1) {
				ip = extractIPByIndex(ipsString, i);
				if(ip == "") {
					break;
				}

				ips->push_back(ip);
				i++;
			}
		}

		return status;
	}

	/**
	 * [extracts the ip at a given index in the backup file list (each entry is seperated by a '|')]
	 * @param  ips   [the string of ips]
	 * @param  index [the index of the ip we want]
	 * @return       [the ip desired]
	 */
	string extractIPByIndex(string ips, int index) {
		string ip = "";
		size_t pos = string::npos;
		int i = 0;

		while (i <= index && (pos = ips.find(IP_LIST_ENTRY_SEPERATOR)) != string::npos) {
			if( i == index) {
				ip = ips.substr(0, pos);
			}

			ips.replace(0, pos + 1, "");
			i++;
		}
		
		return ip;
	}

	/**
	 * [from a given string, extracts the next ip and erases that ip from the string]
	 * @param  ips [the string of ips]
	 * @return     [the ip desired]
	 */
	//NOT USED!!
	string extractAndEraseNextIP(string *ips) {
		string ip = "";
		int pos = ips->find(IP_LIST_ENTRY_SEPERATOR);
		if(pos != string::npos) {
			ip = ips->substr(0, pos);
			ips->replace(0, pos + 1, "");
		}
		
		return ip;

	}

	/**
	 * [checks if the newly joined node is a predecessor. If it is, a message has to pushed to the coordinator]
	 * @param newNodeHash [the hash of the new node]
	 */
    void handleJoin(int newNodeHash) {
    	//there should be more than one machine and the new machine should be the predecessor
    	if(keyToIPMap.size() > 1 && isPredecessor(newNodeHash)) {
    		//build the message
        	Message message;
        	message.setReason(REASON_JOIN);
        	message.setSelfHash(selfHash);
        	message.setNewMemberHash(newNodeHash);

        	int newMachineOwnedRangeStart = -1;
        	if(keyToIPMap.size() == 2) {//special case of only two machines. Each machine is the predecessor and successor of each other
        		newMachineOwnedRangeStart = selfHash;
        	} else {//in this case the predecessor of the new machine is the second predecessor of this machine
        		newMachineOwnedRangeStart = getHashOfSecondPredecessor(selfHash);
        	}
        	message.setNewMachineOwnedRangeStart(newMachineOwnedRangeStart);
        	
        	//store it in the coordinator
        	coordinator->pushMessage(message);
    	}	
    }

    /** Postion checking functions **/

    map<int,string>::iterator getMachineAtDistance(int distance) {
    	map<int,string>::iterator requiredMachine = keyToIPMap.end();
   		
   		int increment = 1;
    	if(distance < 0) {
    		distance = -distance;
    		increment = 0;
    	}

    	if(distance + 1 <= keyToIPMap.size()) {
	    	map<int,string>::iterator selfEntry = keyToIPMap.find(selfHash);
	    	requiredMachine = selfEntry;
	    	while(distance > 0) {
	    		if(increment) {
	    			requiredMachine++;
		    		if(requiredMachine == keyToIPMap.end()) {
		    			requiredMachine = keyToIPMap.begin();
		    		}
	    		} else {
	    			if(requiredMachine == keyToIPMap.begin()) {
	    				requiredMachine = keyToIPMap.end();
	    			}
	    			requiredMachine--;
	    		}
	    		distance--;
	    	}
	    }

    	return requiredMachine;
    }

    string getIPAtDistance(int distance) {
    	string ip = "";
   		map<int,string>::iterator requiredMachine = getMachineAtDistance(distance);
   		if(requiredMachine != keyToIPMap.end()) {
   			ip = requiredMachine->second;
   		}
   		return ip;
    }

    int getHashAtDistance(int distance) {
   		int hash = -1;
   		map<int,string>::iterator requiredMachine = getMachineAtDistance(distance);
   		if(requiredMachine != keyToIPMap.end()) {
   			hash = requiredMachine->first;
   		}
   		return hash;
    }

    int isFirstReplica(int nodeHash) {
    	return nodeHash == getHashAtDistance(1);
    }

	int isSecondReplica(int nodeHash) {
    	return nodeHash == getHashAtDistance(2);
    }    

    int isPredecessor(int nodeHash) {
    	return nodeHash == getHashAtDistance(-1);
    }

    int getHashOfSecondPredecessor(int nodeHash) {
    	return getHashAtDistance(-2);
    }

	public:
	
	/**
	 * Constructor which asks for the machineID and the logger object
	 */
	MembershipList(int ID, string netID, ErrorLog *logObject, Coordinator *coord) {
		machineID = ID;
		logger = logObject;
		networkID = netID;
		coordinator = coord;
		
		/* Initialize the mutex */
		pthread_mutex_init(&mutexsum, NULL);

		MembershipDetails entry;
		entry.id = networkID;
		entry.localTimestamp = time(0);
		entry.nodeHash = Hash::calculateNodeHash(networkID);
		
		selfHash = entry.nodeHash;
        
		addToList(entry);

		updateTimeToCleanup();
		updateTimeToFailure();

		cout << "My node has value " << selfHash << endl;
	}
    
    /**
     * constructor for creating an empty object to hold the another list that is expected to come in
     */
    MembershipList() {};

	//for debugging
	/**
	 * [prints the members in the list]
	 */
	void printMemList() {
		for(int i =0 ; i < memList.size(); i++ ) {
			cout<<"ID: "<< memList[i].id <<" Heartbeat: "<< memList[i].heartbeat<<" Timestamp: "<<memList[i].localTimestamp<<" Failed: "<<memList[i].failed<<" Leaving: "<<memList[i].leaving<<" Failure Timestamp: "<<memList[i].failureTimestamp<<" Node Hash ID: "<<memList[i].nodeHash<<endl;
		}
	}

	/**
	 * [adds a member to the list]
	 * @param entry [the entry to be added]
	 */
	void addToList(MembershipDetails entry) {
		memList.push_back(entry);
        keyToIPMap.insert(std::pair<int,string>(entry.nodeHash,getIPFromNetworkID(entry.id))); 
        printkeyToIPMap();
        
        if(keyToIPMap.size() > 1) {
        	handleJoin(entry.nodeHash);
    	}
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

		pthread_mutex_lock (&mutexsum);
		//merge the lists
		status = mergeList(recvList, errCode);
		pthread_mutex_unlock (&mutexsum);

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

		pthread_mutex_lock (&mutexsum);
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
		pthread_mutex_unlock (&mutexsum);

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

		pthread_mutex_lock (&mutexsum);
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
		pthread_mutex_unlock (&mutexsum);
		//we didn't find ourselves!! identity crisis!! NOOOOOOOOOOOOOO!!!!!
		if(status != FAILURE && found == 0) {
			*errCode = SELF_ENTRY_NOT_FOUND;
			status = FAILURE;
		}
		return status;

	}

    string getIPToSendToFromKeyHash(int keyHash) {
        map<int,string>::iterator it;
        /* Find the IP of the machine with hash higher than the key to be stored */
        for (it=keyToIPMap.begin(); it!=keyToIPMap.end(); ++it){
            if(it->first > keyHash){
                 return it->second;
            }
        }
        /* if no such node found, the correct node must be the first node */
        it = keyToIPMap.begin();
        return it->second;
    }

    /* Return the IP from a node hash */
    string getIPFromHash(int key) {
        return keyToIPMap.find(key)->second;
    }


    /*** position public interfaces **/

    string getIPofFirstPredecessor() {
    	return getIPAtDistance(-1);
    }

    string getIPofSuccessor() {
    	return getIPAtDistance(1);
    }

    string getIPofFirstReplica() {
    	return getIPAtDistance(1);
    }

    string getIPofSecondReplica() {
    	return getIPAtDistance(2);
    }

    /*** leave helper functions ***/
    int isReplicatedKeysSendingRequiredForLeave() {
    	return keyToIPMap.size() > 3;
    }


    /*** position functions end****/

	/**
	 * [used by a machine which wants to volunatarily leave the group]
	 * @param  errCode [space to store error code]
	 * @return         [status of request]
	 */
	int requestRetirement(int *errCode) {
		//bail if there is already an error
		if(*errCode != NO_ERROR) {
			string msg = "Unable to execute requestRetirement(). A previous error with error code: " + Utility::intToString(*errCode) + " exists";
			logger->logError(ERROR_ALREADY_EXISTS, msg , errCode);
			return FAILURE;
		}
		int status = SUCCESS;

		pthread_mutex_lock (&mutexsum);
		int i = 0;
		int found = 0;
		//find yourself and put in a request for retirement. 
		//********make sure to call incrementHeartbeat before requesting to retire. Otherwise the others might treat you as failed rather than retired******
		for(i = 0; i < memList.size(); i++){
			try {
				MembershipDetails *localEntry = &memList.at(i);
				if(networkID.compare(localEntry->id) == 0){
					localEntry->leaving = 1; //requests retirement
					found = 1;
					break;
				}
			}
			catch (exception& e){
				//the index is probably not available. Log it
				string msg = "Accessing index in vector failed in requestRetirement(). Failed index: " + Utility::intToString(i);
				logger->logError(INDEX_ACCESS_FAILED, msg , errCode);
				
				//whether the logger failed or not, the caller of this function only needs to know that accessing an index failed
				*errCode = INDEX_ACCESS_FAILED;
				status = FAILURE;
				break;
			}
		}
		pthread_mutex_unlock (&mutexsum);
		//we didn't find ourselves!! identity crisis!! NOOOOOOOOOOOOOO!!!!!
		if(status != FAILURE && found == 0) {
			*errCode = SELF_ENTRY_NOT_FOUND;
			status = FAILURE;
		}
		return status;
	}

	/**
	 * [sends a list of ips that the message needs to be sent to]
	 * @param  fraction [the fraction of machines the message should be sent to]
	 * @param  ips      [space for returning the ips]
	 * @param  errCode  [space for error code if any]
	 * @return          [status]
	 */
	int getListOfMachinesToSendTo(float fraction, vector<string> *ips, int *errCode) {
		//bail if there is already an error
		if(*errCode != NO_ERROR) {
			string msg = "Unable to execute getListOfMachinesToSendTo(). A previous error with error code: " + Utility::intToString(*errCode) + " exists";
			logger->logError(ERROR_ALREADY_EXISTS, msg , errCode);
			return FAILURE;
		}

		int status = SUCCESS;

		srand(time(0));

		int noOfEntries = getNumberOfEntriesInList();
		//fraction of machines we want to send to
		int toChoose = max(1, (int)(noOfEntries * fraction));
		
		//choose a random starting index
		int start = rand() % noOfEntries;
		int end = start + toChoose;
		int entryIndex = start;
		int startSeenOnce = 0;

		for (int i = start; i < end; i++) {
			try {
					if(entryIndex % noOfEntries == start) {
						if(startSeenOnce) {
							break;
						}
						startSeenOnce = 1;
					}

					MembershipDetails entry = memList.at(entryIndex % noOfEntries);
					entryIndex++;
					
					
					if(entry.id == networkID || entry.failed == 1 || entry.leaving == 1) {
						i--;
						continue;
					}

					ips->push_back(getIPFromNetworkID(entry.id));
			}
			catch (exception& e){
				//the index is probably not available. Log it
				string msg = "Accessing index in vector failed in getListOfMachinesToSendTo(). Failed index: " + Utility::intToString(i);
				msg += ". The number of entries in the table is: ";
				msg += Utility::intToString(noOfEntries);
				msg += ". Number of entries to be chosen was: ";
				msg += Utility::intToString(toChoose);
				msg += " The start and end were: ";
				msg += Utility::intToString(start);
				msg += " and ";
				msg += Utility::intToString(end);
				logger->logError(INDEX_ACCESS_FAILED, msg , errCode);
				
				//whether the logger failed or not, the caller of this function only needs to know that accessing an index failed
				*errCode = INDEX_ACCESS_FAILED;
				status = FAILURE;
				break;
			}
		}

		if(ips->size() == 0){
			//there are no ips to send to. This opens us up to two possibilities, master and non master
			if(machineID == 1) {//master
				//read a list of ips from the save file.
				//if the save file has no entries, then this is the first incarnation of the master 
				status = readIPsFromFile(ips, errCode);
			}
			else {//not master. so we are probably trying to join. inform the master
				ips->push_back(MASTER_IP);
			}

		}

		return status;
	}

	int writeIPsToFile(int *errCode) {
		//bail if there is already an error
		if(*errCode != NO_ERROR) {
			string msg = "Unable to execute writeIPsToFile(). A previous error with error code: " + Utility::intToString(*errCode) + " exists";
			logger->logError(ERROR_ALREADY_EXISTS, msg , errCode);
			return FAILURE;
		}

		int status = SUCCESS;

		//get the filename and path
		string fileName = getIPsBackupFileName();
		string filePath = "./" + fileName;
		string ips = "";

		//iterate through the list and pick up the ips
		for(int i = 0; i < memList.size(); i++) {
			MembershipDetails entry = memList.at(i);

			if(entry.id != networkID && entry.failed != 1 && entry.leaving != 1) {
				ips += getIPFromNetworkID(entry.id);
				ips += IP_LIST_ENTRY_SEPERATOR;
			}
		}

		//remove the old file
		string removeCmd = "rm -rf " + filePath;
		CommandResultDetails *details = new CommandResultDetails();
		CommandLineTools::executeCmd(removeCmd, details);
		status = details->returnStatus;

		if(status != FAILURE) {
			status = FileHandler::writeToFile(filePath, ips, errCode);
		}

		delete details;
		return status;

	}

	string getkeyToIPMapDetails() {
		string mapString = "";
		for(map<int, string>:: iterator it = keyToIPMap.begin(); it != keyToIPMap.end(); it++) {
			mapString += it->second;
			mapString += "\n";
		}

		return mapString;
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

	//--------------------------STATIC FUNCTIONS------------------------------//
	
	/**
	 * [gets the network ID. Will differ between reincarnations]
	 * @param  ip [the ip address]
	 * @return    [the network ID]
	 */
	static string getNetworkID(string ip) {
		return ip + ":" + Utility::intToString(time(0));
	}

	
};
#endif
