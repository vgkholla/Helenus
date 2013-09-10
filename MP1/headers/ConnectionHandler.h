#ifndef CONNECTIONHANDLER_HEADER
#define CONNECTIONHANDLER_HEADER

#include <iostream>

#include "Peer.h"


namespace P2P
{

    class Peer;
    
    typedef struct _mystruct{
        int *sock;
        void* owner;
    } mystruct;

    class ConnectionHandler
    {
        private:
            /** Owner. */
            Peer      *owner;

        public:
            std::map<int,std::string> peers;
            std::string data;

            /**
             * Constructor.
             *
             * @param owner  Owner.
             */
            explicit ConnectionHandler(Peer *owner,
                                       int src);

            /**
             * Distructor.
             */
            ~ConnectionHandler();

        private:
            /** No use of the default constructor */
            ConnectionHandler();

            /** No use of the copy constructor */
            ConnectionHandler(ConnectionHandler &);

            static void* SocketHandler(void *lp);
            static void* ClientHandler(void *lp);

    };

};

#endif /* CONNECTIONHANDLER_HEADER */

