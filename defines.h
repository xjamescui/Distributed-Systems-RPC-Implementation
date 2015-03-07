/******************************************************************
 * this file contains general #define                             *
 *                                                                *
 *                                                                *
 *******************************************************************/



/******************************************************************
 * message types                                                  *
 *                                                                *
 ******************************************************************/
#define MSG_REGISTER            0b00000101
#define MSG_REGISTER_SUCCESS    0b00000110
#define MSG_REGISTER_FAILURE    0b00000111

#define MSG_LOC_REQUEST         0b00001001
#define MSG_LOC_SUCCESS         0b00001010
#define MSG_LOC_FAILURE         0b00001011

#define MSG_EXECUTE             0b00010001
#define MSG_EXECUTE_SUCCESS     0b00010010
#define MSG_EXECUTE_FAILURE     0b00010011

#define MSG_TERMINATE           0b10000000

#define MAX_RW_CHUNK_SIZE       16348

#define FUNCTION_NAME_SIZE     100
