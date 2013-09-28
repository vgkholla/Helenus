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
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/iostreams/stream_buffer.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/mpl/inserter.hpp>
#include <boost/iostreams/filtering_stream.hpp>

#include "ConnectionHandler.h"
#include "Timer.h"
//#include "headers/clt.h"

#define BUFLEN 2024
#define SERVER_PORT 45000
using namespace std;
using namespace P2P;

std::stringstream ss; 

class person 
{ 
public: 
  person() 
  { 
  } 

  person(int age) 
    : age_(age) 
  { 
  } 

  int age() const 
  { 
    return age_; 
  } 

private: 
  friend class boost::serialization::access; 

  template <typename Archive> 
  void serialize(Archive &ar, const unsigned int version) 
  { 
    ar & age_; 
  } 

  int age_; 
};

ConnectionHandler::ConnectionHandler(string src,
                                     int machineno,
                                     std::list<string> dest,
                                     int interval):Timer(interval)
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
    names.push_back("alok");
    names.push_back("pallavi");
    names.push_back("ap");

//    pos = src.find(":");
//    address = src.substr(0,pos);
//    port = src.substr(pos+1,src.length() - pos);
    address = src;

    machine_no = machineno;
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
    string recvstr;
    
    try
    {
        while(1)
        {
            if (recvfrom(ptr->sock, recvstr.c_str(), sizeof(person), 0, (struct sockaddr*)&cli_addr, &slen)==-1)
            {
                cout << "Error accepting connection " << strerror(errno) << endl;
                throw string("error");
            }
            ss << recvstr;
            boost::archive::text_iarchive ia(ss);
            person p;
            ia >> p;
            person *pp = &p;
            ss.clear();
  	    std::cout << pp->age() << std::endl; 
        }
    }
    catch(string sockException)
    {
        close(ptr->sock);
        ptr->sock = -1;
        pthread_exit(NULL);
    }
    close(ptr->sock);
    ptr->sock = -1;
    pthread_exit(NULL);


}

void ConnectionHandler::executeCb()
{
    
    struct sockaddr_in serv_addr;
    int sockfd, i, slen=sizeof(serv_addr);
    char buf[BUFLEN];
    string host_name = "127.0.0.1";

    boost::archive::text_oarchive oa(ss); 
    //person p(31); 
    person *p = new person(31);
    oa << *p; 
    std::string mystring(ss.str());

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
        cout << "Error opening socket" << strerror(errno) << std::endl;

    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(45002);
    serv_addr.sin_addr.s_addr = inet_addr(host_name.c_str());

    if (sendto(sockfd, mystring.c_str(), names.size(), 0, (struct sockaddr*)&serv_addr, slen)==-1)
        cout << "Error Sending on socket " << strerror(errno) << std::endl; 

    close(sockfd);
}

#if 0

