#ifndef CS340_GA_HXX_
#define CS340_GA_HXX_

//------------------------------------------------------------------------------
//
// This header contains the declarations for the simulation. It also
// includes type definitions for the simulation arguments. Any
// parameters that can be changed about the simulation runtime should
// be stored in those structs.
//
//------------------------------------------------------------------------------

#include "types.hxx"

#include <cstddef>

namespace cs340 
{
  // Parameters for a single run of the simulation.
  struct simulation_parameters 
  {
    size_t generations;
    size_t pool_size;
    size_t threads;
  };

  // Run the genetic algorithm for a specified number of
  // generations. Returns the schedule with the best score after all
  // that.
  schedule run_simulation(runtime_matrix const&,
    simulation_parameters const&,
    random_generator&);
}

//------------------------------------------------------------------------------

#endif
