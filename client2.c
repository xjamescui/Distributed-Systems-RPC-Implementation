/*
 * client.c
 *
 * This file is the client program,
 * which prepares the arguments, calls "rpcCall", and checks the returns.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rpc.h"

#define CHAR_ARRAY_LENGTH 100

int main()
{

    /* prepare the arguments for f0 */
    int a0 = 5;
    int b0 = 10;
    int count0 = 3;
    int return0;
    int argTypes0[count0 + 1];
    void **args0;

    argTypes0[0] = (1 << ARG_OUTPUT) | (ARG_INT << 16);
    argTypes0[1] = (1 << ARG_INPUT) | (ARG_INT << 16);
    argTypes0[2] = (1 << ARG_INPUT) | (ARG_INT << 16);
    argTypes0[3] = 0;

    args0 = (void **)malloc(count0 * sizeof(void *));
    args0[0] = (void *)&return0;
    args0[1] = (void *)&a0;
    args0[2] = (void *)&b0;

    /* prepare the arguments for f1 */
    char a1 = 'a';
    short b1 = 100;
    int c1 = 1000;
    long d1 = 10000;
    int count1 = 5;
    long return1;
    int argTypes1[count1 + 1];
    void **args1;

    argTypes1[0] = (1 << ARG_OUTPUT) | (ARG_LONG << 16);
    argTypes1[1] = (1 << ARG_INPUT) | (ARG_CHAR << 16);
    argTypes1[2] = (1 << ARG_INPUT) | (ARG_SHORT << 16);
    argTypes1[3] = (1 << ARG_INPUT) | (ARG_INT << 16);
    argTypes1[4] = (1 << ARG_INPUT) | (ARG_LONG << 16);
    argTypes1[5] = 0;

    args1 = (void **)malloc(count1 * sizeof(void *));
    args1[0] = (void *)&return1;
    args1[1] = (void *)&a1;
    args1[2] = (void *)&b1;
    args1[3] = (void *)&c1;
    args1[4] = (void *)&d1;

    /* rpcCalls */
    int s0 = rpcCacheCall("f0", argTypes0, args0);
    /* test the return f0 */
    printf("\nEXPECTED return of f0 is: %d\n", a0 + b0);
    if (s0 >= 0) {
        printf("ACTUAL return of f0 is: %d\n", *((int *)(args0[0])));
    } else {
        printf("Error: %d\n", s0);
    }

    /* rpcCalls */
    s0 = rpcCacheCall("f0", argTypes0, args0);
    /* test the return f0 */
    printf("\nEXPECTED return of f0 is: %d\n", a0 + b0);
    if (s0 >= 0) {
        printf("ACTUAL return of f0 is: %d\n", *((int *)(args0[0])));
    } else {
        printf("Error: %d\n", s0);
    }

    /* rpcCalls */
    s0 = rpcCacheCall("f0", argTypes0, args0);
    /* test the return f0 */
    printf("\nEXPECTED return of f0 is: %d\n", a0 + b0);
    if (s0 >= 0) {
        printf("ACTUAL return of f0 is: %d\n", *((int *)(args0[0])));
    } else {
        printf("Error: %d\n", s0);
    }

    /* rpcCalls */
    s0 = rpcCacheCall("f0", argTypes0, args0);
    /* test the return f0 */
    printf("\nEXPECTED return of f0 is: %d\n", a0 + b0);
    if (s0 >= 0) {
        printf("ACTUAL return of f0 is: %d\n", *((int *)(args0[0])));
    } else {
        printf("Error: %d\n", s0);
    }

    /* rpcTerminate */
    printf("\ndo you want to terminate? y/n: ");
    if (getchar() == 'y')
        rpcTerminate();

    /* end of client.c */
    return 0;
}




