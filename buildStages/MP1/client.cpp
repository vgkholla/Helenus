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

    /* Take command from the user */
    cout << "Enter the command to be sent to the server (press enter)" << endl;
    if(cmd.empty())
        cmd = CommandLineTools::showAndHandlePrompt(1);
    time_t start = time(0);
    strcat(buffer, "client@");
    strcat(buffer, cmd.c_str());
    buffer[strlen(buffer)]='\0';
    
    if( (bytecount=send(hsock, buffer, strlen(buffer),0))== -1){
        cout << "Error sending command to server " << strerror(errno) << endl;
        exit(1);
    }

    string filename;
    filename = "FinalOutput";
    const char* fr_name = filename.c_str();
    FILE *fr = fopen(fr_name, "a");
    if(fr == NULL) {
        cout << "File " << fr_name << " Cannot be opened" << endl;
        exit(1);
    }
    else
    {
        /* Receive the file size from the server
           and then receive the full file
         */
        bzero(revbuf, SIZE);
        int fr_block_sz = 0;
        std::string size;
        int count = 0;
        int filesize= 0;
        int rcv_filesize = 0;
 
        /* Receive file size */
        if((count = recv(hsock, size.c_str(), SIZE, 0))== -1){
            cout << "Error receiving file size from server" << strerror(errno) << endl;
            exit(1);
        }
        filesize = atoi(size.c_str());
        int write_sz = fwrite(size.c_str(), sizeof(char), count, fr);
        if(write_sz < fr_block_sz)
        {
            cout << "File write failed" << endl;
            exit(1);
        }
        rcv_filesize += fr_block_sz;
        bzero(revbuf, SIZE);

        cout << "FILE SIZE RETURNED " << filesize << std::endl;
        /* Start receiving the file */
        while((fr_block_sz = recv(hsock, revbuf, SIZE, 0)) >  0)
        {
            write_sz = fwrite(revbuf, sizeof(char), fr_block_sz, fr);
            if(write_sz < fr_block_sz)
            {
                cout << "File write failed " << endl;
                exit(1);
            }
            bzero(revbuf, SIZE);
            rcv_filesize += fr_block_sz;
            if (rcv_filesize >= filesize)
            {
                break;
            }
        }
        if(fr_block_sz < 0)
        {
            if (errno == EAGAIN)
            {
                cout << "File recv timed out" << endl;
            }
            else
            {
                cout << "File recv failed due to errno " << strerror(errno) << endl;
                exit(1);
            }
        }
        time_t end = time(0);
        cout<<"Time taken: " << end -start <<endl;
        cout << "Log files received" << endl;
        fclose(fr);
    }

    close(hsock);
}
