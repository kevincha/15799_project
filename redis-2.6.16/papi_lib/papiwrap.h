#ifndef _PAPI_WRAP_H
#define _PAPI_WRAP_H

#include "papi.h"
#include <stdio.h>
#include <stdlib.h>

#define NUM_EVENTS 8
static int EVENTS[NUM_EVENTS] = {PAPI_TOT_CYC, PAPI_TOT_INS, PAPI_L1_DCM, PAPI_L2_DCA, PAPI_L2_DCH, PAPI_L2_DCM, PAPI_L2_DCR, PAPI_L2_DCW};

typedef struct events_struct {
  int EventSet;
  long long values[NUM_EVENTS];
} events;

//thanks to http://www.cs.utexas.edu/~pingali/CSE392/2011sp/hw2/
#define ERROR_RETURN(retval) { fprintf(stderr, "Error %d %s:line %d: \n", retval,__FILE__,__LINE__);  exit(retval); }

/* Invoke from master thread */
void initialize();
/* Call before the section to start collecting counters */
events* start_events();
/* Call to finish collecting counters */
void stop_events(events* evts);

#endif
