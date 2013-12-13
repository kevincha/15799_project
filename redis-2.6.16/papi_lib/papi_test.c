#include "papiwrap.h"

//size of array to do act on
#define SIZE 1000

void Work(int tid, int arr[][SIZE]);

int main(int argc, char **argv) {
  int i, j, max_threads;
  int arr[SIZE][SIZE];
  events* evts;

  //Set up PAPI
  initialize();

  for (i = 0; i < SIZE; ++i) {
    for (j = 0; j < SIZE; ++j) {
      arr[i][j] = i + j;
    }
  }

  Work(0, arr);

  return 0;
}

/**
 * Do work on arr by summing up its elements
 * (all elements if an even thread index,
 *  or 1/4 of the elements if an odd thread index)
 */
void Work(int tid, int arr[][SIZE]) {
  int i, j;
  long long sum;
  events* evts;

  //start collecting for non-master
  evts = start_events();

  if (tid % 2 == 0) {
    for (i = 0; i < SIZE; ++i) {
      for (j = 0; j < SIZE; ++j) {
        sum += arr[i][j];
      }
    }
  } else {
    for (i = 0; i < SIZE / 2; ++i) {
      for (j = 0; j < SIZE / 2; ++j) {
        sum += arr[i][j];
      }
    }
  }

  //print summary for non-master
  stop_events(evts);
  printf("Thread %2d got %Ld and had %Ld L1 misses, %Ld L2 misses, %Ld loads\n",
      tid, sum, evts->values[0], evts->values[1], evts->values[2]);
}
