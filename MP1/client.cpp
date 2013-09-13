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

int main(){

    int host_port= 45001;
    string host_name="192.17.11.5";

    struct sockaddr_in my_addr;

    char buffer[1024];
    char revbuf[SIZE];
    int bytecount;
    int buffer_len=0;

    int hsock;
    int *p_int;
    int err;

    hsock = socket(AF_INET, SOCK_STREAM, 0);
    if(hsock == -1){
        printf("Error initializing socket %d\n",errno);
        exit(1);
    }
    
    p_int = (int*)malloc(sizeof(int));
    *p_int = 1;
        
    if( (setsockopt(hsock, SOL_SOCKET, SO_REUSEADDR, (char*)p_int, sizeof(int)) == -1 )||
        (setsockopt(hsock, SOL_SOCKET, SO_KEEPALIVE, (char*)p_int, sizeof(int)) == -1 ) ){
        printf("Error setting options %d\n",errno);
        free(p_int);
        exit(1);
    }
    free(p_int);

    my_addr.sin_family = AF_INET ;
    my_addr.sin_port = htons(host_port);
    
    memset(&(my_addr.sin_zero), 0, 8);
    my_addr.sin_addr.s_addr = inet_addr(host_name.c_str());

    if( connect( hsock, (struct sockaddr*)&my_addr, sizeof(my_addr)) == -1 ){
        if((err = errno) != EINPROGRESS){
            fprintf(stderr, "Error connecting socket %s\n", strerror(errno));
            exit(1);
        }
    }

    //Now lets do the client related stuff

    buffer_len = 1024;

    memset(buffer, '\0', buffer_len);

    printf("Enter some text to send to the server (press enter)\n");
    string cmd = CommandLineTools::showAndHandlePrompt(1);
    //fgets(buffer, 1024, stdin);
    strcat(buffer, "client@");
    strcat(buffer, cmd.c_str());
    buffer[strlen(buffer)]='\0';
    
    if( (bytecount=send(hsock, buffer, strlen(buffer),0))== -1){
        fprintf(stderr, "Error sending data %d\n", errno);
        exit(1);
    }
    printf("Sent bytes %d\n", bytecount);








    string filename;
    filename = "FinalOutput";
    const char* fr_name = filename.c_str();
    FILE *fr = fopen(fr_name, "a");
    if(fr == NULL)
        printf("File %s Cannot be opened file on server.\n", fr_name);
    else
    {
        bzero(revbuf, SIZE);
        int fr_block_sz = 0;
        while((fr_block_sz = recv(hsock, revbuf, SIZE, 0)) > 0)
        {
            int write_sz = fwrite(revbuf, sizeof(char), fr_block_sz, fr);
            if(write_sz < fr_block_sz)
            {
                printf("File write failed on server.\n");
            }
            bzero(revbuf, SIZE);
            if (fr_block_sz == 0 || fr_block_sz != SIZE)
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
}
