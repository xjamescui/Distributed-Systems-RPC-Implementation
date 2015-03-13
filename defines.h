/******************************************************************
 * this file contains general #define                             *
 *                                                                *
 *                                                                *
 *******************************************************************/



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

#define MSG_TERMINATE           0x70

#define MAX_RW_CHUNK_SIZE       16348

/******************************************************************
 * error codes                                                    *
 *                                                                *
 ******************************************************************/
#define MSG_REGISTER_SUCCESS_NO_ERRORS              0

#define MSG_REGISTER_SUCCESS_OVERRIDE_PREVIOUS      1

#define MSG_REGISTER_FAILURE_INVALID_SERVER_PORT    -1
#define MSG_REGISTER_FAILURE_INVALID_SERVER_IP      -2
#define MSG_REGISTER_FAILURE_INVALID_FCT_NAME       -3
#define MSG_REGISTER_FAILURE_INVALID_ARGTYPES       -4

#define MSG_LOC_FAILURE_SIGNATURE_NOT_FOUND         -1
#define MSG_LOC_FAILURE_SIGNATURE_NO_HOSTS          -2




/******************************************************************
 * For binder database                                            *
 *                                                                *
 ******************************************************************/
// put
#define BINDER_DB_PUT_SIGNATURE_SUCCESS         0
#define BINDER_DB_PUT_SIGNATURE_DUPLICATE       1
#define BINDER_DB_PUT_SIGNATURE_FAIL            -1

// get
#define BINDER_DB_GET_SIGNATURE_FOUND           0
#define BINDER_DB_GET_SIGNATURE_NOT_FOUND       -1
#define BINDER_DB_GET_SIGNATURE_HAS_NO_HOSTS    -2

// delete
#define BINDER_DB_DELETE_HOST_SUCCESS           0
#define BINDER_DB_DELETE_HOST_NOT_FOUND         1
#define BINDER_DB_DELETE_SIGNATURE_NOT_FOUND    2

