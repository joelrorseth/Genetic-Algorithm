#ifndef CS340_PROGRAM_OPTIONS_
#define CS340_PROGRAM_OPTIONS_

//------------------------------------------------------------------------------
//
// Include this header file to parse command line arguments.
//
//------------------------------------------------------------------------------

#include <random>
#include <vector>
#include <cstddef>
#include <string>
#include <sstream>
#include <cstdlib>
#include <iostream>

#include <boost/program_options.hpp>

namespace cs340 
{
  // Arguments to be read in from the command line.
  struct program_options 
  {
    program_options(int argc, char* argv[]);

    std::vector<std::size_t> seeds;   // Seeds for the random number generator.
    std::size_t min_pool_size;        // Starting pool size.
    std::size_t max_pool_size;        // Ending pool size.
    std::size_t pool_size_step;       // How much to increase it each step.
    std::size_t tasks;                // Number of tasks to schedule.
    std::size_t machines;             // Number of machines to schedule tasks to.
    std::size_t generations;          // Number of generations.
    std::size_t threads;              // Number of threads to use to run the sim.
  };

  program_options::program_options(int argc, char* argv[])
  {
    using namespace std;
    namespace po = boost::program_options;

    string seed_string;

    po::options_description desc{"Available options"};
    desc.add_options()
      ("help", "display help message then exit")
      ("seeds",
        po::value<string>(&seed_string),
        "Initial seeds for the RNG: comma-separated positive integers")
      ("min_pool_size",
        po::value<size_t>(&min_pool_size)->default_value(1000),
        "starting pool size")
      ("max_pool_size",
        po::value<size_t>(&max_pool_size)->default_value(20000),
        "ending pool size")
      ("pool_size_step",
        po::value<size_t>(&pool_size_step)->default_value(1000),
        "pool size step")
      ("tasks",
        po::value<size_t>(&tasks)->default_value(1000),
        "number of tasks to schedule")
      ("machines",
        po::value<size_t>(&machines)->default_value(10),
        "number of machines to use")
      ("generations",
        po::value<size_t>(&generations)->default_value(1000),
        "number of generations to run the sim for")
      ("threads",
        po::value<size_t>(&threads)->default_value(1),
        "number of CPU threads to use")
      ;

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help")) 
    {
      std::cout << desc << '\n';
      std::exit(0);
    }

    // Parse the seed string.
    stringstream ss{seed_string};
    while (ss.good()) 
    {
      string seed;
      getline(ss, seed, ',');
      if (seed.empty()) continue;
      seeds.push_back(stoi(seed));
    }

    if (seeds.empty()) 
    {
      // Randomly selected.
      random_device rd;
      seeds = { rd(), rd(), rd(), rd(), rd() };
    }
  }
}

#endif