void* ConnectionHandler::ClientHandler(void* lp)
{
    int host_port;
    string host_name;
    mystruct *ptrc = static_cast<mystruct*>(lp);
    string hostAndPort = ptrc->hostAndPort;
    //ConnectionHandler *ptr1 = (ConnectionHandler*)ptrc->owner;

    struct sockaddr_in my_addr;

    char buffer1[1024];
    char revbuf[SIZE];
    int bytecount1;
    int buffer_len1=0;

    int hsock;
    int * p_int;
    int err;
    pthread_t thread_id=0;
    std::size_t pos;
    cout << "Initializing connection to " << hostAndPort << endl; 
    pos = hostAndPort.find(":");
    host_name = hostAndPort.substr(0,pos);
    host_port = atoi((hostAndPort.substr(pos+1,hostAndPort.length() - pos)).c_str());

    try
    {
        /* Open connection to my peers
           to get back log files
         */
        hsock = socket(AF_INET, SOCK_STREAM, 0);
        if(hsock == -1){
            cout << "Error initializing socket " << strerror(errno) << endl;
            throw string("error");
        }

        p_int = (int*)malloc(sizeof(int));
        *p_int = 1;

        if( (setsockopt(hsock, SOL_SOCKET, SO_REUSEADDR, (char*)p_int, sizeof(int)) == -1 )||
            (setsockopt(hsock, SOL_SOCKET, SO_KEEPALIVE, (char*)p_int, sizeof(int)) == -1 ) ){
            cout << "Error setting socket options " << strerror(errno) << endl;
            free(p_int);
            throw string("error");
        }
        free(p_int);

        my_addr.sin_family = AF_INET ;
        my_addr.sin_port = htons(host_port);

        memset(&(my_addr.sin_zero), 0, 8);
        my_addr.sin_addr.s_addr = inet_addr(host_name.c_str());

        if( connect( hsock, (struct sockaddr*)&my_addr, sizeof(my_addr)) == -1 ){
            if((err = errno) != EINPROGRESS){
                cout << "Error connecting to peer " << hostAndPort << " Error " << strerror(errno) << endl;
                throw string("error");
            }
        }

        buffer_len1 = 1024;

        memset(buffer1, '\0', buffer_len1);
        memcpy(buffer1,"peer@",5);
        strcat(buffer1,ptrc->cmd.c_str());
        buffer1[strlen(buffer1)]='\0';

        /* 
         Identify myself as a peer and
         send the command to be executed 
         on my peer.
         */
        if( (bytecount1=send(hsock, buffer1, strlen(buffer1),0))== -1){
            cout << "Error sending command to peer " << strerror(errno) << endl;
            throw string("error");
        }

        /* 
         Get back the log file from my peer */
    	std::ostringstream oss;
    	string filename;
    	filename = "machine.";
    	filename += host_name;
    	filename += ":";
    	oss << host_port;
    	filename += oss.str();
    	filename += ".log";
    	char* fr_name = filename.c_str();
    	FILE *fr = fopen(fr_name, "a");
    	if(fr == NULL)
        {
    	    cout << "File to receive data could not be opened " << endl;
            throw string("error");
        }
    	else
    	{
            bzero(revbuf, SIZE);
            int fr_block_sz = 0;
            string size;
            int count;
            int filesize;
            int rcv_filesize = 0;
          
            /* Receive the file size I am about
               to receive from my peer
             */
            if((count = recv(hsock, size.c_str(), SIZE, 0))== -1){
                cout << "Error receiving file size from peer " << strerror(errno) << endl;
                throw string("error");
            }
 
            filesize = atoi(size.c_str());
            int write_sz = fwrite(size.c_str(), sizeof(char), count, fr);
            if(write_sz < fr_block_sz)
            {
                cout << "File write failed" << endl;
            }
            rcv_filesize += fr_block_sz;
            bzero(revbuf, SIZE);

            cout << "FILE SIZE RETURNED " << filesize << "SOCKET NO" << hsock << std::endl;

            /*
             Start receiving file from the peer
             */
            while((fr_block_sz = recv(hsock, revbuf, SIZE, 0)) >  0)
            {
	        write_sz = fwrite(revbuf, sizeof(char), fr_block_sz, fr);
                if(write_sz < fr_block_sz)
                {
                    cout << "File write failed" << endl;
                }
            	bzero(revbuf, SIZE);
            	rcv_filesize += fr_block_sz;
            	if (rcv_filesize >= filesize)
            	{
                    break;
            	}
            }
            cout << "FR BLOCK SZ = " << fr_block_sz << endl;
            if(fr_block_sz < 0)
            {
                if (errno == EAGAIN)
                {
              	    printf("recv() timed out.\n");
            	}
            	else
            	{
                    cout << "recv() failed due to errno " << strerror(errno) << endl;
                    throw string("error");
             	}
            }
            cout << "File Transfer complete from peer" << endl;
            fclose(fr);
        }
    }
    catch(string sockException)
    {
        close(hsock);
        hsock = -1;
        pthread_exit(NULL);
    }
    close(hsock);
    hsock = -1;
    pthread_exit(NULL);
}

int ConnectionHandler::sendFile(string filept, void *lp)
{
    struct stat filestat;
    mystruct *ptr = static_cast<mystruct*>(lp);
    ConnectionHandler *ptr1 = (ConnectionHandler*)ptr->owner;

    char* fs_name = filept.c_str();
    char sdbuf[SIZE];
    std::ostringstream oss;
    string filesize;
    int sentsize =0;
    stat(fs_name, &filestat);
    cout << "File size to be sent " << filestat.st_size << endl;
    /*
     Open the file to be sent
     */
    FILE *fs = fopen(fs_name, "r");
    if(fs == NULL)
    {
        cout << "ERROR: File " << fs_name << " not found." << endl;
        return 1;
    }

    bzero(sdbuf, SIZE);
    int fs_block_sz;
    oss << filestat.st_size;
    filesize = oss.str();
    
    /*
     Send file size to the peer or client
     */
    if(send(*ptr->sock, filesize.c_str(), filesize.length(), 0) < 0)
    {
        cout << "ERROR: Failed to send file size" << strerror(errno) << endl;
        return 1;
    }
    /* 
     Send my log file back to the peer or client
     */
    while((fs_block_sz = fread(sdbuf, sizeof(char), SIZE, fs)) > 0)
    {
        if(send(*ptr->sock, sdbuf, fs_block_sz, 0) < 0)
        {
            cout << "ERROR: Failed to send file from peer " << strerror(errno) << endl;
            return 1;
        }
        bzero(sdbuf, SIZE);
        sentsize += fs_block_sz;
        int pct = ((float)sentsize / atoi(filesize.c_str())) * 100;
        std::cout << "\r\033[K" << "Sent size " << (float)sentsize/1000000 << "MB (" << pct << "% Complete)"<< std::flush;
    }
    cout << endl << "File sent" << endl;
    return 0;
}
#endif
ConnectionHandler::~ConnectionHandler()
{
}

