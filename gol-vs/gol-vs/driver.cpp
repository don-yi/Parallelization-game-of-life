#include "gol.h"
#include <fstream>   /* ifstream */
#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <tuple>
#include <cstdio>    /* sscanf */

void draw( std::vector< std::tuple<int,int> > & population, int max_x, int max_y )
{
    std::sort( population.begin(), population.end(),
            []( std::tuple<int,int> const& a, std::tuple<int,int> const& b ) {
                return  ( std::get<1>(a) < std::get<1>(b) )
                        or 
                        ( std::get<1>(a) == std::get<1>(b) and std::get<0>(a) < std::get<0>(b) );
            }
            );

    // draw population          +---------> x
    int old_y=-1; // down       |
    int old_x=-1; // right      V y


#if 0
    // +--------------+ style
    std::cout << std::setw(4) << "+"; // top-left corner
    std::cout << std::setfill( '-' ) << std::setw( max_x+1 ) << "+" << std::setfill( ' ' ); // line
#else
    //  000000000011111111112222222222
    // +012345678901234567890123456789+
    std::cout << std::setw(4) << " "; // top-left corner
    if ( max_x>= 12 ) { for ( int i=0; i<max_x; ++i ) { std::cout << i/10; } }
    std::cout << std::endl << std::setw(4) << " ";
    for ( int i=0; i<max_x; ++i ) { std::cout << i%10; }
    std::cout << std::endl;
    std::cout << std::setw(4) << "+"; // top-left corner
    std::cout << std::setfill( '-' ) << std::setw( max_x+1 ) << "+" << std::setfill( ' ' ); // line
#endif

    // notation
    std::cout << "--> x";  // x-axis symbol

    for ( auto const & t : population ) {
        int x,y;
        std::tie( x,y ) = t;

        for ( int j = old_y; j<y; ++j ) { // skip rows if y changed
            if ( j != -1 ) { // if not the first line
                std::cout << std::setw( max_x -old_x ) << "|"; // finish line
            }
            if ( j<max_y-1) {  // if not the last line
                std::cout << std::endl << std::setw(3) << j+1 << "|";    // start of the next line
            }
            old_x = -1;
        }

        // skip columns and place "*"
        std::cout << std::setw( x -old_x ) << "*";

        // prepare for next iteration
        old_x = x;
        old_y = y;
    }

    // finish the field
    for ( int j = old_y; j<max_y; ++j ) { // skip rows if y changed
        std::cout << std::setw( max_x -old_x ) << "|" << std::endl; // finish line
        if ( j<max_y-1) {  // if not the last line
            std::cout << std::setw(3) << j+1 << "|";    // start of the next line
        }
        old_x = -1;
    }
    
    std::cout 
        << std::setw(4) << "+" // bottom-left corner
        << std::setfill( '-' ) << std::setw( max_x+1 ) << "+" << std::setfill( ' ' ) // line
        << std::endl << std::setw(4) << "|" << "\n" << std::setw(4) << "V" << "  y" << std::endl;  // y-axis symbol
}

std::tuple< std::vector< std::tuple<int,int> >, int, int > // population, max_x, max_y
read( char const * infile )
{
    int max_x =0, max_y =0;
    std::vector< std::tuple<int,int> > initial_population;
    std::ifstream file_init_population( infile, std::ifstream::in );
    file_init_population >> max_x >> max_y;

    int x,y;
    file_init_population >> x >> y;
    while ( !file_init_population.eof() ) {
        initial_population.push_back( std::make_tuple( x,y ) );
        file_init_population >> x >> y;
    }

//    for ( auto t : initial_population ) {
//        int x,y;
//        std::tie( x,y ) = t;
//        std::cout << x << " " << y << std::endl;
//    }

    return std::make_tuple( initial_population, max_x, max_y );
}

void test( char const * infile, int num_iter )
{
    std::vector< std::tuple<int,int> > initial_population;
    int max_x, max_y;
    std::tie( initial_population, max_x, max_y ) = read( infile );

    //draw( initial_population, max_x, max_y );
 
    std::vector< std::tuple<int,int> > final_population = run( initial_population, num_iter, max_x, max_y );
    draw( final_population, max_x, max_y );
    // print final_population
#if 0
    for ( auto const & t : final_population ) {
        int i,j;
        std::tie( i,j ) = t;
        std::cout << i << " " << j << std::endl;
    }
#endif
}

// in0 - blinker
// in1 - glider on 10x10
// in2 - glider on 25x25
void test0() { test( "in0", 10 ); }     
void test1() { test( "in0", 11 ); }
void test2() { test( "in0", 100 ); }
void test3() { test( "in0", 101 ); }
void test4() { test( "in1", 10 ); }
void test5() { test( "in1", 11 ); }
void test6() { test( "in1", 24 ); }
void test7() { test( "in2", 84 ); }

void (*pTests[])() = { 
    test0,test1,test2,test3,test4,test5,test6,test7
}; 

int main( int argc, char ** argv ) 
{
    if ( argc == 2 ) { // single argument - assume test number provided
		int test = 0;
		std::sscanf(argv[1],"%i",&test);
		try {
            pTests[test]();
		} catch( const char* msg) {
			std::cerr << msg << std::endl;
		}
        return 0;
    }

    // else try to read 2 arguments - initial population file and number of iterations
    if ( argc != 3 ) {
        std::cout << "expected 2 parameters 1) init population file to read 2) number  iterations" << std::endl;
        return 1;
    }
        
    int num_iter = 0;
    std::sscanf(argv[2],"%i",&num_iter);

    test( argv[1], num_iter );
        return 0;
}
