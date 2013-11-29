#include <sys/un.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <resolv.h>
#include <unistd.h>
#include <pthread.h>
#include <list>
#include <sys/stat.h>
#include <math.h>
#include <fstream>
#include <signal.h>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/iostreams/stream_buffer.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/mpl/inserter.hpp>
#include <boost/iostreams/filtering_stream.hpp>

#include "headers/ConnectionHandler.h"
#include "headers/Timer.h"
#include "headers/membershipList.h"
#include "headers/logger.h"
#include "headers/utility.h"
#include "headers/coordinator.h"

#define BUFLEN 10000
#define SERVER_PORT 45000
#define FILEPATH "/tmp/ag/peers.dump"
#define FRACTION 0.5
//#define MASTER "192.168.159.133"

using namespace std;
using namespace P2P;

//std::stringstream ss; 
//pthread_mutex_t mutexsum;
bool leave;
long int leaveTimeStamp;
static long int sends = 1;
ErrorLog *logger;

ConnectionHandler::ConnectionHandler(string src,
                                     int machineno,
                                     std::list<string> dest,
                                     int sendPercentage,
                                     float interval):Timer(interval)
{
    mystruct* udp = new mystruct;
    mystruct* tcp = new mystruct;
    mystruct* show = new mystruct;

    int udpSock;
    int tcpSock;

    pthread_t thread_id=0;
    string address;
    string port;
    
    //std::size_t pos;
    //struct sockaddr_in my_addr;
    //int err;

    address = src;
    machine_no = machineno;
    sendPct = sendPercentage;

    try
    {
        /* Create my own membership list
           Initialize the logger 
         */
        string netId = MembershipList::getNetworkID(address);
        logger = new ErrorLog(machine_no);
        //MembershipList *memList = new MembershipList(machine_no, netId, logger);
        Coordinator *coord = new Coordinator();
        KeyValueStore *keyValueStore1 = new KeyValueStore(machine_no, logger, coord);
        MembershipList *memList = new MembershipList(machine_no, netId, logger, coord);
        this->setCoordinatorPtr(coord);
        this->setMemPtr(memList);
        this->setKeyValuePtr(keyValueStore1);
        myIP = src;
        leave = false;
        
        /* Initialize signal handler */
        struct sigaction sigAct;
        memset(&sigAct, 0, sizeof(sigAct));
        sigAct.sa_handler = ConnectionHandler::sendLeaveMsg;
        sigaction(SIGTERM, &sigAct, 0);

        /* Opening a udp socket for membershiplist exchange */
        udpSock = Utility::udpSocket(address,SERVER_PORT);
        //if(udpSock == -1)

        /* Opening a tcp socket for key value store operation */
        tcpSock = Utility::tcpSocket(address,SERVER_PORT);
            
        /* Create threads to start listening 
           to updates from other members
         */
        udp->sock = udpSock;
        udp->owner = this;

        tcp->orgsock = tcpSock;
        tcp->owner = this;
        show->owner = this;
        
        pthread_create(&thread_id,0,&ConnectionHandler::UDPSocketHandler, (void*)udp);
        pthread_detach(thread_id);
        pthread_create(&thread_id,0,&ConnectionHandler::TCPSocketHandler, (void*)tcp);
        pthread_detach(thread_id);
        pthread_create(&thread_id,0,&ConnectionHandler::showCommandPrompt, (void*)tcp);
        pthread_detach(thread_id);

        /* Add a timer task to send membership list
           at regular intervals
         */
        this->addTask(this);
    }
    catch(string sockException)
    {
        close(udpSock);
        udpSock = -1;
        exit(-1);
    }
    
}

void* ConnectionHandler::showCommandPrompt(void* lp)
{
    mystruct *ptr = static_cast<mystruct*>(lp);
    ConnectionHandler *ptr1 = (ConnectionHandler*)ptr->owner;

    string cmdToSend;
    int errCode;
    cmdToSend = CommandLineTools::showAndHandlePrompt("1");

    while(cmdToSend != "exit") {
        KeyValueStoreCommand command = CommandLineTools::parseKeyValueStoreCmd(cmdToSend);

        if(command.isValidCommand()) {
            string msg;
            msg = "Key value store entries:\n";
            msg += ptr1->getKeyValuePtr()->returnAllEntries(&errCode);
            msg += "Membership list:\n";
            msg += ptr1->getMemPtr()->getkeyToIPMapDetails();
            cout << msg << endl;
        } else {
            cout<<"Malformed command!"<<endl;
        }

        cmdToSend = CommandLineTools::showAndHandlePrompt("1");
    }
    pthread_exit(NULL);
}

