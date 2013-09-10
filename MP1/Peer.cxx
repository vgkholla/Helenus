#include <iostream>
#include <csignal>
#include <syslog.h>
#include <list>
#include <map>

#include "ConnectionHandler.h"

#include "Peer.h"

using namespace P2P;
using namespace std;

Peer::Peer(int src,
           std::string filename,
           std::list<int> dest)
{
    syslog(LOG_INFO,"Callin tdm con ");
    destinations = dest;
    filepath = filename;
    cout<< "My log file " << filepath << std::endl;
    new ConnectionHandler(this,src);
}

Peer::~Peer()
{
}
