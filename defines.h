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
 * For database                                                   *
 *                                                                *
 ******************************************************************/
// TODO: maybe refactor these names
// put
#define SIGNATURE_PUT_SUCCESS   0
#define SIGNATURE_PUT_DUPLICATE 1
#define SIGNATURE_PUT_FAIL      -1

// get
#define SIGNATURE_FOUND         0
#define SIGNATURE_NOT_FOUND     -1
#define SIGNATURE_HAS_NO_HOSTS  -2

// delete
#define DELETE_HOST_SUCCESS     0
#define DELETE_HOST_NOT_FOUND   1
#define DELETE_SIG_NOT_FOUND    2


// Skeleton Database
#define RECORD_PUT_SUCCESS      0
#define RECORD_PUT_DUPLICATE    1
#define RECORD_PUT_FAIL         -1
