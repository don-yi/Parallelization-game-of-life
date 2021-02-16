#include <pthread.h>
#include <iostream>

#include "gol.h"

// return vector of coordinates of the alive cells of the final population
std::vector<std::tuple<int,int>> run(
  std::vector<std::tuple<int,int>> initial_population,
  int num_iter,
  int max_x,
  int max_y
) {
  int const numThr = max_x * max_y;
  pthread_t* threads = new pthread_t [numThr];
  pthread_attr_t attr;

  // init and set thr joinable
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

  for (int i = 0; i < numThr; ++i) {
    //int rc = pthread_create(&threads[i], &attr, , (void*));
    //if (rc) {
    //  std::cout << "Err: unable to create thread, " << rc << std::endl;
    //  exit(-1);
    //}
  }

  pthread_attr_destroy(&attr);
  for (int i = 0; i < numThr; ++i)
  {
    void* status;
    int rc = pthread_join(threads[i], &status);
    if (rc) {
      std::cout << "Err: unable to join, " << rc << std::endl;
      exit(-1);
    }
  }

  pthread_exit(NULL);

  return {};
}

