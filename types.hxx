#ifndef CS340_TYPES_HXX_
#define CS340_TYPES_HXX_

//------------------------------------------------------------------------------
//
// This header file contains the declarations and inline function
// definitions for the types used in the simulation.
//
// random_generator: A type alias for the kind of random number
// generator we will use throughout the simulation.
//
// schedule: The solution type for the simulation. A schedule is an
// assignment of tasks to machines.
//
// runtime_matrix: The matrix containing the amount of time (in
// seconds) a task T takes to run on a machine M. These are treated as
// read-only objects during the simulation run and are consulted for
// scoring etc.
//
//------------------------------------------------------------------------------

#include <vector>
#include <cstddef>
#include <numeric>
#include <iosfwd>
#include <random>

//------------------------------------------------------------------------------

namespace cs340 
{
  // Our random number generator of choice.
  using random_generator = std::mt19937_64;

  // Forward declaration.
  class runtime_matrix;

  // A schedule is a vector of length T, where T is the number of
  // tasks to assign. Each element t[k] is a value in the range [0,
  // M), where M is the number of that we can assign tasks to.
  struct schedule 
  {
    schedule() = default;

    explicit schedule(std::size_t sz)
      : data_(sz)
    { 
    }

    schedule(schedule const&) = default;
    schedule(schedule&&) = default;

    schedule& operator = (schedule const&) = default;
    schedule& operator = (schedule&&) = default;

    ~schedule() = default;

    auto task_assignment(size_t i) const 
    { 
      return data_[i]; 
    }

    void set_task_assignment(size_t i, size_t m)
    { 
      data_[i] = m; 
      has_cache_ = false; 
    }

    auto tasks() const 
    { 
      return data_.size(); 
    }

    // Scoring a chromosome involves computig the "makespan" of the
    // schedule. We use the times stored in the matrix to compute
    // the overall summary time of the soluion.  Faster times lead
    // to better scores.
    //
    // The makespan of a schedule is the total time it takes to
    // complete all tasks, from start to finish.
    double score(runtime_matrix const&) const;

  private:
    std::vector<std::size_t> data_;
    mutable bool has_cache_ = false;
    mutable double cached_score_;
  };

  // So that we can output solutions.
  std::ostream& operator << (std::ostream&, schedule const&);

  // A runtime matrix is a |T| x |M| matrix where elements RT[i,j] is
  // the time (in seconds) that task i takes to run on task j.
  // T represents tasks and M represents machines. |T| refers to the
  // total number of tasks and |M| refers to the total number of
  // machines.
  class runtime_matrix
  {
  public:
    // Construct our matrix object. Pay close attention to the
    // different brace-styles. The elements std::vector needs to use 
    // the traditonal init style, otherwise the t * m will be treated as
    // a single-element initializer_list object. The latter will prevent
    // invoking the std::vector accepting a single size_t argument.
    runtime_matrix(std::size_t t, std::size_t m)
      : elements_(t * m), tasks_{t}, machines_{m}
    { 
    }

    runtime_matrix(runtime_matrix const&) = default;
    runtime_matrix(runtime_matrix&&) = default;

    runtime_matrix& operator = (runtime_matrix const&) = default;
    runtime_matrix& operator = (runtime_matrix&&) = default;

    ~runtime_matrix() = default;

    // Map an (i,j) position down to the 1-D representation.
    //
    // IMPORTANT: Elsewhere in the source code, you will need to invoke
    //            this function. It is therefore important to note the
    //            definition of i and j above, i.e., know what RT[i,j]
    //            stands for.
    auto const& operator () (std::size_t i, std::size_t j) const
    { return elements_[i * machines_ + j]; }

    auto& operator () (std::size_t i, std::size_t j)
    { return elements_[i * machines_ + j]; }

    auto tasks() const { return tasks_; }
    auto machines() const { return machines_; }
    auto size() const { return elements_.size(); } // == tasks * machines

  private:
    std::vector<std::size_t> elements_;
    std::size_t tasks_;
    std::size_t machines_;
  };

  std::ostream& operator << (std::ostream&, runtime_matrix const&);

  // Generate a random matrix.
  // Parameters are (i, j), max-time, and a random_generator.
  runtime_matrix create_random_matrix(std::size_t, std::size_t, std::size_t,
    random_generator&);
}

//------------------------------------------------------------------------------

#endif
