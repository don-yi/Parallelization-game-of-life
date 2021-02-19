#define HAVE_STRUCT_TIMESPEC

#include <iostream>
#include <utility>
#include <pthread.h>
#include <semaphore.h>

#include "gol.h"

void* thread_fnc(void* args);

typedef std::vector<std::tuple<int, int>> population;
struct args {
  population p;
  int numIter{};
  int maxX{};
  int maxY{};
} args;

// Returns vector of coordinates of the alive cells of the final population.
std::vector<std::tuple<int,int>> run(
  std::vector<std::tuple<int,int>> initial_population,
  int num_iter,
  int max_x,
  int max_y
) {
  // Initializations
  args.p = std::move(initial_population);
  args.numIter = num_iter;
  args.maxX = max_x;
  args.maxY = max_y;

  int const numThread = max_x * max_y;
  pthread_t* threads = new pthread_t [numThread];

  // Creates threads.
  for (int i = 0; i < numThread; ++i) {
    if (pthread_create(&threads[i], nullptr, thread_fnc, &args)) {
      std::cout << "\n ERROR creating thread";
      exit(EXIT_FAILURE);
    }
  }

  // Joins threads.
  void* populationPtrReturn;
  for (int i = 0; i < numThread; ++i) {
    if (pthread_join(threads[i], &populationPtrReturn)) {
      std::cout << "\n ERROR joining thread";
      exit(EXIT_FAILURE);
    }
  }

  // Exits thread and returns.
  pthread_exit(nullptr);
  population res = *static_cast<population*>(populationPtrReturn);
  free(populationPtrReturn);
  return res;
}

// read data from neighbors
// calculate the next state
// wait all threads to finish their calculations
// write the state back to array
// wait all threads to finish their writes
// goto 1st line
void* thread_fnc(void* args) {


  return nullptr;
}
