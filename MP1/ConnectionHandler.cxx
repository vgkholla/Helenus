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

#include "ConnectionHandler.h"
using namespace std;
using namespace P2P;

ConnectionHandler::ConnectionHandler(int src,
                                     std::string filename,
                                     std::list<int> dest)
{
    int host_port= src;
    struct sockaddr_in my_addr;
    mystruct* f = new mystruct;

    int hsock;
    int * p_int ;
    int err;

    socklen_t addr_size = 0;
    int* csock;
    sockaddr_in sadr;
    pthread_t thread_id=0;

    filepath = filename;
    for (std::list<int>::iterator it = dest.begin(); it != dest.end(); it++)
        peers[*it] = "unconnected";

    for(std::map<int,string>::const_iterator it = peers.begin();
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
    my_addr.sin_port = htons(host_port);
    
    memset(&(my_addr.sin_zero), 0, 8);
    my_addr.sin_addr.s_addr = INADDR_ANY ;
    
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
        printf("waiting for a connection on port %d\n",host_port);
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
    int bytecount;
    int thread_cnt = ptr1->peers.size();
    pthread_t thread_id[thread_cnt];
    pthread_attr_t attr;
    void* status;
    int rc;
    long t;

    memset(buffer, 0, buffer_len);
    if((bytecount = recv(*ptr->sock, buffer, buffer_len, 0))== -1){
        fprintf(stderr, "Error receiving data %s\n", strerror(errno));
        goto FINISH;
    }
    printf("Received bytes %d\nReceived string \"%s\"\n", bytecount, buffer);
    cout << "file nameeeeeeeeeee" << ptr1->filepath.c_str();
    if (memcmp(buffer,"peer",4)==0) {
        strcat(buffer, " SERVER ECHO TIAGI");
        printf("\n got Connection from a peer, sending back data");

/////////////////////////////////////////////////////////////////////////////////////////////////////////
//        char* fs_name = "/tmp/house.jpg";
        char* fs_name = ptr1->filepath.c_str();
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
/////////////////////////////////////////////////////////////////////////////////////////////////////////

        //if((bytecount = send(*ptr->sock, buffer, strlen(buffer), 0))== -1){
        //    fprintf(stderr, "Error sending data %d\n", errno);
        //    goto FINISH;
        //}
        //printf("Sent bytes %d\n", bytecount);
    }
    else {
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
        for(t=0; t < thread_cnt; t++)
        {
            printf("\nOpening connectionn to peers socket fd ");
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
 

        if((bytecount = send(*ptr->sock, ptr1->data.c_str(), strlen(ptr1->data.c_str()), 0))== -1){
            fprintf(stderr, "Error sending data %d\n", errno);
            goto FINISH;
        }
        printf("Sent bytes %d\n", bytecount);

    }

    pthread_exit(NULL);

FINISH:
    //free(csock);
    return 0;
}

void* ConnectionHandler::ClientHandler(void* lp)
{
    int host_port;
    char* host_name="127.0.0.1";
    mystruct *ptr = (mystruct*)lp;
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

    for(map<int,string >::const_iterator it = ptr1->peers.begin();
        it != ptr1->peers.end(); ++it)
    {
        if(it->second == "unconnected")
        {
            host_port = it->first;
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
    my_addr.sin_addr.s_addr = inet_addr(host_name);

    if( connect( hsock, (struct sockaddr*)&my_addr, sizeof(my_addr)) == -1 ){
        if((err = errno) != EINPROGRESS){
            fprintf(stderr, "Error connecting socket %d\n", errno);
            //goto FINISH;
        }
    }

    buffer_len1 = 1024;

    memset(buffer1, '\0', buffer_len1);

    memcpy(buffer1,"peer",4);
    buffer1[strlen(buffer1)]='\0';

    if( (bytecount1=send(hsock, buffer1, strlen(buffer1),0))== -1){
        fprintf(stderr, "Error sending data %d\n", errno);
        //goto FINISH;
    }

//    if((bytecount1 = recv(hsock, buffer1, buffer_len1, 0))== -1){
//        fprintf(stderr, "Error receiving data %d\n", errno);
//        goto FINISH;
//    }
//////////////////////////////////////////////////////////////////////////////////////////////////////
    char* fr_name = "/home/alok/project/build/house1.jpg";
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
///////////////////////////////////////////////////////////////////////////////////////////////////////

//    ptr1->data.append(buffer1);
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

