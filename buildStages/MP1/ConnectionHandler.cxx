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

#include "ConnectionHandler.h"
#include "headers/clt.h"

#define SIZE 1024
using namespace std;
using namespace P2P;

ConnectionHandler::ConnectionHandler(string src,
                                     int machineno,
                                     std::list<string> dest)
{
    struct sockaddr_in my_addr;
    mystruct* f = new mystruct;

    int hsock;
    int * p_int ;
    int err;

    socklen_t addr_size = 0;
    int* csock;
    sockaddr_in sadr;
    pthread_t thread_id=0;
    string address;
    string port;
    std::size_t pos;

    pos = src.find(":");
    address = src.substr(0,pos);
    port = src.substr(pos+1,src.length() - pos);

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
        hsock = socket(AF_INET, SOCK_STREAM, 0);
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
        my_addr.sin_port = htons(atoi(port.c_str()));
    
        memset(&(my_addr.sin_zero), 0, 8);
        my_addr.sin_addr.s_addr = inet_addr(address.c_str());;
    
        if( bind( hsock, (sockaddr*)&my_addr, sizeof(my_addr)) == -1 ){
            cout << "Error binding to socket, make sure nothing else is listening on this port " << strerror(errno) << endl;
            throw string("error");
        }
        if(listen( hsock, 10) == -1 ){
            cout << "Error listening on socket " << strerror(errno) << endl;
            throw string("error");
        }

        addr_size = sizeof(sockaddr_in);

        /* Accepting connections from client */
        
        while(true){
            cout << "Waiting for a connection on port " << atoi(port.c_str()) << endl;
            csock = (int*)malloc(sizeof(int));
            if((*csock = accept( hsock, (sockaddr*)&sadr, &addr_size))!= -1){
                f->sock = csock;
                f->owner = this;
                cout << "Received connection from " <<  inet_ntoa(sadr.sin_addr) << endl;
                pthread_create(&thread_id,0,&ConnectionHandler::SocketHandler, (void*)f);
                pthread_detach(thread_id);
            }
            else
            {
                cout << "Error accepting connection " << strerror(errno) << endl;
                throw string("error");
            }
        }
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

    char buffer[1024];
    int buffer_len = 1024;
    string msg;
    string peerOrClient;
    string command;
    std::size_t pos;

    int bytecount;
    int thread_cnt = ptr1->peers.size();
    pthread_t thread_id[thread_cnt];
    pthread_attr_t attr;
    void* status;
    int rc;
    long t;
    string filept;
    string filenames[thread_cnt+1];
    struct stat filestat;
    list<mystruct*> ptrList;

    CommandResultDetails *details = new CommandResultDetails();

    /* Receive packet from the remote machine and find out 
       if it is a peer or a client and the command send 
       from the remote machine
     */
    try
    {
    	memset(buffer, 0, buffer_len);
    	if((bytecount = recv(*ptr->sock, buffer, buffer_len, 0))== -1){
    	    cout << "Error receiving data " << strerror(errno) << endl;
            throw string("error");
    	}
    
    	msg = buffer;
    	pos = msg.find("@");
    	peerOrClient = msg.substr(0,pos);
    	command = msg.substr(pos+1,msg.length() - pos);

    	cout << "Got connection from: " << peerOrClient << " Command received: " << command << endl;

    	if (peerOrClient == "peer") 
    	{
            int retVal;
            command = CommandLineTools::parseGrepCmd(ptr1->machine_no, command);
            details->reset();
            filept = CommandLineTools::tagAndExecuteCmd(ptr1->machine_no, command, details);

            /* Send back file to the peer I got connection from */

            retVal = ptr1->sendFile(filept, (void*)lp);
            if( retVal == 1)
                throw string("error");
            else
            {
                cout << "File sent to the peer" << endl;
                close(*ptr->sock);
            }
    	}
    	else 
    	{
            /* 
               If connection from a client, open 
               connection to my peers to get back 
               log files
             */
            pthread_attr_init(&attr);
            pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
            t = 0;
            for(map<string,string >::const_iterator it = ptr1->peers.begin();
                it != ptr1->peers.end(); ++it)
            {
            	mystruct *ptrToSend = new mystruct;
            	ptrToSend->owner = ptr->owner;
            	ptrToSend->cmd = command;
            	ptrToSend->hostAndPort = it->first;
            	ptrToSend->sock = ptr->sock;
            	it->second = "connected";
            	cout << "Thread created for machine " << it->first << "Host and port is "<<ptrToSend->hostAndPort<<std::endl;
            	pthread_create(&thread_id[t],&attr,&ConnectionHandler::ClientHandler,(void*)ptrToSend );
            	t++;
            	ptrList.push_back(ptrToSend);
            	cout<<"Finished processing: " <<it->first<<endl;
	    }

            /* Check that all peers have finished sending back files */

            pthread_attr_destroy(&attr);
            for(t=0; t< thread_cnt; t++) {
                rc = pthread_join(thread_id[t], &status);
                if (rc) {
                    cout << "error: return code from pthread_join()is " << rc << endl;
                    throw string("error");
            	}
            }
            
            /*
             Prepare to merge files from peers
             to be sent back to the client
             */
            t = 0;
            for(map<string,string >::const_iterator it = ptr1->peers.begin();
                it != ptr1->peers.end(); ++it)
            {
            	if(it->second == "connected")
            	{
                    filenames[t] = "machine.";
                    filenames[t] += it->first;
                    filenames[t] += ".log";
                    t++;
            	}
            }
        
	    details->reset();
            command = CommandLineTools::parseGrepCmd(ptr1->machine_no, command);
            details->reset();
	    filenames[t] = CommandLineTools::tagAndExecuteCmd(ptr1->machine_no, command, details);
            t++;
            details->reset();	
            filept = CommandLineTools::mergeFileOutputs(filenames,t,details,0);
            cout << "File names ------" << filept << " count " << t << std::endl;
 
            /* Sending back merged file to the client */

            int retVal;
            retVal = ptr1->sendFile(filept, (void*)lp);
            if( retVal == 1)
                throw string("error");
            else
            {
                cout << "File " << filept.c_str() << " sent from  Peer ------> Client" << endl;
                close(*ptr->sock);
            }
            string rmcmd = "rm -rf ";
            int i;
            for (i = 0;i < t;i++)
            {
                rmcmd += filenames[i];
                rmcmd += " ";
            } 
            rmcmd += " > /dev/null 2>&1";
            details->reset();
            CommandLineTools::executeCmd(rmcmd, details);
    	}
    }
    catch(string sockException)
    {
        close(*ptr->sock);
        *ptr->sock = -1;
        pthread_exit(NULL);
    }
    pthread_exit(NULL);
}

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

ConnectionHandler::~ConnectionHandler()
{
}

