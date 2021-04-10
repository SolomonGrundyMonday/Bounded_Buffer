/*
 *  CSCI-3753 Design and Analysis of Operating Systems, PA3 Bounded Buffer Problem - input_processor
 *  header file.  
 *  Created by Jeff Colgan; April 4, 2021.
 * 
 *  This header file's name is a bit of a misnomer, since it deals with both input and output, and it 
 *  doesn't really process either.  In this header file, I simply defined a bunch of structs and methods
 *  to deal with the various input and output files that are used by my implementation of multi-lookup.
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

typedef struct OutFile{
  pthread_mutex_t lock;
  FILE* fd;
  char* name;
} OutFile;

typedef struct Input{
  pthread_mutex_t lock;
  FILE* fd;
  char* name;
  int complete;
} Input;

typedef struct FileList{
  pthread_mutex_t lock;
  int current;
  int total;
  int processed;
  Input list[];
} FileList;

struct RequesterArgs{
  FileList* data;
  OutFile results;
};

struct ResolverArgs{
  FileList* data;
  OutFile serviced;
};

/*
 *  Prototype of create_file_list method. 
 *  This method allocates memory for the list of input data files, and initializes a FileList struct with
 *  the appropriate values for total files to be processed, current file, and number of files that have already been
 *  processed.  The mutex lock protecting this struct is also initialized here.
 *  Params:  the number of input files entered in the command line by the user.
 */
FileList* create_file_list(int total);

/*
 *  Prototype of open_files method.
 *  This method loops through all of the given input data files, opens them with fopen and initializes an Input struct
 *  with the appropriate values.  The files are added to the FileList struct, and each input file's mutex lock is initialized.
 *  Params:  total number of input files, list of input files struct used by multi-lookup, list of filenames to be opened provided
 *  by the userr.
 */
int open_files(int total, FileList* list, char* files[]);

/*
 *  Prototype of open_results method.
 *  This method opens the provided results file to be written to by requester threads, initializes the members of the OutFile struct
 *  with the appropriate values, and initializes the mutex lock for the requester log.
 *  Params:  the struct repersenting the results file, the name of the file provided by the user.
 */
int open_results(OutFile* results, char* log);

/*
 *  Prototype of open_serviced method.
 *  This method opens the provided serviced file to be written to by resolver threads, initializes the members of the OutFile struct
 *  with the appropriate values, and initializes the mutex lock for the resolver log.
 *  Params:  the struct representing the serviced file, the name of the file provided by the user.
 */
int open_serviced(OutFile* serviced, char* log);


