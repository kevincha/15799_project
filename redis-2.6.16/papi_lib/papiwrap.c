#include "papiwrap.h"

void initialize() {
  int retval;
  char errstring[PAPI_MAX_STR_LEN];

  /*retval = PAPI_library_init(PAPI_VER_CURRENT);
  if (retval != PAPI_VER_CURRENT){
    PAPI_perror(retval,errstring,PAPI_MAX_STR_LEN);
    printf("%s:%d::PAPI_library_init failed. %d %s\n",
           __FILE__,__LINE__,retval,errstring);
    exit(1);
  }*/

  if((retval = PAPI_library_init(PAPI_VER_CURRENT)) != PAPI_VER_CURRENT )
  {
    fprintf(stderr, "Error: %s\n", errstring);
    exit(1);
  }


  if (PAPI_set_debug(PAPI_VERB_ECONT) != PAPI_OK) {
    exit(1);
  }

  /*retval = PAPI_thread_init((unsigned long (*)(void))(omp_get_thread_num));
  if (retval != PAPI_OK){
    PAPI_perror(retval,errstring,PAPI_MAX_STR_LEN);
    printf("%s:%d::PAPI_thread_init failed. %d %s\n",
           __FILE__,__LINE__,retval,errstring);
    exit(1);
  }*/
}

events* start_events() {
  int retval;
  events *evts = calloc(1, sizeof(events));
  evts->EventSet = PAPI_NULL;

  retval = PAPI_query_event(PAPI_MEM_SCY);
  if (retval != PAPI_OK) {
    printf("PAPI_MEM_SCY not available\n");
  }
  retval = PAPI_query_event(PAPI_L2_DCM);
  if (retval != PAPI_OK) {
    printf("PAPI_L2_DCM not available\n");
  }
  retval = PAPI_query_event(PAPI_LD_INS);
  if (retval != PAPI_OK) {
    printf("PAPI_LD_INS not available\n");
  }

  if ((retval=PAPI_create_eventset(&evts->EventSet)) != PAPI_OK)
    ERROR_RETURN(retval);
  //int events[NUM_EVENTS] = {PAPI_L1_DCM, PAPI_L2_DCM, PAPI_LD_INS};
  if ((retval=PAPI_add_events(evts->EventSet, EVENTS, NUM_EVENTS)) != PAPI_OK)
    ERROR_RETURN(retval);
  if ( (retval=PAPI_start(evts->EventSet)) != PAPI_OK)
    ERROR_RETURN(retval);

  return evts;
}

void stop_events(events* evts) {
  int retval;

  if ( (retval=PAPI_stop(evts->EventSet, evts->values)) != PAPI_OK)
    ERROR_RETURN(retval);
  if ( (retval=PAPI_remove_events(evts->EventSet, EVENTS, NUM_EVENTS))!=PAPI_OK)
    ERROR_RETURN(retval);
  if ( (retval=PAPI_destroy_eventset(&evts->EventSet)) != PAPI_OK)
    ERROR_RETURN(retval);
}
