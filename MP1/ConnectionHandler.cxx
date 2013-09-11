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
#include "headers/clt.h"

#include "ConnectionHandler.h"
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
    for (std::list<string>::iterator it = dest.begin(); it != dest.end(); it++)
        peers[*it] = "unconnected";

    for(std::map<string,string>::const_iterator it = peers.begin();
        it != peers.end(); ++it)
    {
        std::cout << it->first << " " << it->second << "\n";
    }

    hsock = socket(AF_INET, SOCK_STREAM, 0);
    if(hsock == -1){
        printf("Error initializing socket %d\n", errno);
        goto FINISH;
    }
    
    p_int = (int*)malloc(sizeof(int));
    *p_int = 1;
        
    if( (setsockopt(hsock, SOL_SOCKET, SO_REUSEADDR, (char*)p_int, sizeof(int)) == -1 )||
        (setsockopt(hsock, SOL_SOCKET, SO_KEEPALIVE, (char*)p_int, sizeof(int)) == -1 ) ){
        printf("Error setting options %d\n", errno);
        free(p_int);
        goto FINISH;
    }
    free(p_int);

    my_addr.sin_family = AF_INET ;
    my_addr.sin_port = htons(atoi(port.c_str()));
    
    memset(&(my_addr.sin_zero), 0, 8);
    my_addr.sin_addr.s_addr = inet_addr(address.c_str());;
    
    if( bind( hsock, (sockaddr*)&my_addr, sizeof(my_addr)) == -1 ){
        fprintf(stderr,"Error binding to socket, make sure nothing else is listening on this port %d\n",errno);
        goto FINISH;
    }
    if(listen( hsock, 10) == -1 ){
        fprintf(stderr, "Error listening %d\n",errno);
        goto FINISH;
    }

    addr_size = sizeof(sockaddr_in);
        
    while(true){
        printf("waiting for a connection on port %d\n",atoi(port.c_str()));
        csock = (int*)malloc(sizeof(int));
        if((*csock = accept( hsock, (sockaddr*)&sadr, &addr_size))!= -1){
            f->sock = csock;
            f->owner = this;
            printf("---------------------\nReceived connection from %s at socket %d and %d address %p\n",inet_ntoa(sadr.sin_addr),*csock,*f->sock,&f);
            pthread_create(&thread_id,0,&ConnectionHandler::SocketHandler, (void*)f);
            pthread_detach(thread_id);
        }
        else{
            fprintf(stderr, "Error accepting %s\n", strerror(errno));
        }
    }
    
FINISH:
;
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

    CommandResultDetails *details = new CommandResultDetails();

    memset(buffer, 0, buffer_len);
    if((bytecount = recv(*ptr->sock, buffer, buffer_len, 0))== -1){
        fprintf(stderr, "Error receiving data %s\n", strerror(errno));
        goto FINISH;
    }
    
    msg = buffer;
    pos = msg.find("@");
    peerOrClient = msg.substr(0,pos);
    command = msg.substr(pos+1,msg.length() - pos);

    cout << "peer or client " << peerOrClient << "Command " << command << std::endl;
    printf("Received bytes %d\nReceived string \"%s\"\n", bytecount, buffer);

    if (peerOrClient == "peer") 
    {
        strcat(buffer, " SERVER ECHO TIAGI");
        printf("\n got Connection from a peer, sending back data");
        filept = CommandLineTools::tagAndExecuteCmd(ptr1->machine_no, command, details);
        char* fs_name = filept.c_str();
        char sdbuf[512];
        FILE *fs = fopen(fs_name, "r");
        if(fs == NULL)
        {
            printf("ERROR: File %s not found.\n", fs_name);
            exit(1);
        }

        bzero(sdbuf, 512);
        int fs_block_sz;
        while((fs_block_sz = fread(sdbuf, sizeof(char), 512, fs)) > 0)
        {
            if(send(*ptr->sock, sdbuf, fs_block_sz, 0) < 0)
            {
                fprintf(stderr, "ERROR: Failed to send file %s. (errno = %d)\n", fs_name, errno);
                break;
            }
            bzero(sdbuf, 512);
        }
        printf("Ok File %s from Client was Sent!\n", fs_name);
    }
    else 
    {
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
        for(t=0; t < thread_cnt; t++)
        {
            printf("\nOpening connectionn to peers socket fd ");
            ptr->cmd = command;
            pthread_create(&thread_id[t],&attr,&ConnectionHandler::ClientHandler,(void*)ptr );
        }
        pthread_attr_destroy(&attr);
        for(t=0; t< thread_cnt; t++) {
            rc = pthread_join(thread_id[t], &status);
            if (rc) {
                printf("ERROR; return code from pthread_join()is %d\n", rc);
                exit(-1);
            }
        }
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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
	filenames[t] = CommandLineTools::tagAndExecuteCmd(ptr1->machine_no, command, details);
        t++;
        details->reset();	
        filept = CommandLineTools::mergeFileOutputs(filenames,t,details,0);
        cout << "File names ------" << filept << " count " << t << std::endl;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        char* fs_name = filept.c_str();
        char sdbuf[512];
        FILE *fs = fopen(fs_name, "r");
        if(fs == NULL)
        {
            printf("ERROR: File %s not found.\n", fs_name);
            exit(1);
        }

        bzero(sdbuf, 512);
        int fs_block_sz;
        while((fs_block_sz = fread(sdbuf, sizeof(char), 512, fs)) > 0)
        {
            if(send(*ptr->sock, sdbuf, fs_block_sz, 0) < 0)
            {
                fprintf(stderr, "ERROR: Failed to send file %s. (errno = %d)\n", fs_name, errno);
                break;
            }
            bzero(sdbuf, 512);
        }
        printf("Ok File %s from Peer ------> Client was Sent!\n", fs_name);

    }

    pthread_exit(NULL);

