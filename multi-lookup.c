#include "multi-lookup.h"

/*
 * Main entry point.
 */
int main(int argc, char* argv[])
{
  //Get start time using gettimeofday function.
  struct timeval start, end;
  gettimeofday(&start, NULL);

  // Print usage error if there are too few cmd args.
  if (argc < 5){
    printf("%s\n", "Usage: ./multi-lookup <# requesters> <# resolvers> <requester log> <resolver log> [<data file> ...]");
    exit(1);
  }

  int requesters, resolvers;
  int err;
  int totalFiles = argc - 5;

  // Verify that the user requested a valid integer number of requester threads.
  err = sscanf(argv[1], "%d", &requesters);
  if(err != 1){
    printf("%s\n", "Usage: ./multi-lookup <# requesters> <# resolvers> <requester log> <resolver log> [<data file> ...]");
    exit(1);
  }

  // If the number of requester threads is out of range, print error to stderr and terminate.
  if(requesters < 0 || requesters > MAX_REQUESTER_THREADS){
    fprintf(stderr, "Arguments out of range! There must be no less than 0 and no more than 10 requester threads!\n");
    exit(1);
  }

  // Verify that the user requested a valid integer number of resolver threads.
  err = sscanf(argv[2], "%d", &resolvers);
  if(err != 1){
    printf("%s\n", "Usage: ./multi-lookup <# requesters> <# resolvers> <requester log> <resolver log> [<data file> ...]");
    exit(1);
  }

  // If the number of resolver threads is out of range, print error to stderr and terminate.
  if(resolvers < 0 || resolvers > MAX_REQUESTER_THREADS){
    fprintf(stderr, "Argument out of range! There must be no less than 0 and no more than 10 resolver threads!\n");
    exit(1);
  }

  // Since we were told that handling 0 of either thread type in lecture, I chose to terminate in this scenario, since no useful work can be done.
  if(requesters == 0 || resolvers == 0){
    printf("%s\n", "Since multi-lookup requires at least one requester thread and at least one resolver thread, no useful work can be done. Terminating with exit status 0.");
    exit(0);
  }

  // If too many input files are passed into multi-lookup, print error to stderr and terminate.
  if(totalFiles >= MAX_INPUT_FILES){
    fprintf(stderr, "ERROR: Too many input files were passed into multi-lookup through the command-line terminal!");
    exit(1);
  }
  
  char* requesterLog = argv[3];
  char* resolverLog = argv[4];

  // Generate the list of input data files.
  FileList* inData = create_file_list(totalFiles);
  err = open_files(totalFiles, inData, argv);

  if(err != 0){
    printf("ERROR: Failed to initialize input files struct!\n");
    exit(1);
  }
  
  //Initialize the shared array
  err = init();

  // Verify that the shared array initialized properly.
  if(err != 0){
    printf("%s\n", "ERROR: Failed to initialize the shared array!");
    free(inData);
    exit(1);
  }

  OutFile resultsFile;
  err = open_results(&resultsFile, requesterLog);

  // Verify that the results file mutex lock initialized properly.
  if(err != 0){
    destroy();
    pthread_mutex_destroy(&inData->lock);
    free(inData);
    exit(1);
  }

  // Generate the serviced file struct.
  OutFile servicedFile;
  open_serviced(&servicedFile, resolverLog);

  // Verify that the serviced file mutex lock initialized properly.
  if(err != 0){
    destroy();
    pthread_mutex_destroy(&inData->lock);
    free(inData);
    exit(1);
  }

  pthread_t* reqThreads = malloc(requesters * sizeof(pthread_t));

  // If memory cannot be allocated for requester thread ids, free all allocated memory and exit in error state.
  if(reqThreads == NULL){
    printf("%s\n", "ERROR: Failed to allocate memory for requester threads!");
    destroy();
    pthread_mutex_destroy(&inData->lock);
    pthread_mutex_destroy(&resultsFile.lock);
    pthread_mutex_destroy(&servicedFile.lock);
    free(inData);
    exit(1);
  }

  pthread_t* resThreads = malloc(resolvers * sizeof(pthread_t));

  // If memory cannot be allocated for resolver thread ids, free all allocated memory and exit in error state
  if(resThreads == NULL){
    printf("%s\n", "ERROR: Failed to allocate memory for resolver threads!");
    destroy();
    pthread_mutex_destroy(&inData->lock);
    pthread_mutex_destroy(&resultsFile.lock);
    pthread_mutex_destroy(&servicedFile.lock);
    free(inData);
    free(reqThreads);
    exit(1);
  }

  // Generate args to be passed into requester/resolver threads.
  struct RequesterArgs* reqArgs = malloc(sizeof(inData) + sizeof(resultsFile));
  reqArgs->data = inData;
  reqArgs->results = resultsFile;

  struct ResolverArgs* resArgs = malloc(sizeof(inData) + sizeof(servicedFile));
  resArgs->data = inData;
  resArgs->serviced = servicedFile;
  
  // Generate requester threads and resolver threads.
  generate_requesters(requesters, reqThreads, reqArgs);
  generate_resolvers(resolvers, resThreads, resArgs);
  err = join_threads(requesters, reqThreads);

  // If requester threads could not be joined, free all allocated memory and exit in error state.
  if(err != 0){
    printf("%s\n", "Failed joining the requester threads.  Terminating multi-lookup!");
    destroy();
    pthread_mutex_destroy(&inData->lock);
    pthread_mutex_destroy(&resultsFile.lock);
    pthread_mutex_destroy(&servicedFile.lock);
    free(inData);
    free(reqThreads);
    free(resThreads);
    exit(1);
  }

  err = join_threads(resolvers, resThreads);

  // If resolver threads could not be joined, free all allocated memory and exit in error state.
  if(err != 0){
    printf("%s\n", "Failed joining the resolver threads.  Terminating multi-lookup!");
    destroy();
    pthread_mutex_destroy(&inData->lock);
    pthread_mutex_destroy(&resultsFile.lock);
    pthread_mutex_destroy(&servicedFile.lock);
    free(inData);
    free(reqThreads);
    free(resThreads);
    exit(1);
  }

  // Close serviced/results files.
  fclose(servicedFile.fd);
  fclose(resultsFile.fd);

  // Free the allocated memory for input data files, destroy shared array and destroy mutexes
  pthread_mutex_destroy(&inData->lock);
  pthread_mutex_destroy(&resultsFile.lock);
  pthread_mutex_destroy(&servicedFile.lock);
  for(int i = 0; i < totalFiles; i++){
    pthread_mutex_destroy(&inData->list[i].lock);
    if(inData->list[i].fd != NULL){
      fclose(inData->list[i].fd);
    }
  }
  free(inData);
  free(reqThreads);
  free(resThreads);
  free(reqArgs);
  free(resArgs);
  destroy();

  // Get the end time from gettimeofday function and compute total runtime.
  gettimeofday(&end, NULL);
  double runtime = (double)(end.tv_usec - start.tv_usec)/1000000 + (double)(end.tv_sec - start.tv_sec);

  // Print total runtime to stdout.
  printf("%s%f%s\n", "Total runtime of multi-lookup: ", runtime, " seconds.");
  return 0;
}

