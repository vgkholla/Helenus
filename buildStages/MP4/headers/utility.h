#ifndef AG_UTILITY
#define AG_UTILITY
//don't want multiple declarations

#include <iomanip>
#include <locale>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <iostream>
#include <vector>
#include <errno.h>
#include <sys/ioctl.h>
#include <fstream>
#include <list>

using namespace std;

class Utility {

	public:

	/**
	 * [utility function to convert an integer to a string]
	 * @param  num the integer to be converted
	 * @return the integer converted to string
	 */
	static string intToString(int num) {
		return static_cast<ostringstream*>( &(ostringstream() << num) )->str();
	} 

	/**
	 * [trims out trailing spaces]
	 * @param  str [input string]
	 * @return     [string without trailing spaces]
	 */
	static string trimTrailingSpaces(string str) {
		size_t strlength = str.length();
		while(str[strlength - 1] == ' ' || str[strlength - 1] == '\t') {
			str.erase(strlength - 1, strlength);
			strlength = str.length();
		}
		return str;
	}

	/**
	 * [check if character is escaped]
	 * @param  str [the string]
	 * @param  pos [the position of character]
	 * @return     [whether the character is escaped or not]
	 */
	static int isEscaped(string str, size_t pos) {
		int i = 0;
		while(str[pos - 1] == '\\'){
			i++;
			pos--;
		}

		return i % 2;
	}

    /**
     *  [combines two given vectors]
     * @param  vector1 [the first vector]
     * @param  vector2 [the second vector]
     * @return     [the combined vector]
     */
    static vector <string> combineVectors(vector<string> vector1, vector<string> vector2) {
        vector<string> combinedVector;
        combinedVector.reserve(vector1.size() + vector2.size());
        combinedVector.insert(combinedVector.end(), vector1.begin(), vector1.end());
        combinedVector.insert(combinedVector.end(), vector2.begin(), vector2.end());

        return combinedVector;
    }

    static int udpSocket(string address, int port) {

        int hsock;
        int * p_int ;
        struct sockaddr_in my_addr;
	    //int errCode = 0;
	    //socklen_t addr_size = 0;
	    //sockaddr_in sadr;

        hsock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if(hsock == -1){
            string msg = "Failed to open socket";
            cout<<msg<<endl;
            //logger->logError(SOCKET_ERROR, msg , &errCode);
            return -1;
        } 

        p_int = (int*)malloc(sizeof(int));
        *p_int = 1;

        /* Setting socket options */
        if( (setsockopt(hsock, SOL_SOCKET, SO_REUSEADDR, (char*)p_int, sizeof(int)) == -1 )||
            (setsockopt(hsock, SOL_SOCKET, SO_KEEPALIVE, (char*)p_int, sizeof(int)) == -1 ) ){
            string msg = "Failed to set socket options";
            cout<<msg<<endl;
            //logger->logError(SOCKET_ERROR, msg , &errCode);
            free(p_int);
            return -1;
        }
        free(p_int);

        my_addr.sin_family = AF_INET;
        my_addr.sin_port = htons(port);

        memset(&(my_addr.sin_zero), 0, 8);
        my_addr.sin_addr.s_addr = inet_addr(address.c_str());;

        if( bind( hsock, (sockaddr*)&my_addr, sizeof(my_addr)) == -1 ){
            string msg = "Failed to bind to socket";
            cout<<msg<<endl;
            //logger->logError(SOCKET_ERROR, msg , &errCode);
            return -1;
        }
        return hsock;
    }

    static int tcpSocket(string address, int port) {

        int hsock;
        int * p_int ;
        struct sockaddr_in my_addr;
        //int errCode = 0; 
        //socklen_t addr_size = 0;
        //int* csock;
        // sockaddr_in sadr;

        hsock = socket(AF_INET, SOCK_STREAM, 0);
        if(hsock == -1){
            string msg = "Failed to open socket";
            cout << msg << endl;
            //logger->logError(SOCKET_ERROR, msg , &errCode);
            return -1;
        }

        p_int = (int*)malloc(sizeof(int));
        *p_int = 1;

        /* Setting socket options */
        if( (setsockopt(hsock, SOL_SOCKET, SO_REUSEADDR, (char*)p_int, sizeof(int)) == -1 )||
            (setsockopt(hsock, IPPROTO_TCP, TCP_NODELAY, (char*)p_int, sizeof(int)) == -1 )||
            (setsockopt(hsock, SOL_SOCKET, SO_KEEPALIVE, (char*)p_int, sizeof(int)) == -1 ) ){
            string msg = "Failed to set socket options";
            cout << msg << endl;
            //logger->logError(SOCKET_ERROR, msg , &errCode);
            free(p_int);
            return -1;
        }
        free(p_int);

        my_addr.sin_family = AF_INET ;
        my_addr.sin_port = htons(port);

        memset(&(my_addr.sin_zero), 0, 8);
        my_addr.sin_addr.s_addr = inet_addr(address.c_str());;

        if( bind(hsock, (sockaddr*)&my_addr, sizeof(my_addr)) == -1 ){
            string msg = "Failed to bind to socket";
            cout << msg << endl;
            //logger->logError(SOCKET_ERROR, msg , &errCode);
            return -1;
        }
        if(listen(hsock, 10) == -1 ){
            string msg = "Failed to listen on socket";
            cout << msg << endl;
            //logger->logError(SOCKET_ERROR, msg , &errCode);
            return -1;
        }
        return hsock;
    }