FINISH:
    //free(csock);
    return 0;
}

void* ConnectionHandler::ClientHandler(void* lp)
{
    int host_port;
    string host_name;
    mystruct *ptr = static_cast<mystruct*>(lp);
    ConnectionHandler *ptr1 = (ConnectionHandler*)ptr->owner;

    struct sockaddr_in my_addr;

    char buffer1[1024];
    char revbuf[512];
    int bytecount1;
    int buffer_len1=0;

    int hsock;
    int * p_int;
    int err;
    pthread_t thread_id=0;
    std::size_t pos;

    for(map<string,string >::const_iterator it = ptr1->peers.begin();
        it != ptr1->peers.end(); ++it)
    {
        if(it->second == "unconnected")
        {
            cout << "AIEEEEEEE host " << it->first << std::endl;
            pos = it->first.find(":");
            host_name = it->first.substr(0,pos);
            host_port = atoi((it->first.substr(pos+1,it->first.length() - pos)).c_str());
            it->second = "connected";
            break;
        }
    }

    hsock = socket(AF_INET, SOCK_STREAM, 0);
    if(hsock == -1){
        printf("Error initializing socket %d\n",errno);
        //goto FINISH;
    }

    p_int = (int*)malloc(sizeof(int));
    *p_int = 1;

    if( (setsockopt(hsock, SOL_SOCKET, SO_REUSEADDR, (char*)p_int, sizeof(int)) == -1 )||
        (setsockopt(hsock, SOL_SOCKET, SO_KEEPALIVE, (char*)p_int, sizeof(int)) == -1 ) ){
        printf("Error setting options %d\n",errno);
        free(p_int);
        //goto FINISH;
    }
    free(p_int);

    my_addr.sin_family = AF_INET ;
    my_addr.sin_port = htons(host_port);

    memset(&(my_addr.sin_zero), 0, 8);
    my_addr.sin_addr.s_addr = inet_addr(host_name.c_str());

    if( connect( hsock, (struct sockaddr*)&my_addr, sizeof(my_addr)) == -1 ){
        if((err = errno) != EINPROGRESS){
            fprintf(stderr, "Error connecting socket %d\n", errno);
            //goto FINISH;
        }
    }

    buffer_len1 = 1024;

    memset(buffer1, '\0', buffer_len1);

    memcpy(buffer1,"peer@",5);
    strcat(buffer1,ptr->cmd.c_str());
    buffer1[strlen(buffer1)]='\0';

    if( (bytecount1=send(hsock, buffer1, strlen(buffer1),0))== -1){
        fprintf(stderr, "Error sending data %d\n", errno);
        //goto FINISH;
    }

//////////////////////////////////////////////////////////////////////////////////////////////////////
    std::ostringstream oss;
    string filename;
    filename = "machine.";
    filename += host_name;
    filename += ":";
    oss << host_port;
    filename += oss.str();
    filename += ".log";
    cout << "AIEEEEEEEEEEE " << filename << " host" << host_name << std::endl;
    char* fr_name = filename.c_str();
    FILE *fr = fopen(fr_name, "a");
    if(fr == NULL)
        printf("File %s Cannot be opened file on server.\n", fr_name);
    else
    {
        bzero(revbuf, 512);
        int fr_block_sz = 0;
        while((fr_block_sz = recv(hsock, revbuf, 512, 0)) > 0)
        {
            int write_sz = fwrite(revbuf, sizeof(char), fr_block_sz, fr);
            if(write_sz < fr_block_sz)
            {
                printf("File write failed on server.\n");
            }
            bzero(revbuf, 512);
            if (fr_block_sz == 0 || fr_block_sz != 512)
            {
                break;
            }
        }
        if(fr_block_sz < 0)
        {
            if (errno == EAGAIN)
            {
                printf("recv() timed out.\n");
            }
            else
            {
                fprintf(stderr, "recv() failed due to errno = %d\n", errno);
                exit(1);
            }
        }
        printf("Ok received from client!\n");
        fclose(fr);
    }
    close(hsock);
    pthread_exit(NULL);

FINISH:
    return 0;

}

ConnectionHandler::~ConnectionHandler()
{
//    close(getFd());
//    setFd(-1);
}

