
#ifndef PEER_HEADER
#define PEER_HEADER

#include <iostream>
#include <list>
#include <map>

namespace P2P
{
    class Peer 
    {
        public:

            std::list<int> destinations;
   
            std::string filepath;

            /**
             * Constructor.
             *
             * @param debugDumpPath  Path to debug dump file.
             */
            explicit Peer(int src,
                          std::string filename,
                          std::list<int> dest);
            /**
             * Destructor.
             */
            ~Peer();

        private:
    };

};
#endif /* PEER_HEADER */