// Definition of generate_requesters method of multi-lookup.
int generate_requesters(int numRequesters, pthread_t* tids, struct RequesterArgs* args)
{
  // Generate the specified number of requester threads.
  for(int i = 0; i < numRequesters; i++)
  {
    int err;
    err = pthread_create(&tids[i], NULL, requester, (void *) args);

    // Confirm that there were no issues creating each thread.
    if(err != 0)
    {
      printf("%s%d%s\n", "Something went terribly wrong creating the ", i, "th requester thread. Whoopsie.");
      return -1;
    }
  }

  return 0;
}

// Definition of generate_resolvers method of multi-lookup.
int generate_resolvers(int numResolvers, pthread_t* tids, struct ResolverArgs* args)
{
  // Generate the specified number of resolver threads.
  for(int i=0; i < numResolvers; i++)
  {
    int err;
    err = pthread_create(&tids[i], NULL, resolver, (void *) args);

    // Confirm that there were no issues creating each thread.
    if(err != 0)
    {
      printf("%s%d%s\n", "Something went terribly wrong creating the ", i, "th resolver thread. Whoopsie.");
      return -1;
    }
  }

  return 0;
}

// Definition of join_threads method of multi-lookup.
int join_threads(int numThreads, pthread_t* tids){

  // Attempt to join the specified number of threads with given tids.
  for(int i = 0; i < numThreads; i++){
    int err;
    err = pthread_join(tids[i], 0);

    // Confirm that there were no issues joining each thread.
    if (err != 0){
      printf("%s%li%s\n", "Failed operation to join thread number ", tids[i], ".");
      return -1;
    }
  }

  return 0;
}

