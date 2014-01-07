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
#include "headers/utility.h"
#define SIZE 1024

using namespace std;

int main(int argc,
         char **argv){


    if (argc < 5) {
        std::cerr << "Usage: " << argv[0] << " --dst DEST_IP --port DEST_PORT --word WORD" << std::endl;
        return 1;
    }
    int host_port;
    string host_name;
    string word = "";
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
        else if (std::string(argv[i]) == "--word") {
            if (i + 1 < argc) { 
                word += argv[++i];
                while(i + 1 < argc) {
                    word += " ";
                    word += argv[++i];
                }
            } else { 
                  std::cerr << "--word option requires one argument." << std::endl;
                return 1;
            }
        }
    }
    
    string wordToSend;
    if(word == "") {
        cout<<"******************Movie search application**********************"<<endl<<endl;
        wordToSend = CommandLineTools::showAndHandlePrompt("Enter word to search: ");
    } else {
        wordToSend = word;
    }
    
    while(wordToSend != "@exit") {
        string command = "lookup(" + wordToSend + ")";
        
        string msg = Utility::tcpConnectSocket(host_name.c_str(),host_port,command);

	    cout << msg << endl;
        if(word == "") {
            wordToSend = CommandLineTools::showAndHandlePrompt("Enter word to search: ");
        } else {
            wordToSend = "@exit";
        }
    }
}