    static string tcpConnectSocket(string address, int port, string msg) {

        int hsock;
        int * p_int ;
        struct sockaddr_in my_addr;
        //char buffer[12288];
        //int buffer_len = 12288;
        //int bytecount;
        //int errCode = 0;
        //socklen_t addr_size = 0;
        //int* csock;
        //sockaddr_in sadr;

        hsock = socket(AF_INET, SOCK_STREAM, 0);
        if(hsock == -1){
            string msg = "Failed to open socket";
            cout << msg << endl;
            //logger->logError(SOCKET_ERROR, msg , &errCode);
        }

        p_int = (int*)malloc(sizeof(int));
        *p_int = 1;

        /* Setting socket options */
        if( (setsockopt(hsock, SOL_SOCKET, SO_REUSEADDR, (char*)p_int, sizeof(int)) == -1 )||
            (setsockopt(hsock, SOL_SOCKET, SO_KEEPALIVE, (char*)p_int, sizeof(int)) == -1 ) ){
            string msg = "Failed to set socket options";
            cout << msg << endl;
            //logger->logError(SOCKET_ERROR, msg , &errCode);
            free(p_int);
        }
        free(p_int);

        my_addr.sin_family = AF_INET ;
        my_addr.sin_port = htons(port);

        memset(&(my_addr.sin_zero), 0, 8);
        my_addr.sin_addr.s_addr = inet_addr(address.c_str());

        if( connect( hsock, (struct sockaddr*)&my_addr, sizeof(my_addr)) == -1 ){
            string msg = "Failed to Connect";
            cout << msg << endl;
            close(hsock);
            hsock = -1;
            return msg;
            //logger->logError(SOCKET_ERROR, msg , &errCode);
        }

        if(sendData(hsock,msg) < 0) {
            string msg = "Failed to send";
            return msg;
        }    
        string result = rcvData(hsock);

        close(hsock);
        hsock = -1;
        return result;

    }
    static int sendData(int sock, string msg) {

        char buffer[1024];
        int buffer_len = 1024;
        std::ostringstream oss;
        int len = strlen(msg.c_str());

        if(send(sock, &len, sizeof(len), 0) < 0)
        {
                cout << "ERROR: Failed to send file size" << strerror(errno) << endl;
                return -1;
        }
        memset(buffer, 0, buffer_len);
        size_t pos = 0, last_pos = 0;
        std::string::iterator it = msg.begin();
        do
        {
            if(it == msg.end())
            {
                std::size_t length = msg.copy(buffer,msg.length(),last_pos);
                buffer[length]='\0';
                if(send(sock, buffer, buffer_len, 0) < 0)
                {
                        cout << "ERROR: Failed to send file size" << strerror(errno) << endl;
                        return -1;
                }
                break;
            }
            if((pos - last_pos) == 1024)
            {
                std::size_t length = msg.copy(buffer,pos - last_pos,last_pos);
                if(send(sock, buffer, buffer_len, 0) < 0)
                {
                        cout << "ERROR: Failed to send file size" << strerror(errno) << endl;
                        return -1;
                }
                last_pos = pos++;
            }
            pos++;
            ++it;
        }
        while(true);
        return 0;
    }

    static string rcvData(int sock) {
        int size;
        int rcv = 0;
        string result;
        char buffer[1024];
        int buffer_len = 1024;
        int bytecount;

        memset(buffer, 0, buffer_len);
        if((bytecount = recv(sock, &size, sizeof(size), 0))== -1)
        {
            string msg = "Failed to receive reply via socket during size get";
            cout << msg << endl;
            return msg;
            //logger->logError(SOCKET_ERROR, msg , &errCode);
        }

        memset(buffer, '\0', buffer_len);
        while(rcv < size) {
            if((bytecount = recv(sock, buffer, buffer_len, 0)) <= 0)
            {
                string msg = "Failed to receive reply via socket during data get";
                cout << msg << endl;
                return msg;
            }
            else {
                for(int i =0 ; buffer[i] != '\0' && i < bytecount; i++) {
                    result += buffer[i];
                }
                rcv += bytecount;
                memset(buffer, '\0', buffer_len);
            }
        }
        return result;
    }

    static string findMovies(string indexes) {
        string line;
        ifstream Myfile;
        std::size_t next = 0;
        list<int> lineno;
        while(next < indexes.length())
        {
            next = indexes.find(",");
            lineno.push_back(atoi((indexes.substr(0,next)).c_str()));
            indexes = indexes.substr(next+1,indexes.length() - next);
        }
        if(lineno.size() == 0) {
            indexes = "No Matching Movies Found";
            return indexes;
        }

        Myfile.open ("top2000");
        int linenum = 1;
        indexes = "";
        if(Myfile.is_open())
        {
            while(!Myfile.eof())
            {
                getline(Myfile,line);
                for (std::list<int>::iterator it=lineno.begin(); it != lineno.end(); ++it)
                {
                    if(linenum == *it)
                    {
                        indexes += line;
                        indexes += "\n";
                    }
                }
                linenum++;
            }
            Myfile.close();
        }
        indexes += "\0";
        return indexes;
    }

};

#endif