// Definition of requester thread.
void* requester(void* args)
{
  int filesProcessed = 0;
  struct RequesterArgs* reqArgs = (struct RequesterArgs *) args;
  FileList* files = reqArgs->data;
  char* hostname;
  hostname = (char *) malloc(sizeof(char) * MAX_NAME_LENGTH);
  char newline[2] = "\n\0";

  // Exit thread if memory failed to allocate for the hostnames.
  if(hostname == NULL){
    printf("%s%lu%s\n", "ERROR: Failed to allocate memory for hostname in thread: ", pthread_self(), "!");
    pthread_exit(PTHREAD_CANCELED);
  }

  while(1)
  {
    FILE* fd;
    pthread_t tid = pthread_self();

    // Lock file list
    pthread_mutex_lock(&files->lock);

    // Find the next valid input data file, if there are none, terminate thread, and print the number of files processed.
    if(files->current == files->total){
      pthread_mutex_unlock(&files->lock);
      printf("%s%lu%s%d%s\n", "Thread ", tid, " serviced ", filesProcessed, " files.");
      break;
    }else{
      fd = files->list[files->current].fd;
      files->current++;
      
      pthread_mutex_unlock(&files->lock);
    }

    while(fd != NULL)
    {
      // Retrieve the next hostname from input file, if fgets returns null, exit the loop and find the next input file.
      char* read = fgets(hostname, MAX_NAME_LENGTH, fd);
      if(read == NULL)
      {
	files->processed++;
	filesProcessed++;
	break;
      }

      //Place hostname into shared array.
      ts_write(hostname);

      strcat(hostname, newline);
      // Write hostname to results file
      pthread_mutex_lock(&reqArgs->results.lock);
      fputs(hostname, reqArgs->results.fd);
      pthread_mutex_unlock(&reqArgs->results.lock);
    }   
  }
  free(hostname);
  return 0;
}

// Definition of resolver thread.
void* resolver(void* args)
{
  int numHostnames = 0;
  struct ResolverArgs* resArgs = (struct ResolverArgs *) args;
  char* hostname = malloc(sizeof(char) * MAX_NAME_LENGTH);
  memset(hostname, '\0', MAX_NAME_LENGTH);

  while(1)
  {
    if(resArgs->data->processed == resArgs->data->total && get_num_elements() == 0){
      break;
    }
    int err;
    
    ts_read(hostname);

    // Create string for the hostname, separator string, and ip address (or NOT_RESOLVED).
    char temp[MAX_NAME_LENGTH+INET6_ADDRSTRLEN+2];
    char seperator[3] = ", \0";
    char ip[INET6_ADDRSTRLEN+1];
    char endline[2] = "\n\0";
    memset(ip, '\0', sizeof(INET6_ADDRSTRLEN));
    memset(temp, '\0', sizeof(temp));

    // Resolve hostname
    err = dnslookup(hostname, ip, INET6_ADDRSTRLEN);

    // Build resolution string
    strcat(temp, hostname);
    strcat(temp, seperator);

    // If dnslookup succeeded, append returned ip to temp string.  Otherwise append NOT_RESOLVED.
    if(err == UTIL_SUCCESS)
    {
      strcat(temp, ip);
      strcat(temp, endline);
      numHostnames++;
    }else
    {
      char errorStr[14] = "NOT_RESOLVED\n\0";
      strcat(temp, errorStr);
    }

    // Print temp string to serviced file.
    pthread_mutex_lock(&resArgs->serviced.lock);
    fputs(temp, resArgs->serviced.fd);
    pthread_mutex_unlock(&resArgs->serviced.lock);
  
  }

  printf("%s%lu%s%d%s\n", "Thread ", pthread_self(), " resolved ", numHostnames, " hostnames.");
  free(hostname);
  return 0;
}