void* ConnectionHandler::TCPSocketHandler(void* lp)
{
    mystruct *ptr = static_cast<mystruct*>(lp);
    socklen_t addr_size = 0;
    int* csock;
    sockaddr_in sadr;
    pthread_t thread_id=0;

    //ConnectionHandler *ptr1 = (ConnectionHandler*)ptr->owner;

    while(true)
    {
        /* accept connection from peers for
         * exchange of key and value pairs
         */
        csock = (int*)malloc(sizeof(int));
        if((*csock = accept(ptr->orgsock, (sockaddr*)&sadr, &addr_size))!= -1)
        {
            mystruct *ptrToSend = new mystruct;
            ptrToSend->owner = ptr->owner;
            //ptrToSend->orgsock = ptr->orgsock;
            ptrToSend->sock = *csock;
            pthread_create(&thread_id,0,&ConnectionHandler::updateKeyValue,(void*)ptrToSend);
            pthread_detach(thread_id);
        }
        else
        {
            cout << "Error accepting connection " << strerror(errno) << endl;
        }
    }
}

void* ConnectionHandler::updateKeyValue(void* lp)
{
    mystruct *ptr = static_cast<mystruct*>(lp);
    ConnectionHandler *ptr1 = (ConnectionHandler*)ptr->owner;
    char buffer[12288];
    int buffer_len = 12288;
    string msg;
    int bytecount;
    string keyToInsert;
    int hash;
    int interval = 1;

    string received = Utility::rcvData(ptr->sock);
    /* Parse and find out the command type */
    KeyValueStoreCommand command = CommandLineTools::parseKeyValueStoreCmd(received);
    /* Find the key */
    keyToInsert = command.getKey();
    /* Calculate the key hash */
    hash = Hash::calculateKeyHash(keyToInsert);
    
    string ip;
    if(command.getOperation() != SHOW_KVSTORE && command.isNormalOperation()) {
        /* Find the IP of the node where the key value pair should reside */
        ip = ptr1->getMemPtr()->getIPToSendToFromKeyHash(hash);
    } else {
        ip = ptr1->myIP;
    }

    cout<<"Operation is "
        << command.getOperation()
        << ". Key is "
        << command.getKey()
        << ". Value is "
        <<command.getValue()
        << ". Key Hash is "
        << hash
        <<endl;


    if(ptr1->myIP == ip || command.isForceOperation())//the second check is superfluous, but keeping it anyway
    {
        /* If IP same as my IP or a force operation
         * Perform the necessary operation 
         * on the local node */

        //if this is not a show and not a force operation, then this machine is the owner of the key
        //For replication, send force operations to replicas
        int replicaExists = 0;
        int consistentReplicas = 1;
        string replicaMsg;
        if(command.getOperation() != SHOW_KVSTORE && command.isNormalOperation()) {
            replicaMsg = ConnectionHandler::sendForceOperations(received, ptr1->getMemPtr(), &replicaExists, &consistentReplicas);
        }

        msg = ConnectionHandler::performOperationLocally(command, ptr1->getKeyValuePtr(), ptr1->getMemPtr());
        if(msg != "Command failed" && replicaExists && (!consistentReplicas || msg != replicaMsg)) {
            msg = "Agreement Failure";
        }
    }    
    else
    {
        do
        {
            /* If the IP is different, connect to the correct node */

            cout << "Key hash higher than my Node Hash.. Connecting to the right node with ip"
                 << ip
                 << endl;
            msg = Utility::tcpConnectSocket(ip,SERVER_PORT,received);
            ip = ptr1->getMemPtr()->getIPToSendToFromKeyHash(hash);
            sleep(2*interval);
            interval++;
        }
        while(msg == "Failed to Connect");
    }
    Utility::sendData(ptr->sock,msg);
    close(ptr->sock);
    ptr->sock = -1;
    pthread_exit(NULL);
}

