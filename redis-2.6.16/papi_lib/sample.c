#include <stdio.h>
#include <stdlib.h>
//#include "papi.h" /* This needs to be included every time you use PAPI */
#include "papi.h" /* This needs to be included every time you use PAPI */

#define NUM_EVENTS 1
#define ERROR_RETURN(retval) { fprintf(stderr, "Error %d %s:line %d: \n", retval,__FILE__,__LINE__);  exit(retval); }

double ** alloc_matrix(int size);
void free_matrix(double ** M, int size);
void matrix_rand_init(double ** M, int size);
/*void print_matrix(double **M, int size);*/


int main(int argc, char** argv){
  int retval;

  /*must be initialized to PAPI_NULL before calling PAPI_create_event*/
  int EventSet = PAPI_NULL;

  char errstring[PAPI_MAX_STR_LEN];
  long long values[NUM_EVENTS];

  if (argc != 2) {
    fprintf(stdout,"Usage: %s #size\n",argv[0]);
    return 1;
  }

  int size = atoi(argv[1]);

  /* allocate matrices spaces */
  double ** A = alloc_matrix(size);
  double ** B = alloc_matrix(size);
  double ** C = alloc_matrix(size);

  /* initialize matrices */
  matrix_rand_init(A, size);
  matrix_rand_init(B, size);

  int i,j,k;

  /***************************************************************************
   * This part initializes the library and compares the version number of the *
   * header file, to the version of the library, if these don't match then it *
   * is likely that PAPI won't work correctly.If there is an error, retval    *
   * keeps track of the version number.                                       *
   ****************************************************************************/

  if((retval = PAPI_library_init(PAPI_VER_CURRENT)) != PAPI_VER_CURRENT )
  {
    fprintf(stderr, "Error: %s\n", errstring);
    exit(1);
  }

  /* Creating event set   */
  if ((retval=PAPI_create_eventset(&EventSet)) != PAPI_OK)
    ERROR_RETURN(retval);

  int EVENT = PAPI_L3_DCM;

  /* Add the array of events PAPI_TOT_INS and PAPI_TOT_CYC to the eventset*/
  if ((retval=PAPI_add_event(EventSet, EVENT)) != PAPI_OK)
    ERROR_RETURN(retval);

  /* Start counting */
  if ( (retval=PAPI_start(EventSet)) != PAPI_OK)
    ERROR_RETURN(retval);



  /****** this is where your computation goes *********/
  for (i=0; i<size; i++) {
    for (j=0; j<size; j++) {
      for (k=0; k<size; k++) {
  /* here please do your computation */
      }
    }
  }




  /* Stop counting, this reads from the counter as well as stop it. */
  if ( (retval=PAPI_stop(EventSet,values)) != PAPI_OK)
    ERROR_RETURN(retval);

  double tot_access = 4 * size * size *size;
  double miss_ratio = values[0] / tot_access;
  printf("%d\t%f\n",size,miss_ratio);

  if ( (retval=PAPI_remove_event(EventSet, EVENT))!=PAPI_OK)
    ERROR_RETURN(retval);

  /* Free all memory and data structures, EventSet must be empty. */
  if ( (retval=PAPI_destroy_eventset(&EventSet)) != PAPI_OK)
    ERROR_RETURN(retval);

  /* free the resources used by PAPI */
  PAPI_shutdown();

  /* free matrices */
  free_matrix(A,size);
  free_matrix(B,size);
  free_matrix(C,size);

  exit(0);
}

double ** alloc_matrix(int size) {
  int i;
  double ** M = malloc(size * sizeof(double *));
  for (i=0; i<size; i++) {
    M[i] = malloc(size * sizeof(double));
  }
  return M;
}

void free_matrix(double ** M, int size) {
  int i;
  for (i=0; i<size; i++) {
    free(M[i]);
  }
  free(M);
}

void matrix_rand_init(double ** M, int size) {
  int i,j;
  for (i=0; i<size; i++) {
    for (j=0; j<size; j++) {
      M[i][j] = rand()/123.0;
    }
  }
}

void print_matrix(double **M, int size) {
  int i, j;
  for (i=0; i<size; i++) {
    for (j=0; j<size; j++)
      printf("%f\t", M[i][j]);
    printf("\n");
  }
}
