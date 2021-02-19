#define HAVE_STRUCT_TIMESPEC

#include <iostream>
#include <utility>
#include <pthread.h>

#include "gol.h"

// fwd decl
void* ThrFn(void* args);

// thr arg struct
typedef std::vector<std::tuple<int, int>> population;
typedef struct args { population p; int numIter{}; int maxX{}; int maxY{}; }
  args;
args passedArg;

// init thr cond, lock & ct
pthread_cond_t cond1 = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond2 = PTHREAD_COND_INITIALIZER;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
int ct = 0;

// Returns vector of coordinates of the alive cells of the final population.
std::vector<std::tuple<int,int>> run(
  std::vector<std::tuple<int,int>> initial_population,
  int num_iter, int max_x, int max_y
) {
  // arg init
  passedArg.p = std::move(initial_population);
  passedArg.numIter = num_iter; passedArg.maxX = max_x; passedArg.maxY = max_y;

  // thr init
  int const numThread = max_x * max_y;
  pthread_t* thrs = new pthread_t [numThread];

  // create thr
  for (int i = 0; i < numThread; ++i)
    pthread_create(&thrs[i], nullptr, ThrFn, &passedArg);

  // join thr
  for (int i = 0; i < numThread; ++i) pthread_join(thrs[i], nullptr);

  return passedArg.p;
}

void* ThrFn(void* passedArg) {

  // read data from neighbors
  // -----------------------------------

  // wait all threads to finish their calculations
  // -----------------------------------
  // acquire lock
  pthread_mutex_lock(&lock);

  // get num thr
  args* castedArgPtr = static_cast<args*>(passedArg);
  int const numThr = castedArgPtr->maxX * castedArgPtr->maxY;
  if (ct++ < numThr) {
    // calculate the next state
    // -----------------------------------

    // w8 on cond
    pthread_cond_wait(&cond1, &lock);
  } else {
    // init back
    ct = 0;
    pthread_cond_broadcast(&cond1);
  }

  // release lock
  pthread_mutex_unlock(&lock);

  // TODO: maybe fn
  // wait all threads to finish their writes
  // -----------------------------------
  // acquire lock
  pthread_mutex_lock(&lock);

  if (ct++ != numThr) {
    // write the state back to array
    // -----------------------------------

    // w8 on cond
    pthread_cond_wait(&cond2, &lock);
  } else {
    pthread_cond_signal(&cond2);
  }

  // release lock
  pthread_mutex_unlock(&lock);

  // goto 1st line
  // -----------------------------------

  return nullptr;
}