string ConnectionHandler::sendForceOperations(string command, MembershipList *memList, int *replicaExists, int *consistentReplicas) {
    command = "f" + command;
    
    string replyFromFirstReplica = "";
    string replyFromSecondReplica = "";
    string firstReplicaIP = memList->getIPofFirstReplica();
    if(firstReplicaIP != "") {
        *replicaExists = 1;
        
        replyFromFirstReplica = Utility::tcpConnectSocket(firstReplicaIP,SERVER_PORT,command);

        string secondReplicaIP = memList->getIPofSecondReplica();
        if(secondReplicaIP != "") {
            replyFromSecondReplica = Utility::tcpConnectSocket(secondReplicaIP,SERVER_PORT,command);
        } else {
            replyFromSecondReplica = replyFromFirstReplica;
        }
    }

    if(replyFromFirstReplica != replyFromSecondReplica) {
        *consistentReplicas = 0;
    }

    return replyFromFirstReplica;

}

string ConnectionHandler::performOperationLocally(KeyValueStoreCommand command, KeyValueStore *kvStore, MembershipList *memList) {

        int errCode = 0;
        int status = SUCCESS;
        
        string msg = "";
        string operation = command.getOperation();
        string key = command.getKey();
        string value = command.getValue();
        
        cout << "Correct node found, performing " << command.getOperation() << " locally" << endl;
        if(operation == INSERT_KEY) {//insert
            status = kvStore->insertKeyValue(key, value, &errCode);
        } else if(operation == FORCE_INSERT_KEY ) {//force insert
            status =kvStore->forceInsertKeyValue(key, value, &errCode);
        } else if(operation == DELETE_KEY) {//delete
            status =kvStore->deleteKey(key, &errCode);
        } else if(operation == FORCE_DELETE_KEY) {//force delete
            status =kvStore->forceDeleteKey(key, &errCode);
        } else if(operation == UPDATE_KEY) {//update
            status = kvStore->updateKeyValue(key, value, &errCode);
        } else if(operation == FORCE_UPDATE_KEY) {//force update
            status = kvStore->forceUpdateKeyValue(key, value, &errCode);
        } else if(operation == LOOKUP_KEY) {//lookup
            msg = kvStore->lookupKey(key, &errCode);
        } else if(operation == FORCE_LOOKUP_KEY) {//force lookup
            msg = kvStore->forceLookupKey(key, &errCode);
        } else if(operation == SHOW_KVSTORE) {//show store
            msg = "Key value store entries:\n";
            msg += kvStore->returnAllEntries(&errCode);
            msg += "Membership list:\n";
            msg += memList->getkeyToIPMapDetails();
        }

        if(operation != LOOKUP_KEY && operation != FORCE_LOOKUP_KEY && operation != SHOW_KVSTORE) {
            msg = status == SUCCESS ? "Command succeeded" : "Command failed";
        }
        else {
            msg = Utility::findMovies(msg);
        }

        return msg;
}

void* ConnectionHandler::UDPSocketHandler(void* lp)
{
    mystruct *ptr = static_cast<mystruct*>(lp);

    struct sockaddr_in cli_addr;
    socklen_t slen=sizeof(cli_addr);
    
    int byte_count;
    char buf[1024];
    int errCode = 0;
    
    pthread_t thread_id=0;
    
    /* Initialize the mutex 
    pthread_mutex_init(&mutexsum, NULL);*/

    //ConnectionHandler *ptr1 = (ConnectionHandler*)ptr->owner;
    //int sockfd, i; 
    //string recvstr;
    
    while(1)
    {
        try
        {
            /* receive data from other members */
            memset(buf, 0, 1024);
            byte_count = recvfrom(ptr->sock, buf, 1024, 0, (struct sockaddr*)&cli_addr, &slen);
            if(byte_count == -1)
            {
                string msg = "Failed to receive on socket";
                int errCode = 0;
                logger->logError(SOCKET_ERROR, msg , &errCode);
                //cout << "Error accepting connection " << strerror(errno) << endl;
            }
            /* deserialze the received membership list */
            string recvstr(buf);
            std::stringstream ss1; 
            ss1 << recvstr;
    
            boost::archive::text_iarchive ia(ss1);
            MembershipList recvList;
            ia >> recvList;
            errCode = 0;
            ss1.clear();
    
            MembershipList *list = new MembershipList();
            *list = recvList;
            mystruct *ptrToSend = new mystruct;
            ptrToSend->owner = ptr->owner;
            ptrToSend->sock = ptr->sock;
            ptrToSend->mPtr = list;

            /* Thread to update my membership list
               and add new members */
            pthread_create(&thread_id,0,&ConnectionHandler::updateMembershipList, (void*)ptrToSend);
            pthread_detach(thread_id);
        }
        catch(exception& e)
        {
            string msg = "Failed to de-serialize";
            errCode = 0;
            logger->logError(SERIALIZATION_ERROR, msg , &errCode);
        }
    }
    close(ptr->sock);
    ptr->sock = -1;
    pthread_exit(NULL);
}

