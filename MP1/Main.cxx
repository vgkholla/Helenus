#include <cstdlib>
#include <csignal>
#include <vector>
#include <exception>
#include <getopt.h>
#include <syslog.h>
#include <list>

#include "ConnectionHandler.h"
using namespace std;
using namespace P2P;

int main(int argc,
         char **argv)
{
    if (argc < 5) {
        std::cerr << "Usage: " << argv[0] << " --src SOURCE --file FILENAME DESTINATIONS" << std::endl;
        return 1;
    }
    std::list <int> dest;
    int src;
    std::string filename;
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--src") {
            if (i + 1 < argc) { // Make sure we aren't at the end of argv!
                src = atoi(argv[++i]); // Increment 'i' so we don't get the argument as the next argv[i].
            } else { // Uh-oh, there was no argument to the destination option.
                  std::cerr << "--src option requires one argument." << std::endl;
                return 1;
            }
        } 
        else if (std::string(argv[i]) == "--file") {
            if (i + 1 < argc) { // Make sure we aren't at the end of argv!
                filename = argv[++i]; // Increment 'i' so we don't get the argument as the next argv[i].
            } else { // Uh-oh, there was no argument to the destination option.
                  std::cerr << "--src option requires one argument." << std::endl;
                return 1;
            }
        }        
        else {
            dest.push_back(atoi(argv[i]));
        }
    }
    cout << filename << std::endl;
    ConnectionHandler conn(src,filename,dest);
    syslog(LOG_INFO,"\nPeer ran");
    while (1) {}
    return (0);
}




