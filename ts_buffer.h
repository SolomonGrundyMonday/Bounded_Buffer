/*
 *  Thread-safe bounded buffer header file.  CSCI-3753 PA3 Bounded Buffer Solution.
 *  
 *  Created by Jeff Colgan; March 23, 2021.
 */

#include <semaphore.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ARRAY_SIZE 10
#define MAX_NAME_LENGTH 255

unsigned int urls;
char *buffer[MAX_ARRAY_SIZE];
pthread_cond_t readBlock;
pthread_cond_t writeBlock;
pthread_mutex_t mutex;

/*
 *  This method initializes the shared array and allocates the necessary memory.  It must be successfully
 *  called before any requester or resolver threads can be generated.
 *  Returns 0 on success and nonzero on failure.
 */
int init();

/*
 *  This method provides synchronized access to the shared array to resolver threads, which take one url from
 *  the shared resource, consuming the values in the array.
 *  If the array is empty, resolver threads should block until there is at least one piece of data in the array to be
 *  consumed.
 *  Params: the variable to be written to by the shared array.
 *  Returns 0 success.
 */
int ts_read(char* hostname);

/*
 *  This method provides synchronized access to the shared array to requester threads, which produce values to
 *  be placed on the array.
 *  If the array is full, requester threads should block until the array has room for at least one piece of produced
 *  data to be placed in the array.
 *  Params: the data to be placed into the shared array.
 *  Returns 0 on success, nonzero on failure.
 */
int ts_write(char* data);

/*
 *  This is a simple getter method, which allows requester/resolver threads to get the number of hostnames currently stored
 *  in the shared array.
 */
int get_num_elements();

/*
 *  This method frees all system resources utilized by the ts_buffer.  Must be called before program termination to avoid
 *  memory leaks.
 *  Returns 0 upon success, nonzero on failure.
 */
int destroy();