void* ConnectionHandler::updateMembershipList(void* lp)
{
    mystruct *ptr = static_cast<mystruct*>(lp);
    ConnectionHandler *ptr1 = (ConnectionHandler*)ptr->owner;
    int errorcode = 0;

    /* Take a lock on the membership list */
    //pthread_mutex_lock (&mutexsum);
    //ptr->mPtr->printMemList();
    ptr1->getMemPtr()->updateMembershipList(ptr->mPtr,&errorcode);
    /* Release the lock after updating the membership list */
    //pthread_mutex_unlock (&mutexsum);

    delete ptr->mPtr;
    delete ptr;
    pthread_exit(NULL);
}

void ConnectionHandler::executeCb()
{
    int errorcode = 0;
    vector<string> memberIPs;
    
    /* Task called after every regular interval
       Send membership list to a fraction of machines in my list
     */

    this->getMemPtr()->getListOfMachinesToSendTo(FRACTION, 
                                                 &memberIPs, 
                                                 &errorcode);

    //cout << "Sending to : " << memberIPs.size() << endl;
    /* Set the leave bit if SIGTERM received */
    if(leave)
    {
        this->getMemPtr()->requestRetirement(&errorcode);
    }
    sendMemberList(memberIPs);
}

void ConnectionHandler::sendMemberList(vector<string> memberIPs)
{
    struct sockaddr_in serv_addr;
    int sockfd, slen=sizeof(serv_addr);
    int errorcode = 0;
    std::stringstream ss;
    
    //char buf[BUFLEN];

    #ifdef TESTING
    int calPct = rand() % 100;
    if(sendPct > 0 && (calPct < sendPct))
    #endif
    
    {
        /* Increment heartbeat */
    	this->getMemPtr()->incrementHeartbeat(1,&errorcode);
        /* Call process list to check for timeouts */
    	if(!leave) {
            this->getMemPtr()->processList(&errorcode);
    	}
        /* Check if there are any new messages in the coordinator */
        Coordinator *coord = this->getCoordinatorPtr();
        while(coord->hasMessage())
        {
            KeyValueStore *kvStore = this->getKeyValuePtr();
            MembershipList *memList = this->getMemPtr();

            Message message = coord->popMessage();
            if(message.getReason() == REASON_JOIN) {
              ConnectionHandler::handleJoinEvent(message, kvStore, memList);
            } else if (message.getReason() == REASON_FAILURE){
                ConnectionHandler::handleFailOrLeaveEvent(message, kvStore, memList);
            } else {
                string msg = "Did not recognize the message reason: '" + message.getReason() + "'";
                logger->logError(UNRECOGNIZED_MESSAGE_REASON, msg, &errorcode);
            }
        }
        /* If time exceeds the time to cleanup exit the process */
    	if(leave && 
           ((time(0) - leaveTimeStamp) > this->getMemPtr()->timeToCleanupInSeconds())) {
           ConnectionHandler::handleLeaveEvent(logger, this->getKeyValuePtr(), this->getMemPtr());
    	}

        try
        {
            /* Serialize the membership list */
            boost::archive::text_oarchive oa(ss);
            oa << *(this->getMemPtr());
            std::string mystring(ss.str());


    	    if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
            {
                string msg = "Failed to open socket";
                int errCode = 0;
                logger->logError(SOCKET_ERROR, msg , &errCode);
                //cout << "Error opening socket" << strerror(errno) << std::endl;
            }
            /* Write current members to a file
               helps to find an initial list of members to talk to 
               if the master restarts
             */
    	    if(sends % 20 == 0 && machine_no == 1)
    	    {
    	        this->getMemPtr()->writeIPsToFile(&errorcode);
    	    }
    	    sends++;

    	    bzero(&serv_addr, sizeof(serv_addr));
    	    serv_addr.sin_family = AF_INET;
    	    serv_addr.sin_port = htons(45000);
            /* send membership list to list of member IPs */
    	    for(int i = 0; i < memberIPs.size(); i++)
    	    {
                //cout << "Sending  Machine " << machine_no << " Sending to IP " << memberIPs.at(i) << endl;
                serv_addr.sin_addr.s_addr = inet_addr(memberIPs.at(i).c_str());
                if (sendto(sockfd, mystring.c_str(), strlen(mystring.c_str()), 0, (struct sockaddr*)&serv_addr, slen)==-1)
                {
                    string msg = "Failed to send on socket";
                    int errCode = 0;
                    logger->logError(SOCKET_ERROR, msg , &errCode);
                }
    	    }
    	    close(sockfd);
        }
        catch(exception& e)
        {
            //Failed to serialize. Log it
            string msg = "Failed to serialize";
            int errCode = 0;
            logger->logError(SERIALIZATION_ERROR, msg , &errCode);
        }
    }
}

