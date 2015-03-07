/******************************************************************
 * this file contains general #define                             *
 *                                                                *
 *                                                                *
 *******************************************************************/



/******************************************************************
 * message types                                                  *
 *                                                                *
 ******************************************************************/
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

#define FUNCTION_NAME_SIZE      80
