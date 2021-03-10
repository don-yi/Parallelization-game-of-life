#define HAVE_STRUCT_TIMESPEC

#include <pthread.h>
#include <vector>
#include "gol.h"

using Row   = std::vector<int>;
using Cells = std::vector<Row>;

// thr arg struct
struct args {
  int numIter = -1, row = -1, col = -1;
  args(int numIter, int x, int y)
  : numIter(numIter), row(x), col(y) {}
};

// init
pthread_cond_t cond1 = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond2 = PTHREAD_COND_INITIALIZER;
pthread_mutex_t lock1 = PTHREAD_MUTEX_INITIALIZER
, lock2 = PTHREAD_MUTEX_INITIALIZER;
int ct = 0;
Cells board;

// fwd decl
Cells InitMap(
  std::vector<std::tuple<int, int>>& initial_population, int maxX, int maxY);
void* ThrFn(void* args);
std::vector<std::tuple<int, int>>& map2pop();

// Returns vector of coordinates of the alive cells of the final population.
std::vector<std::tuple<int, int>> run(
  std::vector<std::tuple<int, int>> initial_population,
  int num_iter, int max_x, int max_y) {

  // init
  board = InitMap(initial_population, max_x, max_y);
  std::vector<std::vector<pthread_t>> thrs(max_y,
    std::vector<pthread_t>(max_x, pthread_t()));

  // create thr
  for (int x = 0; x < max_x; ++x) {
    for (int y = 0; y < max_y; ++y) {
      args thrArgs(num_iter, x, y); // arg init
      pthread_create(&thrs.at(x).at(y), nullptr, ThrFn, &thrArgs);
    }
  }

  // join thr
  for (int x = 0; x < max_x; ++x) for (int y = 0; y < max_y; ++y)
    pthread_join(thrs.at(x).at(y), nullptr);

  return map2pop();
}

Cells InitMap(
  std::vector<std::tuple<int, int>>& initial_population, int maxX, int maxY) {
  Cells res(maxX, Row(maxY));
  // init map w/ 0
  for (int x = 0; x < maxX; ++x) for (int y = 0; y < maxY; ++y)
    res[x][y] = 0;
  // up live cells
  for (std::tuple<int, int> livePos : initial_population) {
    int x, y; std::tie(x, y) = livePos;
    res[x][y] = 1;
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
  args* argPtr = static_cast<args*>(passedArg);
  int const numThr = board.size() * board[0].size(),
  row = argPtr->row, col = argPtr->col;

  while (argPtr->numIter-- > 0) { // for num iter

    // read data from neighbors and calc nxt state
    int neighCt = GetNeighCt(row, col);

    // wait all threads to finish their calculations
    pthread_mutex_lock(&lock1);    // acquire lock
    if (++ct == numThr) {
      ct = 0;
      pthread_cond_broadcast(&cond1);
    } else pthread_cond_wait(&cond1, &lock1);
    pthread_mutex_unlock(&lock1);  // release lock

    pthread_mutex_lock(&lock2);    // acquire lock
    if (ct++ == numThr) {
      ct = 0;
      pthread_cond_broadcast(&cond2);
    } else {
      board[row][col] = tick(row, col, neighCt); // write the state back to array
      // wait all threads to finish their writes
      pthread_cond_wait(&cond2, &lock2);
    }
    pthread_mutex_unlock(&lock2);  // release lock
  }

  return nullptr;
}

std::vector<std::tuple<int, int>>& map2pop() {
  std::vector<std::tuple<int, int>> res;
  for (int x = 0; x < board.size(); ++x) {
    for (int y = 0; y < board[0].size(); ++y) {
      if (!board[x][y]) continue;
      res.push_back(std::make_tuple(x, y));
    }
  }
  return res;
}

