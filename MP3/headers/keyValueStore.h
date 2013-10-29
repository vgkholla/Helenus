#ifndef AG_KV_STORE
#define AG_KV_STORE

#include <iostream>
#include <string>
#include <ctime>
#include <vector>

#include <boost/serialization/vector.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/unordered_map.hpp>

#include "logger.h"
#include "errorCodes.h"
#include "utility.h"
#include "fileHandler.h"
#include "clt.h"

using namespace std;

class Value {
	//the actual value
	string value;

	public:
	/**
	 * Constructor
	 */
	Value (string valueString) {
		setValue(valueString);
	}

	/**
	 * [sets a particular value]
	 * @param valueString [the value that has to be set]
	 */
	void setValue(string valueString) {
		value = valueString;
	}

	/**
	 * [gets the string at this value object]
	 * @return [description]
	 */
	string getValue() {
		return value;
	}
};


class KeyValueStore {

	//the hash map to store our keys
	boost::unordered_map<int,Value> keyValueStore;
	//the machine ID that is the owner of this key value store
	int machineID;
	//the logger object
	ErrorLog *logger;

	/**
	 * [checks for any preexisting errors before executing APIs]
	 * @param  errCode      [the error code. should be 0 if no error]
	 * @param  functionName [the name of the function requesting the check]
	 * @return              [status of the error code. SUCCESS if 0]
	 */
	int checkForPreExistingError(int *errCode, const char* functionName) {
		//bail if there is already an error
		if(*errCode > ERROR_IO_LOGIC) {
			string msg = "Unable to execute " + string(functionName) + ". A previous error with error code: " + Utility::intToString(*errCode) + " exists";
			logger->logError(ERROR_ALREADY_EXISTS, msg , errCode);
			*errCode = ERROR_ALREADY_EXISTS;
			return FAILURE;
		}

		return SUCCESS;
	}

	/**
	 * [private lookup function that can be optionally asked to suppress errors]
	 * @param  key           [key to lookup]
	 * @param  errCode       [place tp store error codes if any]
	 * @param  suppressError [if true, doesn't log an errors]
	 * @return               [the iterator in the hash map which points to the key]
	 */
	boost::unordered_map<int, Value>::iterator lookupKey(int key, int *errCode, int suppressError) {
		boost::unordered_map<int, Value>::iterator entry = keyValueStore.end();

		//check for preexisting error
		int status = checkForPreExistingError(errCode, __func__);
		if (status == FAILURE) {
			return entry;
		}

		//see if the key occurs more than once. Shoulf not happen. Comment this out after testing because this call is costly
		/*if(keyValueStore.count(key) > 1) {
			string msg = "Found more than one matching key while trying to lookup! Key is " + Utility::intToString(key);
			logger->logError(TOO_MANY_KEYS, msg, errCode);
		}*/

		//try to look the key up
		entry = keyValueStore.find(key);

		//is the key there?
		if(entry == keyValueStore.end()) {
			//no such key
			if(!suppressError) {
				string msg = "Tried to lookup a key which does not exist. Key is " + Utility::intToString(key);
				logger->logError(NO_SUCH_KEY, msg , errCode);
			}
		}

		return entry;

	}

	public:
	/**
	 * Constructor. Supply ID of machine which will own the key value store and the logger object of the machine
	 */
	KeyValueStore(int ID, ErrorLog *logObject) {
		machineID = ID;
		logger = logObject;
	}

	/**
	 * [inserts a key into the key value store]
	 * @param  key         [the key]
	 * @param  valueString [the value]
	 * @param  errCode     [place to store errors]
	 * @return             [status of insert]
	 */
	int insertKeyValue(int key, string valueString, int *errCode) {
		//check for preexisting error
		int status = checkForPreExistingError(errCode, __func__);
		if (status == FAILURE) {
			return status;
		}
		//make the value object
		Value value = Value(valueString);

		//try to look the key up first
		boost::unordered_map<int, Value>::iterator lookupEntry = lookupKey(key, errCode, 1);

		if(lookupEntry == keyValueStore.end()) { //key does not exist
			//insert
			status = int(keyValueStore.insert(make_pair(key, value)).second);
		} else { //key exists
			string msg = "Tried to insert a key which already exists. Key is " + Utility::intToString(key);
			logger->logError(KEY_EXISTS, msg , errCode);
			*errCode = KEY_EXISTS;
			status = FAILURE;
		}

		//error check
		if(status == FAILURE){
			string msg = "Inserting key failed. Key is " + Utility::intToString(key);
			logger->logError(INSERT_FAILED, msg , errCode);
		}

		return status;
	}

