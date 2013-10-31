#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <netinet/in.h>
#include <resolv.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "headers/clt.h"
#define SIZE 128

using namespace std;

int main(int argc,
         char **argv){


    if (argc < 5) {
        std::cerr << "Usage: " << argv[0] << " --dst DEST_IP --port DEST_PORT --command COMMAND" << std::endl;
        return 1;
    }
    int host_port;
    string host_name;
    string cmd;
    /* Receive Command Line Input for dest IP
       dest port and optional command
     */
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--dst") {
            if (i + 1 < argc) { 
                host_name = argv[++i];
            } else { 
                  std::cerr << "--dst option requires one argument." << std::endl;
                return 1;
            }
        }
        else if (std::string(argv[i]) == "--port") {
            if (i + 1 < argc) { 
                host_port = atoi(argv[++i]); 
            } else { 
                  std::cerr << "--port option requires one argument." << std::endl;
                return 1;
            }
        }
        else if (std::string(argv[i]) == "--command") {
            if (i + 1 < argc) { 
                cmd = argv[++i]; 
            } else { 
                  std::cerr << "--command option requires one argument." << std::endl;
                return 1;
            }
        }
    }

    struct sockaddr_in my_addr;

    char buffer[1024];
    char revbuf[SIZE];
    int bytecount;
    int buffer_len=0;

    int hsock;
    int *p_int;
    int err;
    /* Open socket to connect to
       Bugsy Inc logging system
     */
    hsock = socket(AF_INET, SOCK_STREAM, 0);
    if(hsock == -1){
        cout << "Error initializing socket " << strerror(errno) << endl;
        exit(1);
    }
    
    p_int = (int*)malloc(sizeof(int));
    *p_int = 1;
        
    if( (setsockopt(hsock, SOL_SOCKET, SO_REUSEADDR, (char*)p_int, sizeof(int)) == -1 )||
        (setsockopt(hsock, SOL_SOCKET, SO_KEEPALIVE, (char*)p_int, sizeof(int)) == -1 ) ){
        cout << "Error setting socket options " << strerror(errno) << endl;
        free(p_int);
        exit(1);
    }
    free(p_int);

    my_addr.sin_family = AF_INET ;
    my_addr.sin_port = htons(host_port);
    
    memset(&(my_addr.sin_zero), 0, 8);
    my_addr.sin_addr.s_addr = inet_addr(host_name.c_str());

    cout << "Connecting to Bugsy Logging System " << endl;

    if( connect( hsock, (struct sockaddr*)&my_addr, sizeof(my_addr)) == -1 ){
        if((err = errno) != EINPROGRESS){
            cout << "Error connecting socket " << strerror(errno) << endl;
            exit(1);
        }
    }

    buffer_len = 1024;
    memset(buffer, '\0', buffer_len);


    string cmdToSend = CommandLineTools::showAndHandlePrompt("1");

    while(cmdToSend != "exit") {
        KeyValueStoreCommand command = CommandLineTools::parseKeyValueStoreCmd(cmdToSend);

        if(command.isValidCommand()) {

            if( (bytecount=send(hsock, cmdToSend.c_str(), strlen(cmdToSend.c_str()),0))== -1){
                cout << "Error sending command to server " << strerror(errno) << endl;
            	exit(1);
            }

            memset(buffer, '\0', buffer_len);
            int count;
            /* Receive file size */
            if((count = recv(hsock, buffer, SIZE, 0))== -1){
                cout << "Error receiving file size from server" << strerror(errno) << endl;
            	exit(1);
            }
    	    cout << buffer << endl;


        } else {
            cout<<"Malformed command!"<<endl;
        }
        cmdToSend = CommandLineTools::showAndHandlePrompt("1");
    }
    close(hsock);
}
