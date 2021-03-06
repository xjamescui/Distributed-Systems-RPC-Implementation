/******************************************************************
 * this file contains general #define                             *
 *                                                                *
 *                                                                *
 *******************************************************************/

/******************************************************************
 * Some Strings                                                   *
 *                                                                *
 ******************************************************************/

#define BINDER_ADDRESS_STRING   "BINDER_ADDRESS"
#define BINDER_PORT_STRING      "BINDER_PORT"


/******************************************************************
 * message types                                                  *
 *                                                                *
 ******************************************************************/

#define PING                    0x01
#define PONG                    0x02

#define MSG_REGISTER            0x10
#define MSG_REGISTER_SUCCESS    0x11
#define MSG_REGISTER_FAILURE    0x12

#define MSG_LOC_REQUEST         0x20
#define MSG_LOC_SUCCESS         0x21
#define MSG_LOC_FAILURE         0x22

#define MSG_EXECUTE             0x30
#define MSG_EXECUTE_SUCCESS     0x31
#define MSG_EXECUTE_FAILURE     0x32

#define MSG_LOC_CACHE_REQUEST   0x40
#define MSG_LOC_CACHE_SUCCESS   0x41
#define MSG_LOC_CACHE_FAILURE   0x42

#define MSG_TERMINATE           0x70

#define MAX_RW_CHUNK_SIZE       16348

/******************************************************************
 * error codes                                                    *
 *                                                                *
 ******************************************************************/
#define MSG_REGISTER_SUCCESS_NO_ERRORS               0

#define MSG_REGISTER_SUCCESS_OVERRIDE_PREVIOUS       1

#define MSG_REGISTER_FAILURE_INVALID_SERVER_PORT    -1
#define MSG_REGISTER_FAILURE_INVALID_SERVER_IP      -2
#define MSG_REGISTER_FAILURE_INVALID_FCT_NAME       -3
#define MSG_REGISTER_FAILURE_INVALID_ARGTYPES       -4

#define MSG_LOC_FAILURE_SIGNATURE_NOT_FOUND         -1
#define MSG_LOC_FAILURE_SIGNATURE_NO_HOSTS          -2

#define MSG_TYPE_NOT_SUPPORTED                      -1


/******************************************************************
 * For host database with round robin algorithm                   *
 *                                                                *
 ******************************************************************/
// put
#define HOST_DB_PUT_SIGNATURE_SUCCESS            0
#define HOST_DB_PUT_SIGNATURE_DUPLICATE          1
#define HOST_DB_PUT_SIGNATURE_FAIL              -1

// get
#define HOST_DB_GET_SIGNATURE_FOUND              0
#define HOST_DB_GET_SIGNATURE_NOT_FOUND         -1
#define HOST_DB_GET_SIGNATURE_HAS_NO_HOSTS      -2

// delete
#define HOST_DB_DELETE_HOST_SUCCESS              0
#define HOST_DB_DELETE_HOST_NOT_FOUND            1
#define HOST_DB_DELETE_SIGNATURE_NOT_FOUND       2


// Skeleton Database
#define SKEL_RECORD_PUT_SUCCESS        0
#define SKEL_RECORD_PUT_DUPLICATE      1
#define SKEL_RECORD_PUT_FAIL          -1

#define SKEL_RECORD_FOUND              0
#define SKEL_RECORD_NOT_FOUND         -1

#define SKEL_RECORD_DELETE_SUCCESS     0
#define SKEL_RECORD_DELETE_FAIL       -1

// Skeleton method return codes
#define SKEL_EXEC_SUCCESS        0
#define SKEL_EXEC_FAIL          -1


/******************************************************************
 * RPC-specific                                                   *
 *                                                                *
 ******************************************************************/


#define RPC_ENVR_VARIABLES_NOT_SET              -100

#define RPC_NULL_PARAMETERS                     -200
#define RPC_INVALID_ARGTYPES                    -201

#define RPC_SOCKET_UNINITIALIZED                -1

#define RPC_SERVER_CREATE_SOCKET_SUCCESS         0
#define RPC_SERVER_CREATE_SOCKET_FAIL           -10
#define RPC_SERVER_BIND_SOCKET_FAIL             -11
#define RPC_SERVER_GET_SOCK_NAME_FAIL           -12

#define RPC_REGISTER_OVERRIDE_PREVIOUS           1
#define RPC_REGISTER_SERVER_SETUP_ERROR         -13
#define RPC_REGISTER_INVALID_FCT_NAME           -14
#define RPC_REGISTER_INVALID_ARGTYPES           -15
#define RPC_REGISTER_UNKNOWN_ERROR              -16

#define RPC_CALL_NO_HOSTS                       -17
#define RPC_CALL_INTERNAL_DB_ERROR              -18


#define RPC_INIT_SUCCESS                         0
#define RPC_REGISTER_SUCCESS                     0
#define RPC_CALL_SUCCESS                         0
#define RPC_CACHE_CALL_SUCCESS                   0
#define RPC_TERMINATE_SUCCESS                    0
#define RPC_EXECUTE_SUCCESS                      0

#define RPC_CONNECT_TO_BINDER_FAIL              -19
#define RPC_CONNECT_TO_SERVER_FAIL              -20

#define RPC_CONNECTION_SELECT_FAIL              -21

#define RPC_WRITE_TO_BINDER_FAIL                -22
#define RPC_READ_FROM_BINDER_FAIL               -23

#define RPC_WRITE_TO_SERVER_FAIL                -24
#define RPC_READ_FROM_SERVER_FAIL               -25



/******************************************************************
 * socket                                                         *
 *                                                                *
 ******************************************************************/

#define SOCKET_CREATE_FAIL               -1
#define SOCKET_BIND_FAIL                 -2
#define SOCKET_CONNECT_FAIL              -3
#define SOCKET_GET_SOCK_NAME_FAIL        -4


#define ASSEMBLE_MSG_FAIL                -1
#define EXTRACT_MSG_FAIL                 -2

#define READ_MSG_FAIL                    -1 
#define READ_MSG_ZERO_LENGTH             -2

#define WRITE_MSG_FAIL                   -1 


#define GET_IP_FROM_SOCKET_FAIL          -1

/******************************************************************
 * etc.                                                           *
 *                                                                *
 ******************************************************************/

#define ARG_TYPE_INVALID_SIZE            -1
#define IOCTL_ERROR                      -1 

// threads
#define PTHREAD_CREATE_FAIL              -1