	/**
	 * [deletes the specified key]
	 * @param  key     [the key]
	 * @param  errCode [place to store error code]
	 * @return         [the status of delete]
	 */
	int deleteKey(int key, int *errCode) {
		//check for preexisting error
		int status = checkForPreExistingError(errCode, __func__);
		if (status == FAILURE) {
			return status;
		}

		//delete the key
		int elementsErased = keyValueStore.erase(key);

		if (elementsErased == 0) {//none deleted?
			string msg = "Tried to delete a key which does not exist. Key is " + Utility::intToString(key);
			logger->logError(NO_SUCH_KEY, msg , errCode);
		} else if (elementsErased > 1) {//too many deleted? (should not happen!)
			string msg = "Found more than one matching key and deleted them all! Key is " + Utility::intToString(key);
			logger->logError(TOO_MANY_KEYS, msg, errCode);
		}

		return status;
	}

	/**
	 * [update the value of a key]
	 * @param  key         [the key]
	 * @param  valueString [the new value]
	 * @param  errCode     [place to store error code]
	 * @return             [status of update]
	 */
	int updateKeyValue(int key, string valueString, int *errCode) {
		//check for preexisting error
		int status = checkForPreExistingError(errCode, __func__);
		if (status == FAILURE) {
			return status;
		}

		//make the value object
		Value value = Value(valueString);

		boost::unordered_map<int, Value>::iterator lookupEntry = lookupKey(key, errCode, 1);
		
		if(lookupEntry != keyValueStore.end()) { //key exists
			lookupEntry->second = value;
		} else { //key does not exist
			string msg = "Updating key failed. Check if key exists. If not, use insert. Key is " + Utility::intToString(key);
			logger->logError(UPDATE_FAILED, msg , errCode);
			*errCode = UPDATE_FAILED;
			status = FAILURE;
		}

		/*//delete the key first
		status = deleteKey(key, errCode);
		if(status == FAILURE) {
			string msg = "Updating key failed. Key is " + Utility::intToString(key);
			logger->logError(UPDATE_FAILED, msg , errCode);
		}

		//reinsert with new value
		status = insertKeyValue(key, valueString, errCode);
		if(status == FAILURE) {
			string msg = "Updating key failed. Key is " + Utility::intToString(key);
			logger->logError(UPDATE_FAILED, msg , errCode);
		}*/

		return status;
	}

	/**
	 * [public interface of lookup key. does not allow suppressing of errors]
	 * @param  key     [key to lookup]
	 * @param  errCode [place to store error code]
	 * @return         [the value of key]
	 */
	string lookupKey(int key, int *errCode) {
		string value = "";
		//get the entry
		boost::unordered_map<int, Value>::iterator lookupEntry = lookupKey(key, errCode, 0);
		//if lookup did not fail, extract the value
		if(lookupEntry != keyValueStore.end()) {
			 value = lookupEntry->second.getValue();
		}

		return value;
	}

	/**
	 * [returns all entries in the key value store]
	 * @param  errCode [place to store errors]
	 * @return         [all the entries as one string]
	 */
	string returnAllEntries(int *errCode) {
		string value = "";

		//check for preexisting error
		int status = checkForPreExistingError(errCode, __func__);
		if (status == FAILURE) {
			return value;
		}

		for(boost::unordered_map<int, Value>::iterator it = keyValueStore.begin(); it != keyValueStore.end(); it++) {
			value += Utility::intToString(it->first);
			value += ":";
			value += it->second.getValue();
			value += "\n";
		}

		return value;
	}

	/**
	 * [prints all entries in the key value store]
	 * @param errCode [place to store error code]
	 */
	void show(int *errCode) {
		int status = checkForPreExistingError(errCode, __func__);
		if (status == FAILURE) {
			cout<<"Unable to show entries in the key value store. Check log for details"<<endl;
		}

		string entries = returnAllEntries(errCode);
		if (entries == "") {
			cout<<"No entries in key value store or unable to show entries. Check the log to see if there are any errors"<<endl;
		} else {
			cout<<"The contents of the key value store at machine "<<machineID<<" are: "<<endl;
			cout<<entries<<endl;
		}
	}

};

#endif