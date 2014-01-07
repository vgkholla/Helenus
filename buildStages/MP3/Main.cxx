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
        std::cerr << "Usage: " << argv[0] << " --src SOURCE --machineno MACHINENO --interval INTERVAL --sendpct SENDPCT DESTINATIONS" << std::endl;
        return 1;
    }
    /* Accept the command inputs
     * My IP address
     * My machine no
     * My peers IP Addresses
     */
    std::list <string> dest;
    string src;
    int machineno;
    float interval;
    int sendPercentage;
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--src") {
            if (i + 1 < argc) { 
                src = argv[++i]; 
            } else { 
                  std::cerr << "--src option requires one argument." << std::endl;
                return 1;
            }
        } 
        else if (std::string(argv[i]) == "--machineno") {
            if (i + 1 < argc) { 
                machineno = atoi(argv[++i]); 
            } else { 
                  std::cerr << "--machineno option requires one argument." << std::endl;
                return 1;
            }
        }        
        else if (std::string(argv[i]) == "--interval") {
            if (i + 1 < argc) {
                interval = atof(argv[++i]);
            } else {
                  std::cerr << "--interval option requires one argument." << std::endl;
                return 1;
            }
        }
        else if (std::string(argv[i]) == "--sendpct") {
            if (i + 1 < argc) {
                sendPercentage = atof(argv[++i]);
            } else {
                  std::cerr << "--sendpct option requires one argument." << std::endl;
                return 1;
            }
        }
        else {
            dest.push_back(argv[i]);
        }
    }

    /** Start the Peer with IP address and its machine no */
    ConnectionHandler conn(src,
                           machineno,
                           dest,
                           sendPercentage,
                           interval);
    syslog(LOG_INFO,"\nStarting Peer");
    while (1) {}
    return (0);
}




