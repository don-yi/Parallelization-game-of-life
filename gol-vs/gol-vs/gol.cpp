#define HAVE_STRUCT_TIMESPEC

#include <pthread.h>
#include <vector>
#include <semaphore.h>
#include "gol.h"

using Col   = std::vector<int>;
using Cells = std::vector<Col>;

// thr arg struct
struct thrData {
  pthread_t thr;
  int numIter = -1, row = -1, col = -1;
  //thrData(int numIter, int row, int col)
  //: numIter(numIter), row(row), col(col) {}
};

// init
sem_t sem1, sem2;
pthread_mutex_t mtx1, mtx2;
int ct;
Cells board;

// fwd decl
Cells InitMap(
  std::vector<std::tuple<int, int>>& initial_population, int maxX, int maxY);
void* ThrFn(void* args);
std::vector<std::tuple<int, int>> map2pop();

// Returns vector of coordinates of the alive cells of the final population.
std::vector<std::tuple<int, int>> run(
  std::vector<std::tuple<int, int>> initial_population,
  int num_iter, int max_x, int max_y) {

  // init
  sem_init(&sem1, 0, 0);
  sem_init(&sem2, 0, 1);
  pthread_mutex_init(&mtx1, nullptr);
  pthread_mutex_init(&mtx2, nullptr);
  board = InitMap(initial_population, max_x, max_y);
  std::vector<std::vector<thrData>> thrs(
    max_y, std::vector<thrData>(max_x));

  // create thr
  for (int row = 0; row < max_y; ++row) for (int col = 0; col < max_x; ++col) {
    thrs[row][col].numIter = num_iter;
    thrs[row][col].row = row;
    thrs[row][col].col = col;
    pthread_create(&thrs[row][col].thr, nullptr, ThrFn, &thrs[row][col]);
  }

  // join thr
  for (int row = 0; row < max_y; ++row) for (int col = 0; col < max_x; ++col)
    pthread_join(thrs[row][col].thr, nullptr);

  // clean up sem's & mtx's
  sem_destroy(&sem1); sem_destroy(&sem2);
  pthread_mutex_destroy(&mtx1); pthread_mutex_destroy(&mtx2);

  return map2pop();
}

Cells InitMap(
  std::vector<std::tuple<int, int>>& initial_population, int maxX, int maxY) {
  Cells res(maxY, Col(maxX, 0));
  // up live cells
  for (std::tuple<int, int> livePos : initial_population) {
    int row, col; std::tie(col, row) = livePos; // y -> row & x -> col
    res[row][col] = 1;
  }
  return res;
}

int GetNeighCt(int row, int col) {
    int count = 0;
    std::vector<int> deltas {-1, 0, 1}; // delta from curr row & col
    int numRow = board.size(), numCol = board.at(0).size();

    for (int dc : deltas) for (int dr : deltas) {
      if (!dr && !dc) continue;         // skip self pos
      // boundary check (skip)
      int neighRow = row + dr, neighCol = col + dc;
      if (neighRow < 0 || neighRow >= numRow
        || neighCol < 0 || neighCol >= numCol) continue;

      count += board[neighRow][neighCol];
    }

    return count;
}

int tick(int row, int col, int count) {
    bool birth = !board.at(row).at(col) && count == 3;
    bool survive = board.at(row).at(col) && (count == 2 || count == 3);
    return birth || survive;
}

void* ThrFn(void* passedArg) {
  // parse args
  thrData arg = *static_cast<thrData*>(passedArg);
  int const numThr = board.size() * board[0].size(),
  row = arg.row, col = arg.col;

  while (arg.numIter-- > 0) { // for num iter

    pthread_mutex_lock(&mtx1);
    if (++ct == numThr) {
      sem_wait(&sem2);
      sem_post(&sem1);
    }
    pthread_mutex_unlock(&mtx1);
    sem_wait(&sem1);
    sem_post(&sem1);

    // read data from neighbors and calc nxt state
    int neighCt = GetNeighCt(row, col);

    // wait all threads to finish their calculations
    pthread_mutex_lock(&mtx1);
    if (!--ct) {
      sem_wait(&sem1);
      sem_post(&sem2);
    }
    pthread_mutex_unlock(&mtx1);
    sem_wait(&sem2);
    sem_post(&sem2);

    // wait all threads to finish their writes
    pthread_mutex_lock(&mtx2);
    // write the state back to array
    board[row][col] = tick(row, col, neighCt); 
    pthread_mutex_unlock(&mtx2);
  }

  return nullptr;
}

std::vector<std::tuple<int, int>> map2pop() {
  std::vector<std::tuple<int, int>> res;
  for (int row = 0; row < board.size(); ++row) {
    for (int col = 0; col < board[0].size(); ++col) {
      if (!board[row][col]) continue;
      res.push_back(std::make_tuple(col, row));
    }
  }
  return res;
}

