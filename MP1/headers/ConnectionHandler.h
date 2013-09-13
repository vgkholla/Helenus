#ifndef CONNECTIONHANDLER_HEADER
#define CONNECTIONHANDLER_HEADER

#include <iostream>
#include <map>
#include <string>

using namespace std;

namespace P2P
{

    class Peer;
    
    typedef struct _mystruct{
        int *sock;
        void* owner;
        string cmd;
        string hostAndPort;
    } mystruct;

    class ConnectionHandler
    {
        private:
            /** Owner. */
            Peer      *owner;

        public:
            int machine_no;

            std::map<std::string,std::string> peers;

            std::string data;

            /**
             * Constructor.
             *
             * @param owner  Owner.
             */
            explicit ConnectionHandler(string src,
                                       int machineno,
                                       std::list<string> dest);

            /**
             * Distructor.
             */
            ~ConnectionHandler();

        private:
            /** No use of the default constructor */
            //ConnectionHandler();

            /** No use of the copy constructor */
            //ConnectionHandler(ConnectionHandler &);

            static void* SocketHandler(void *lp);
            static void* ClientHandler(void *lp);

    };

};

#endif /* CONNECTIONHANDLER_HEADER */

