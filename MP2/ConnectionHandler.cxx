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

#include "ConnectionHandler.h"
#include "Timer.h"
#include "membershipList.h"
#include "logger.h"

#define BUFLEN 10000
#define SERVER_PORT 45000
#define FILEPATH "/tmp/ag/peers.dump"
#define FRACTION 0.5
//#define MASTER "192.168.159.133"

using namespace std;
using namespace P2P;

//std::stringstream ss; 
pthread_mutex_t mutexsum;
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
    struct sockaddr_in my_addr;
    mystruct* f = new mystruct;

    int hsock;
    int * p_int ;
    int err;

    socklen_t addr_size = 0;
    sockaddr_in sadr;
    pthread_t thread_id=0;
    string address;
    string port;
    std::size_t pos;

    address = src;
    machine_no = machineno;
    sendPct = sendPercentage;
    cout << "Machine No. " << machine_no << " Address" << address << " Port " << port << std::endl;

    /* Marking all clients as unconnected */
    for (std::list<string>::iterator it = dest.begin(); it != dest.end(); it++)
        peers[*it] = "unconnected";

    for(std::map<string,string>::const_iterator it = peers.begin();
        it != peers.end(); ++it)
    {
        std::cout << it->first << " " << it->second << "\n";
    }
    try
    {
        string netId = MembershipList::getNetworkID(address);
        logger = new ErrorLog(machine_no);
        MembershipList *memList = new MembershipList(machine_no, netId, logger);
        this->setMemPtr(memList);
        leave = false;
        
        struct sigaction sigAct;
        memset(&sigAct, 0, sizeof(sigAct));
        sigAct.sa_handler = ConnectionHandler::sendLeaveMsg;
        sigaction(SIGTERM, &sigAct, 0);

        /* Opening socket and binding and listening to it */
        hsock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if(hsock == -1){
            cout << "Error opening socket " << strerror(errno) << endl;
            throw string("error");
        }
    
        p_int = (int*)malloc(sizeof(int));
        *p_int = 1;
       
        /* Setting socket options */ 
        if( (setsockopt(hsock, SOL_SOCKET, SO_REUSEADDR, (char*)p_int, sizeof(int)) == -1 )||
            (setsockopt(hsock, SOL_SOCKET, SO_KEEPALIVE, (char*)p_int, sizeof(int)) == -1 ) ){
            cout << "Error setting socket options " << strerror(errno) << endl;
            free(p_int);
            throw string("error");
        }
        free(p_int);

        my_addr.sin_family = AF_INET ;
        my_addr.sin_port = htons(SERVER_PORT);
    
        memset(&(my_addr.sin_zero), 0, 8);
        my_addr.sin_addr.s_addr = inet_addr(address.c_str());;
    
        if( bind( hsock, (sockaddr*)&my_addr, sizeof(my_addr)) == -1 ){
            cout << "Error binding to socket, make sure nothing else is listening on this port " << strerror(errno) << endl;
            throw string("error");
        }

        f->sock = hsock;
        f->owner = this;
        pthread_create(&thread_id,0,&ConnectionHandler::SocketHandler, (void*)f);
        pthread_detach(thread_id);

        this->addTask(this);
    }
    catch(string sockException)
    {
        close(hsock);
        hsock = -1;
        exit(-1);
    }
    
}