void ConnectionHandler::handleJoinEvent(Message message, KeyValueStore *kvStore, MembershipList *memList) {
    string role = message.getRole();
    if(role == ROLE_PREDECESSOR) {
        ConnectionHandler::handleJoinEventAsPredecessor(message, kvStore, memList);
    } else if(role == ROLE_SUCCESSOR){
        ConnectionHandler::handleJoinEventAsSuccessor(message, kvStore, memList);
    } else {
        cout<<"Unrecognized role during join!!"<<endl;
    }
} 

void ConnectionHandler::handleJoinEventAsPredecessor(Message message, KeyValueStore *kvStore, MembershipList *memList) {
    int errCode = 0;
    vector<string> commands = kvStore->getCommandsToReplicateOwnedKeys(&errCode);

    string newMemberIP = memList->getIPFromHash(message.getNewMemberHash());
    cout << "Replicating my owned keys in the new node in the system with IP " 
         << newMemberIP 
         << " and hash " << message.getNewMemberHash() 
         << endl;

    for(int i = 0; i < commands.size() ; i++) {
        Utility::tcpConnectSocket(newMemberIP, SERVER_PORT,commands[i]);
    }

    cout<<"Replicated "<<commands.size()/2<<" owned key(s)"<<endl;

}

void ConnectionHandler::handleJoinEventAsSuccessor(Message message, KeyValueStore *kvStore, MembershipList *memList) {
    int errCode = 0;
    
    int newMachineOwnedRangeStart = message.getNewMachineOwnedRangeStart();
    cout<<"New machine owned range start: " << newMachineOwnedRangeStart<<endl;
    
    int newMachineOwnedRangeEnd = message.getNewMachineOwnedRangeEnd();
    cout<<"New machine owned range end: " << newMachineOwnedRangeEnd<<endl;
    
    vector<string> deleteCommands;
    vector<string> commands = kvStore->getCommandsToTransferOwnedKeysAtJoin(newMachineOwnedRangeStart, newMachineOwnedRangeEnd, &deleteCommands, &errCode);

    string newMemberIP = memList->getIPFromHash(message.getNewMemberHash());
    cout << "Moving some of my keys to the new node in the system with IP " 
         << newMemberIP 
         << " and hash " << message.getNewMemberHash() 
         << endl;
    
    for(int i = 0; i < commands.size() ; i++) {
        Utility::tcpConnectSocket(newMemberIP, SERVER_PORT,commands[i]);
    }

    cout<<"Moved "<<commands.size()<<" owned key(s)"<<endl;

    if(memList->isReplicatedKeysDeletingRequiredForJoin()) {
        string secondReplicaIP = memList->getIPofSecondReplica();
        if(secondReplicaIP != "") {
            cout << "Deleting the same keys in my second replica with IP " 
                 << secondReplicaIP 
                 << endl;
            for(int i = 0; i < deleteCommands.size() ; i++) {
                Utility::tcpConnectSocket(secondReplicaIP, SERVER_PORT,deleteCommands[i]);
            }
        }
    }

    int deleteKeysRangeStart = message.getDeleteKeysRangeStart();
    int deleteKeysRangeEnd = message.getDeleteKeysRangeEnd();

    if(deleteKeysRangeStart != -1) {
        cout<<"Deleting some replicated keys from my own store"<<endl;
        cout<<"Delete keys range start: " << deleteKeysRangeStart<<endl;
        cout<<"Delete keys range end: " << deleteKeysRangeEnd<<endl;
        
        int deleted = kvStore->deleteReplicatedKeys(deleteKeysRangeStart, deleteKeysRangeEnd, &errCode);
        cout<<"Deleted "<<deleted<<" replicated key(s)"<<endl;
    } 
}

