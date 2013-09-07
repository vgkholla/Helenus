#ifndef AG_ERROR_CODES
#define AG_ERROR_CODES
//don't want multiple declarations

#define SUCCESS 0 //code for success
#define FAILURE -1//code for fail

//I/O error codes go here
#define NO_ERROR 0
#define ERROR_NO_OPEN 1
#define ERROR_NO_WRITE 2
#define ERROR_NO_READ 3
#define ERROR_IO_WRITE 4
#define ERROR_IO_LOGIC 5

//logged error types go here
#define IO_ERROR 1
#define NETWORK_ERROR 2
#define HANDSHAKE_FAILURE 3
#define DEST_NOT_FOUND 4
#define INVALID_OPERATION 5


#endif