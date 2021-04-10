/*
 *  CSCI-3753 Design and Analysis of Operating Systems, PA3 multi-lookup header file
 *  Created by Jeff Colgan; March 26, 2021.
 */

#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include "util.h"
#include "ts_buffer.h"
#include "input_processor.h"

#define MAX_INPUT_FILES 100
#define MAX_REQUESTER_THREADS 10
#define MAX_RESOLVER_THREADS 10
#define MAX_NAME_LENGTH 255
#define MAX_IP_LENGTH INET6_ADDRSTRLEN



int main(int argc, char* argv[]);
/*
 *  Method to generate the requester threads.
 *  Params: the specified number of requester threads
 *  Returns: number of threads generated.
 */
int generate_requesters(int numRequesters, pthread_t* tids, struct RequesterArgs* args);

/*
 *  Method to join a number of threads given an array of pointers to their tids.
 */
int join_threads(int numThreads, pthread_t* tids);

/*
 *  Method to generate the resolver threads.
 *  Params: the specified number of resolver threads.
 *  Returns: number of threads generated.
 */
int generate_resolvers(int numResolvers, pthread_t* tids, struct ResolverArgs* args);

/*
 *  Method for requester threads.  This method does the work of requester threads.
 *  Each requester tread will grab the next available input file and read the hostnames from
 *  the file, placing them on the shared array.  When every hostname has been read from the file
 *  the process will grab the next input file - if there are no more unprocessed input files, the
 *  thread terminates.
 */
void* requester(void *args);

/*
 *  Method for resolver threads.  This method does the work of resolver threads.  
 *  Each resolver thread will read a hostname from the shared array and attempt to resolve the
 *  hostname to an ip address, using the provided dnslookup method in util.c.  The thread will write
 *  the hostname and ip address (or the string NOT RESOLVED) to the serviced.txt file.  When the shared
 *  array is empty and all requester threads have terminated, the resolver threads will terminate.
 */
void* resolver(void *args);







