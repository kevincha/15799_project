#ifndef _PAPIWRAP_H
#define _PAPIWRAP_H

#include "papi.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

FILE *fd_log;

#define NUM_EVENTS 3
#define log_papi(M, ...) fprintf(fd_log, "[INFO] (%s:%d) " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)

//static int all_events[NUM_EVENTS] = {PAPI_TOT_CYC, PAPI_TOT_INS, PAPI_L1_DCM, PAPI_L2_DCA, PAPI_L2_DCH, PAPI_L2_DCM, PAPI_L2_DCR, PAPI_L2_DCW};
//static int all_events[NUM_EVENTS] = {PAPI_TOT_CYC, PAPI_TOT_INS, PAPI_TOT_IIS, PAPI_L1_DCM};
///static int all_events[NUM_EVENTS] = {PAPI_TOT_CYC, PAPI_TOT_INS, PAPI_TOT_IIS, PAPI_TLB_DM};
static int all_events[NUM_EVENTS] = {PAPI_TOT_CYC, PAPI_TOT_INS, PAPI_TLB_DM};
// Realize that we cannot monitor more than a few events... EX: L2 DCM cannot mix with others...hmm...

typedef struct events_struct {
  int EventSet;
  long long values[NUM_EVENTS];
} events;

//thanks to http://www.cs.utexas.edu/~pingali/CSE392/2011sp/hw2/
#define ERROR_RETURN(retval) { fprintf(stderr, "Error %d %s:line %d: \n", retval,__FILE__,__LINE__);  exit(retval); }

/* Prototypes */
void papiInitialize();
void papiCreateEvents(events **pp_evts);
void papiStartEvents(events *evts);
void papiStopEvents(events* evts);
void papiClearEvents(events* evts);
void papiAccumValues(events *evts, long long *values);

#endif
