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
#include "membershipList.h"

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
        MembershipList *mPtr;
    } mystruct;

    class ConnectionHandler: public Timer
    {
        private:
            /** Peer Owner. */
            Peer      *owner;

        public:
            /** Our Machine Number */
            int machine_no;
            /** Send percentage */
            int sendPct;
        
            MembershipList *memListPtr;
            /* Store membership list ptr */
            void setMemPtr(MembershipList *memPtr)
            {
                memListPtr = memPtr;
            } 

            /* return membership list ptr */
            MembershipList* getMemPtr()
            {
                return memListPtr;
            }
     
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
                                       int sendPercentage,
                                       float time);

            /**
             * Destructor.
             */
            ~ConnectionHandler();

        private:
            /** Handle connection from a client */
            static void* SocketHandler(void *lp);
  
            /** Handle File Update Membership List */
            
            static void* updateMembershipList(void *lp);
            
            /* Send membership List */
            void sendMemberList(vector<string> memberList);
            
            /* Signal handler */
            static void sendLeaveMsg(int signal);

        protected:
            /* Call back funtion called when timer expires */
            virtual void executeCb();

    };

};

#endif /* CONNECTIONHANDLER_HEADER */

