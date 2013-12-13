#include "papiwrap.h"

/* GLOBAL VARIABLES */
events *global_papi_evts;
long long total_event_val[NUM_EVENTS];
long long dict_set_event_val[NUM_EVENTS];
long long dict_get_event_val[NUM_EVENTS];

void papiInitialize() {
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

    fd_log = fopen("papi.log", "w");
}

void papiCreateEvents(events **pp_evts) {
    int retval;
    if (pp_evts == NULL)
        ERROR_RETURN(-1);

    // Initialize events
    if (*pp_evts == NULL)
        *pp_evts = calloc(1, sizeof(events));
    else
        memset(*pp_evts, 0, sizeof(events));
    (*pp_evts)->EventSet = PAPI_NULL;
    if ((retval=PAPI_create_eventset(&((*pp_evts)->EventSet))) != PAPI_OK)
        ERROR_RETURN(retval);
    if ((retval=PAPI_add_events((*pp_evts)->EventSet, all_events, NUM_EVENTS)) != PAPI_OK)
        ERROR_RETURN(retval);
}

void papiStartEvents(events *evts) {
    int retval;

    if (evts == NULL)
    {
        ERROR_RETURN(-1);
    }
    else
    {
        memset(evts, 0, sizeof(events));
    }

    if ((retval=PAPI_start(evts->EventSet)) != PAPI_OK)
        ERROR_RETURN(retval);
}

void papiStopEvents(events* evts) {
    int retval;

    if (evts == NULL)
        ERROR_RETURN(-1);
    if ((retval=PAPI_stop(evts->EventSet, evts->values)) != PAPI_OK)
        ERROR_RETURN(retval);
}

void papiAccumValues(events *evts, long long *values) {
    int i = 0;
    for (; i < NUM_EVENTS; i++)
        values[i] += evts->values[i];
}

void papiClearEvents(events* evts) {
    int retval;

    if (evts == NULL)
        ERROR_RETURN(-1);
    if ( (retval=PAPI_stop(evts->EventSet, evts->values)) != PAPI_OK)
        ERROR_RETURN(retval);
    if ( (retval=PAPI_remove_events(evts->EventSet, all_events, NUM_EVENTS))!=PAPI_OK)
        ERROR_RETURN(retval);
    if ( (retval=PAPI_destroy_eventset(&evts->EventSet)) != PAPI_OK)
        ERROR_RETURN(retval);
}