void ConnectionHandler::handleFailOrLeaveEvent(Message message, KeyValueStore *kvStore, MembershipList *memList) {
    string role = message.getRole();
    if(role == ROLE_PREDECESSOR) {
        ConnectionHandler::handleFailOrLeaveEventAsPredecessor(message, kvStore, memList);
    } else if(role == ROLE_SUCCESSOR){
        ConnectionHandler::handleFailOrLeaveEventAsSuccessor(message, kvStore, memList);
    } else {
        cout<<"Unrecognized role during fail/leave!!"<<endl;
    }
} 

void ConnectionHandler::handleFailOrLeaveEventAsPredecessor(Message message, KeyValueStore *kvStore, MembershipList *memList) {
    int errCode = 0;
    vector<string> commands = kvStore->getCommandsToReplicateOwnedKeys(&errCode);

    string newSecondReplicaIP = memList->getIPofSecondReplica();
    if(newSecondReplicaIP != "") {
        cout << "Replicating my owned keys in a another node in the system with IP " 
             << newSecondReplicaIP 
             << endl;

        for(int i = 0; i < commands.size() ; i++) {
            Utility::tcpConnectSocket(newSecondReplicaIP, SERVER_PORT,commands[i]);
        }

        cout<<"Replicated "<<commands.size()/2<<" owned key(s)"<<endl;
    }

}

void ConnectionHandler::handleFailOrLeaveEventAsSuccessor(Message message, KeyValueStore *kvStore, MembershipList *memList) {
    int errCode = 0;
    
    int newOwnedKeysRangeStart = message.getNewOwnedKeysRangeStart();
    cout<<"New owned keys range start: " << newOwnedKeysRangeStart<<endl;
    
    int newOwnedKeysRangeEnd = message.getNewOwnedKeysRangeEnd();
    cout<<"New owned keys range end: " << newOwnedKeysRangeEnd<<endl;
    
    vector<string> commands = kvStore->getCommandsToReplicateNewOwnedKeysAtFailOrLeave(
        newOwnedKeysRangeStart, newOwnedKeysRangeEnd, &errCode);

    cout<<"Marked "<<commands.size()/2<<" key(s) as newly owned"<<endl;

    string firstReplicaIP = memList->getIPofFirstReplica();
    if(firstReplicaIP != "") {
    cout << "Replicating new owned keys at node in the system with IP " 
             << firstReplicaIP 
             << endl;
        
        for(int i = 0; i < commands.size() ; i++) {
            Utility::tcpConnectSocket(firstReplicaIP, SERVER_PORT,commands[i]);
        }

        cout<<"Replicated "<<commands.size()/2<<" newly owned key(s)"<<endl;
    }

    string newSecondReplicaIP = memList->getIPofSecondReplica();
    if(newSecondReplicaIP != "") {
    cout << "Replicating new owned keys at node in the system with IP " 
             << newSecondReplicaIP 
             << endl;
        
        for(int i = 0; i < commands.size() ; i++) {
            Utility::tcpConnectSocket(newSecondReplicaIP, SERVER_PORT,commands[i]);
        }

        cout<<"Replicated "<<commands.size()/2<<" newly owned key(s)"<<endl;
    }
}


void ConnectionHandler::handleLeaveEvent(ErrorLog *logger, KeyValueStore *kvStore, MembershipList *memList) {
    int errCode = 0;
    string msg = "Elvis has left the building";
    logger->logDebug(MEMBER_LEFT, msg , &errCode);
    cout << msg << endl;
    exit(0);
}

void ConnectionHandler::sendLeaveMsg(int signal)
{
    leave = true;    
    leaveTimeStamp = time(0);
}

ConnectionHandler::~ConnectionHandler()
{
}
