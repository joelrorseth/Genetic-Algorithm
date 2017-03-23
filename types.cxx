//
// Joel Rorseth
//

//------------------------------------------------------------------------------
//
// This file contains the definitions for the member functions of the types.
//
//------------------------------------------------------------------------------

#include "types.hxx"
#include <limits>
#include <map>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <iterator>
#include <iostream>

using namespace std;

//------------------------------------------------------------------------------

namespace cs340 
{
  ostream& operator << (ostream& os, schedule const& c)
  {
    os << "{ ";
    for (size_t i{}; i < c.tasks(); ++i)
      os << c.task_assignment(i) << ' ';
    os << '}';
    return os;
  }

  std::ostream& operator << (std::ostream& os, runtime_matrix const& matrix)
  {
    for (size_t i{}; i < matrix.tasks(); ++i) 
    {
      for (size_t j{}; j < matrix.machines(); ++j)
        os << matrix(i, j) << ' ';
      os << '\n';
    }
    return os;
  }

  // Generate a random matrix with the given dimensions.
  runtime_matrix create_random_matrix(size_t const t, size_t const m,
    size_t const time_max, random_generator& gen)
  {
    // 1. Create an object of type runtime_matrix with t tasks and m machines.
    // Here, elements_ is initialized with size, so is t, m, tasks_, machines_
	runtime_matrix matrix{t, m};	


    // 2. Create a uniform_int_distribution that samples size_t's
    // from the range [0, time_max].

	uniform_int_distribution<> dist{0,std::chrono::time_point::max};

    // 3. Write a nested for loop to fill the matrix with random
    // values. Use the distribution object (step 2) and the random 
    // generator (i.e., gen) passed in as an argument to generate
    // suitable random values.
    //
    // HINT: The matrix must be filled in for all (i,j) values.
    //       Use two for loops (one nested inside the other) to
    //       populate these values.

    // TODO: Properly access elements_ property of declared matrix object
    for (int i=0; i < t; ++i) {
        for (int j=0; j < m; ++j) {
            elements_.push_back(RAND);
        }
    }

    // 4. Return the matrix.
    return matrix;
  }

  // We compute the score of each schedule via it's makespan.  The
  // makespan of a schedule is simply the total time from start to
  // finish. In our case, that means the makespan is the maximum time
  // any given machine will take to finish all of the tasks assigned
  // to it.
  //
  // Since a lower makespan is better, but a higher score means a
  // solution is more fit, we invert the makespan and pass the result
  // through a smoothening function to get a decent score.
  double schedule::score(runtime_matrix const& matrix) const
  {
    // 1. Check if data_ is empty. If so, return zero.
    //
    // REQUIREMENT: Do NOT use .size() to determine if data_ is empty!

    if ( tasks() == 0 )
        return 0;

    // 2. Check if we already have a cached score. If so,
    // return the score we've cached. (See the class declaration
    // in types.hxx.)

    if (has_cache_)
        return cached_score_;

    // 3. We need to compute the score. First, create an object
    // of type std::multimap<std::size_t, std::size_t>, which
    // will map machines to the tasks that will run on it (as dictated
    // by the schedule). Call this variable machine_schedule.

    std::multimap<std::size_t, std::size_t> machine_schedule;

    // Populate the multimap with a mutable lambda and for_each.
    for_each(
      begin(data_),
      end(data_),
      [&machine_schedule, i = 0](auto const& m) mutable 
      {
        machine_schedule.emplace(m, i);
        ++i;
      }
    );

    // 4. Next we compute the total runtime for each machine, and
    // store the maximum of them. First create a variable called
    // total_runtime. Assign it to the initial value of
    // std::numeric_limits<std::size_t>::min(), which is the smallest
    // value that can be stored in a size_t.

    auto total_runtime = std::numeric_limits<std::size_t>::min();

    for (size_t m = 0; m < matrix.machines(); ++m) 
    {
        // 5. Find each task assigned to the machine by using
        // machine_schedule's .equal_range() member function. Pass
        // in m as the parameter.

        // Find all pairs with machine m
        // Returns a std::pair<iterator, iterator> with range of found elements
        auto range = machine_schedule.equal_range(m);

        // 6. Using std::accumulate along with the range returned
        // from the call to equal_range, compute the total run time
        // for this machine. Use a lambda, capturing the matrix by
        // reference, to compute the sum.
        //
        // HINTS:
        //   *) The type of value you are accumulating is size_t.
        //   *) Notice that the lambda's arguments don't modify 
        //      anything so they should be const&.
        //   *) You want to add the accumulated value to the next
        //      value. The value being accumulated is accessible 
        //      via matrix's function call operator which has 
        //      two arguments, m(i,j). Refer to the definition
        //      of RT[i,j] in types.hxx to pass the correct values
        //      to m(i,j).


        // data_ is a list of machines
        // There are task_ number of machines
        // machine_schedule contains <machine, task> pairs

        total_runtime = std::accumulate(range.first, range.second, 0,
            [&matrix](const std::size_t & a, const std::size_t & b) -> std::size_t {

                // Not sure if this is correct
                return a + matrix(b.second, b.first);
        });

        // 7. Using std::max, update the total_runtime variable declared
        // outside of the loop.
        
        // TODO: Not enough detail given, needs to be fixed
        total_runtime = std::max(total_runtime, ______);
    }



    // 8. Now compute the cached score by using the following
    // formula:
    //
    //     cached score = 1 / (total runtime + 1) * 1000
    //
    // Also, set the has_cache_ flag to true, so we avoid having
    // to recompute this value the next time we need it.

    cached_score_ = 1 / (total_runtime + 1) * 1000;


    // 9. Finally, return the cached score you computed.
    return cached_score_;
  }
}

//------------------------------------------------------------------------------
