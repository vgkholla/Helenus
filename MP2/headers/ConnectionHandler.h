#ifndef CONNECTIONHANDLER_HEADER
#define CONNECTIONHANDLER_HEADER

#include <iostream>
#include <map>
#include <string>
#include <sstream>

#include <boost/serialization/list.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include "Timer.h"

using namespace std;

namespace P2P
{

    class Peer;
    
    /** My structuire
      * Accept Socket fd
      * The this pointer
      * The command to be run on peers
      * Host and Port of peers
      */
    typedef struct _mystruct{
        int sock;
        void* owner;
        string cmd;
        string hostAndPort;
    } mystruct;

    class ConnectionHandler: public Timer
    {
        private:
            /** Peer Owner. */
            Peer      *owner;

        public:
            /** Our Machine Number */
            int machine_no;

            /** Our Peers Addresses and Connectivity State */
            std::map<std::string,std::string> peers;

            static int sendFile(string filept, void *lp);

            /**
             * Constructor.
             *
             * @src  My IP Address.
             * @machineno My Machine no.
             * @dest My peer IP addresses and Connectivity state.
             */
            explicit ConnectionHandler(string src,
                                       int machineno,
                                       std::list<string> dest,
                                       int time);

            /**
             * Destructor.
             */
            ~ConnectionHandler();

        private:
            /** Handle connection from a client */
            static void* SocketHandler(void *lp);
  
            /** Handle File Transfer from peers */
            static void* ClientHandler(void *lp);

        protected:
            virtual void executeCb();

    };

};

#endif /* CONNECTIONHANDLER_HEADER */