void* ConnectionHandler::SocketHandler(void* lp)
{
    mystruct *ptr = static_cast<mystruct*>(lp);
    ConnectionHandler *ptr1 = (ConnectionHandler*)ptr->owner;

    struct sockaddr_in cli_addr;
    int sockfd, i; 
    socklen_t slen=sizeof(cli_addr);
    //string recvstr;
    int byte_count;
    char buf[1024];
    pthread_t thread_id=0;
    pthread_mutex_init(&mutexsum, NULL);
    
    while(1)
    {
        try
        {
            memset(buf, 0, 1024);
            byte_count = recvfrom(ptr->sock, buf, 1024, 0, (struct sockaddr*)&cli_addr, &slen);
            if(byte_count == -1)
            {
                cout << "Error accepting connection " << strerror(errno) << endl;
            }
    
            string recvstr(buf);
            std::stringstream ss1; 
            ss1 << recvstr;
    
            boost::archive::text_iarchive ia(ss1);
            MembershipList recvList;
            ia >> recvList;
            int errorcode = 0;
            ss1.clear();
    
            MembershipList *list = new MembershipList();
            *list = recvList;
            mystruct *ptrToSend = new mystruct;
            ptrToSend->owner = ptr->owner;
            ptrToSend->sock = ptr->sock;
            ptrToSend->mPtr = list;

            pthread_create(&thread_id,0,&ConnectionHandler::updateMembershipList, (void*)ptrToSend);
            pthread_detach(thread_id);
        }
        catch(exception& e)
        {
            string msg = "Failed to de-serialize";
            int errCode = 0;
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

    pthread_mutex_lock (&mutexsum);
    //ptr->mPtr->printMemList();
    ptr1->getMemPtr()->updateMembershipList(ptr->mPtr,&errorcode);
    pthread_mutex_unlock (&mutexsum);

    delete ptr->mPtr;
    delete ptr;
    pthread_exit(NULL);
}

void ConnectionHandler::executeCb()
{
    int errorcode = 0;
    vector<string> memberIPs;

    this->getMemPtr()->getListOfMachinesToSendTo(FRACTION, 
                                                 &memberIPs, 
                                                 &errorcode);

    cout << "Sending to : " << memberIPs.size() << endl;
    
    if(leave)
    {
        this->getMemPtr()->requestRetirement(&errorcode);
    }
    sendMemberList(memberIPs);
}

void ConnectionHandler::sendMemberList(vector<string> memberIPs)
{
    struct sockaddr_in serv_addr;
    int sockfd, i, slen=sizeof(serv_addr);
    char buf[BUFLEN];
    int errorcode = 0;
    std::stringstream ss;
#ifdef TESTING
    int calPct = rand() % 100;
    if(sendPct > 0 && (calPct < sendPct))
#endif
    {
    	this->getMemPtr()->incrementHeartbeat(1,&errorcode);

    	if(!leave) {
            this->getMemPtr()->processList(&errorcode);
    	}
    
    	if(leave && 
           ((time(0) - leaveTimeStamp) > this->getMemPtr()->timeToCleanupInSeconds()))
    	{
            cout << "Clean up Time expired, Exiting now " << endl;
            exit(0);
    	}

        try
        {

            boost::archive::text_oarchive oa(ss);
            oa << *(this->getMemPtr());
            std::string mystring(ss.str());


    	    if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
                cout << "Error opening socket" << strerror(errno) << std::endl;

    	    if(sends % 20 == 0 && machine_no == 1)
    	    {
    	        this->getMemPtr()->writeIPsToFile(&errorcode);
    	    }
    	    sends++;

    	    bzero(&serv_addr, sizeof(serv_addr));
    	    serv_addr.sin_family = AF_INET;
    	    serv_addr.sin_port = htons(45000);
  
    	    for(int i = 0; i < memberIPs.size(); i++)
    	    {
                cout << "Sending  Machine " << machine_no << " Sending to IP " << memberIPs.at(i) << endl;
                serv_addr.sin_addr.s_addr = inet_addr(memberIPs.at(i).c_str());
                if (sendto(sockfd, mystring.c_str(), strlen(mystring.c_str()), 0, (struct sockaddr*)&serv_addr, slen)==-1)
                    cout << "Error Sending on socket " << strerror(errno) << std::endl;
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

void ConnectionHandler::sendLeaveMsg(int signal)
{
    leave = true;    
    leaveTimeStamp = time(0);
}

ConnectionHandler::~ConnectionHandler()
{
}
