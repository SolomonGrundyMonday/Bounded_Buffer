/*
 *  CSCI-3753 Design and Analyis of Operating Systems, PA3: implementation of ts_buffer.
 *  Created by Jeff Colgan; March 28, 2021.
 *
 *  This file implements the methods defined in the header file "ts_array.h", which
 *  defines the methods of a thread-safe bounded buffer: init, read, write, destroy.
 */

#include "ts_buffer.h"

// Definition of init method for ts_array.
int init()
{
  urls = 0;

  // Allocate memory for each element of the buffer
  for(int i=0; i<MAX_ARRAY_SIZE;i++)
  {
    buffer[i] = malloc(MAX_NAME_LENGTH * sizeof(char));

    // If memory cannot be allocated for any string in buffer, return error state.
    if (buffer[i] == NULL)
    {
      return -1;
    }
  }

  // Initialize mutex, readBlock, writeBlock semaphores.
  int err = pthread_mutex_init(&mutex, NULL);
  readBlock = (pthread_cond_t) PTHREAD_COND_INITIALIZER;
  writeBlock = (pthread_cond_t) PTHREAD_COND_INITIALIZER;

  // If any of the semaphores cannot be initialized, return error state.
  if(err != 0)
  {
    for (int i = 0; i < MAX_ARRAY_SIZE; i++){
      free(buffer[i]);
    }
    return -1;
  }else
  {
    return 0;
  }
  
}

// Definition for read method of ts_array.
int ts_read(char* hostname)
{

  // If the array is empty, block on readBlock semaphore.
  pthread_mutex_lock(&mutex);
  while(urls == 0)
  {
    pthread_cond_wait(&readBlock, &mutex);
  }

  
  strcpy(hostname, buffer[urls-1]);
  memset(buffer[urls-1], '\0', MAX_NAME_LENGTH);\
  urls--;

  if(urls == MAX_ARRAY_SIZE-1){
    pthread_cond_broadcast(&writeBlock);
  }

  // Unlock the mutex when the hostname has been removed and urls decremented.
  pthread_mutex_unlock(&mutex);
  
  return 0;
}

// Definition for write method of ts_array.
int ts_write(char* data)
{
  // Strip newline character from the input data.
  char *newline;
  newline = strchr(data, '\n');
  if(newline != NULL)
  {
    *newline = '\0';
  }

  pthread_mutex_lock(&mutex);
  // If the array is full, block on writeBlock semaphore.
  while(urls == MAX_ARRAY_SIZE){
    pthread_cond_wait(&writeBlock, &mutex);
  }

  
  strcpy(buffer[urls], data);
  urls++;

  if(urls - 1 == 0){
    pthread_cond_broadcast(&readBlock);
  }
  
  // Unlock the mutex lock when the hostname has been written to the shared array and urls incremented.
  pthread_mutex_unlock(&mutex);  
  
  return 0;
}

// Definition of get_num_elements.  Pretty self-evident what this method does.
int get_num_elements()
{
  return urls;
}

// Definition of destroy method of ts_array.
int destroy()
{
  // Free resources allocated to the bounded buffer.
  for(int i=0;i<MAX_ARRAY_SIZE;i++)
  {
    free(buffer[i]);
  }

  // Destroy the ts_array semaphores.
  pthread_mutex_destroy(&mutex);
  pthread_cond_destroy(&readBlock);
  pthread_cond_destroy(&writeBlock);
  return 0;
}
