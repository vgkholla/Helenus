#ifndef AG_ERROR_CODES
#define AG_ERROR_CODES
//don't want multiple declarations

#define SUCCESS 1 //code for success
#define FAILURE 0//code for fail
#define NO_ERROR 0
#define HIGHEST_FILE_ERROR_CODE 5
#define HIGHEST_MEMBER_DEBUG_CODE 4 

//I/O error codes go here
#define ERROR_NO_OPEN 1
#define ERROR_NO_WRITE 2
#define ERROR_NO_READ 3
#define ERROR_IO_WRITE 4
#define ERROR_IO_LOGIC 5

//example logged error types go here
#define IO_ERROR -1
#define NETWORK_ERROR -2
#define HANDSHAKE_FAILURE -3
#define DEST_NOT_FOUND -4
#define INVALID_OPERATION -5

//distributed command exection error codes
#define COMMAND_FAILED_TO_EXEC_AT_PEER 1
#define FILE_TRANSFER_FAILED 2

//debug types for membership
#define NOP 0
#define MEMBER_JOINED 1
#define MEMBER_LEFT 2
#define MEMBER_SUSPECTED_FAILED 3
#define MEMBER_FAILED 4

//error types while performing membership operations
#define INDEX_ACCESS_FAILED 6
#define EMPTY_LIST 7
#define SELF_ENTRY_NOT_FOUND 8
#define ERROR_ALREADY_EXISTS 9
#define SERIALIZATION_ERROR 10
#define SOCKET_ERROR 11

//key value store errors
#define NO_SUCH_KEY 12
#define TOO_MANY_KEYS 13
#define KEY_EXISTS 14
#define INSERT_FAILED 15
#define UPDATE_FAILED 16

//parsing command errors
#define MALFORMED_COMMAND 17

#endif
