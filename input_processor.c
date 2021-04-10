/*
 *  CSCI-3753 Design and Analysis of Operating Systems PA3 Bounded Buffer problem - input_processor definition.
 *  Created by Jeff Colgan; April 4, 2021.
 */

#include "input_processor.h"

// Definition of create_file_list method.
FileList* create_file_list(int total){
  FileList* list;

  // Allocate memory for the struct, and initialize the mutex lock and member variables.
  list = malloc(sizeof(*list) + sizeof(Input) * total);
  int err = pthread_mutex_init(&list->lock, NULL);
  list->total = total;
  list->current = 0;
  list->processed = 0;

  // Check that malloc returned a valid pointer.
  if(list == NULL){
    printf("%s\n", "ERROR: Failed to allocate memory for FileList structure!");
    exit(1);
  }

  // Check that the data file list mutex lock initialized properly.
  if(err != 0){
    printf("%s\n", "ERROR: Failed to initialize FileList mutex!");
    free(list);
    exit(1);
  }

  return list;
}

// Definition of open_files method.
int open_files(int total, FileList* list, char* files[]){

  int err;
  // Get the data files from argv.
  for(int i = 0; i < total; i++){
    Input file;
    err = pthread_mutex_init(&file.lock, NULL);
    file.complete = 0;
    file.name = files[i+5];
    file.fd = fopen(file.name, "r");
    list->list[i] = file;

    // Verify that the data files' mutex locks initialized properly.
    if(err != 0){
      printf("%s%d%s\n", "ERROR: Failed to initialize mutex for ", i, "th data file!");
      free(list);
      return -1;
    }
  }

  return 0;
}

// Definition of open_results method.
int open_results(OutFile* results, char* log){

  // Populate struct values for the results file to be written to by requester threads.
  int err = pthread_mutex_init(&results->lock, NULL);
  results->name = log;
  results->fd = fopen(log, "w");

  // Return error state if the mutex lock failed to initialize.
  if(err != 0){
    printf("Error: Failed to initialize results output file mutex!\n");
    return -1;
  }

  // Return error state and destroy mutex lock if the results log file failed to open.
  if(results->fd == NULL){
    printf("Error: Failed to open results output file!\n");
    pthread_mutex_destroy(&results->lock);
    return -1;
  }
  
  return 0;
}

// Definition of open_serviced method.
int open_serviced(OutFile* serviced, char* log){

  // Populate stuct values for the serviced file to be written to by requester theads.
  int err = pthread_mutex_init(&serviced->lock, NULL);
  serviced->name = log;
  serviced->fd = fopen(log, "w");

  // Return error state if the mutex lock failed to initialize.
  if(err != 0){
    printf("Error: Failed to initialize serviced output file mutex!\n");
    return -1;
  }

  // Return error state and destroy mutex lock if the serviced log file failed to open.
  if(serviced->fd == NULL){
    printf("Error: Failed to open serviced output file!\n");
    pthread_mutex_destroy(&serviced->lock);
    return -1;
  }

  return 0;
}
