#ifndef GOL_H
#define GOL_H
#include <vector>
#include <tuple>

std::vector< std::tuple<int,int> > // return vector of coordinates of the alive cells of the final population
run( std::vector< std::tuple<int,int> > initial_population, int num_iter, int max_x, int max_y );
#endif
