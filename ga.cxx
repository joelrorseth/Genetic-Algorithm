//------------------------------------------------------------------------------
//
// This file contains the implementation of the genetic algorithm simulation.
// There are many helper functions inside of an anonymous namespace. That
// ensures those functions have static linkage and are not visible outside
// of this translation unit.
//
// The helper functions all take in a random generator by reference. The
// reason for passing it in as a parameter instead of relying on a glboal
// RNG is to avoid having to lock the generator when it is being used in
// different threads. The way that these functions are written, each thread
// can have its own RNG (seeded appropriately BEFORE firing off the thread)
// without introducing data races and other nasty surprises.
//
//------------------------------------------------------------------------------



#include "ga.hxx"
#include "types.hxx"

#include <utility>
#include <random>
#include <vector>
#include <algorithm>
#include <numeric>
#include <cstddef>
#include <future>

using namespace std;

//------------------------------------------------------------------------------

namespace cs340
{
    namespace
    {
        // Special sorting function object to keeping our gene pools sorted.
        struct schedule_compare
        {
            explicit schedule_compare(runtime_matrix const& m)
                    : matrix_{m}
            {}

            bool schedule_compare(schedule const& a, schedule const& b, runtime_matrix const& matrix){   //Dalyn
                return (a.score(matrix) > b.score(matrix));
            }

            // Write a function call operator overload that takes in two
            // const lvalue-references to schedule objects. The operator should
            // return true if the first schedule's score is greater than the
            // second schedule's.
            //
            // NOTE: This will be used in sorting contexts.

            /* TODO: WRITE CODE HERE */

        private:
            runtime_matrix const& matrix_;
        };

        // Populate the gene pool with random values. Each machine in each
        // schedule has equal probability of occuring.
        auto populate_gene_pool(runtime_matrix const& matrix,
                                size_t const pool_size, random_generator& gen)
        {
            // 1. Create a std::vector of schedules, and use the .reserve()
            // member function to reserve space for pool_size schedules.
            //***DONE***

            std::vector<schedule> scheduleVector ();  //Dalyn
            scheduleVector.reserve(pool_size);

            // 2. Create a std::uniform_int_distribution to sample from. The
            // resulting objects should be of type size_t, and should fall
            // in the range [0, matrix.machines() - 1]. NOTE: This is an
            // INCLUSIVE range. <-- TAKE THIS INTO ACCOUNT!
            //***DONE***

            std::uniform_int_distribution<size_t> distribution_(0, (matrix.machines() - 1));    //Dalyn

            // 3. Using std::generate_n, fill the vector you created
            // with randomly generated schedules. Use a lambda
            // expression as the argument to generate_n. Within that
            // lambda expression, you should use a for-loop and the
            // .set_task_assignment() member function of the schedule
            // to fill the schedule with random data sampled from your
            // int distribution. (Remember to capture dist, gen, and matrix
            // by reference --not by value.)
            //***DONE***

            std::vector::iterator first = scheduleVector.begin();   //Dalyn
            std::generate_n (first, pool_size,
                             [&distribution_, &gen, &matrix]{
                                 for (int i = 0; i < matrix.tasks(); i++){
                                     schedule(matrix.tasks()).set_task_assignment(i, gen);
                                 }
                             });

            // 4. Sort your randomly generated pool of schedules. Use
            // std::stable_sort, passing in an object of type
            // schedule_compare as the custom comparison operator.
            //***DONE***

            std::stable_sort(scheduleVector.begin(), scheduleVector.end(), schedule_compare);   //Dalyn

            // 5. Return the pool of schedules.
            //***DONE***

            return scheduleVector;  //Dalyn
        }

        // Crossing two schedules over involves selecting a random spot
        // in the first schedule, and copying everything from the
        // beginning to that spot into a new schedule. Everything after
        // that spot is taken from the second schedule. The result is a
        // new schedule that (hopefully) inherits some of the desirable
        // traits from the two parents.
        auto cross_over(schedule c1, schedule const& c2, random_generator& gen)
        {
            // 1. Use a uniform_int_distribution to select a random point
            // in the range [0, c1.tasks() - 1].

            std::uniform_int_distribution<size_t> distribution_ (0, (c1.tasks()  - 1)); //Dalyn

            /* TODO: WRITE CODE HERE */

            // 2. Copy every element from c2, starting at the crossover
            // point, to c1. This is why we took c1 in by-value: we are copying
            // its elements and modifying it, to create a new schedule.

            //std::vector_iterator<size_t> swap = (c1.begin + distribution_);
            int n = distribution_(gen); //Dalyn
            while(
                    auto c1Swap = (c1.begin() + n) < c1.end()
                    || auto (c2Swap = (c2.begin() + n) < c2.end())){
                *c1Swap = *c2Swap;
                c1Swap++;
                c2Swap++;
            }

            /* TODO: WRITE CODE HERE */

            // 3. Return the new, modified, c1.

            return c1;  //Dalyn

            /* TODO: WRITE CODE HERE */
        }

        // Randomly change one of the task entries in the schedule.
        void mutate(runtime_matrix const& matrix, schedule& c,
                    random_generator& gen)
        {
            // 1. Pick a random task in the schedule, and a random machine.
            //
            // NOTE: Ensure the uniform distribution ranges are correct, e.g.,
            //       remembering they start at 0.
            //
            // Then set the task assignment for the selected machine. Pass in
            // appropriately generated uniform_int_distribution values for this
            // call.
            //
            // HINT:
            //   *) You will need to call a member function of schedule to do this.
            //   *) The randomly generated values will both require using gen.

            std::uniform_int_distribution<size_t> distributiont_ (0, (c.tasks()  - 1)); //Dalyn
            std::uniform_int_distribution<size_t> distributionm_ (0, (matrix.machines() - 1))
            c.task_assignment(distributiont_, distributionm_);

            /* TODO: WRITE CODE HERE */
        }

        // Run through a single generation of the genetic algorithm.
        //
        // First we do crossover to create new schedules. Afterward we
        // perform random mutations to the genes already in the pool.
        //
        // ASSUMPTION: The gene pool is in sorted order before calling
        // this function. Each operation done that modifies or produces
        // a new schedule ensures that the sorting invariant is
        // upheld. But that means nothing at all if they aren't sorted
        // from the get-go. This sorting must happen in the calling code
        // because we explicitly want to avoid re-sorting the entire
        // pool every time a change is made.
        void run_single_generation(runtime_matrix const& matrix,
                                   vector<schedule>& gene_pool, random_generator& gen)
        {
            // Some sane defaults.
            size_t const min_max_crossovers{(gene_pool.size() / 2) + 1};
            size_t const min_max_mutations{(gene_pool.size() / 3) + 1};

            size_t const max_crossovers{min(size_t{10}, min_max_crossovers)};
            size_t const max_mutations{min(size_t{25}, min_max_mutations)};

            // Initialize the global distribtions.
            uniform_int_distribution<size_t> x_pairs_dist{0, max_crossovers};
            uniform_int_distribution<size_t> mut_dist{0, max_mutations};


            // 1. Using the x_pairs_dist defined above, generate a random
            // amount of crossover pairs. Call this variable x_pairs_count.

            size_t x_pairs_count = x_pairs_dist(gen);

            /* TODO: WRITE CODE HERE */

            // 2. We will only perform the crossover operations if
            // the number of pairs is greater than zero, and less than
            // the size of your gene pool. Write an if-statement to
            // reflect this condition.

            if (x_pairs_count < gene_pool.size() && x_pairs_count > 0) {



                /* TODO: WRITE CODE HERE */

                // 2a. We need to make space for the new schedules we will
                // be generating. Create an iterator suitable to be passed into
                // the .erase() member function of your gene pool, such that
                // the last N schedules will be removed, where N is the number
                // of crossover pairs. This is why we maintain the pool in sorted
                // order!!!
                //
                // HINTS:
                //   *) Create your iterator by first obtaining the first iterator
                //      from the gene pool.
                //   *) You will need to adjust the iterator by the gene pool's
                //      size and x_pairs_count. Do remember std::vector's iterators
                //      are random-access iterators as this simplifies adjusting
                //      the iterator position.
                //   *) Finally erase the last N elements from the gene pool.

                gene_pool.erase(gene_pool.end() - x_pairs_count, gene_pool.end());

                /* TODO: WRITE CODE HERE */

                // 2b. Each schedule has a chance of being selected for crossover
                // directly proportional to its score. In order to efficiently
                // select these schedules roulette-style, we will sample a random
                // real number from the range [0, SUM-OF-ALL-SCORES). To do so,
                // we will create a table of partial sums using std::partial_sum.
                //
                // Before doing this, however, we need to create a
                // vector<double> containing the scores to sum over. Call this
                // vector totals. After creating the totals vector,
                // use std::transform to transfom the range
                // [begin(gene_pool), end(gene_pool)) into a vector of scores.
                //
                // HINTS:
                //   *) Use std::back_inserter on totals to fill totals with
                //      scores.
                //   *) Each score is computed by invoking the score() member
                //      function on each element in the gene pool.
                //      NOTE: Remember to capture the matrix by reference!

                std::vector<double> totals;
                totals.reserve(gene_pool.size());

                std::transform(gene_pool.begin(), gene_pool.end(), std::back_inserter(totals&), score());

                /* TODO: WRITE CODE HERE */

                // 2c. Now that we have the scores in a new vector, compute the
                // partial sum of that vector. In order to save space, make the
                // output iterator for the algorithm be the beginning of your
                // vector of scores.

                std::partial_sum(totals.begin(), totals.end(), totals);

                /* TODO: WRITE CODE HERE */

                // 2d. Next we will be selecting x_pairs_count pairs to
                // cross over.  Start by declaring a
                // uniform_real_distribution for our roulette-style
                // selection process. The result type should be double,
                // and the range must be [0, totals.back()).

                uniform_real_distribution<double> distributions_ (0, totals.back());

                /* TODO: WRITE CODE HERE */

                // 2e. Now write a for loop that will execute x_pairs_count times...

                for(int i = 0; i < x_pairs_count; i++) {

                    /* TODO: WRITE CODE HERE */

                    // 2e i. Generate two random numbers from our range.

                    double rand1 = distributions_(gen);
                    double rand2 = distributions_(gen);

                    /* TODO: WRITE CODE HERE */

                    // 2e ii. Using std::lower_bound, find the first value in the
                    // totals vector that is NOT LESS than the first random number
                    // you drew.

                    auto val1 = std::lower_bound(totals.begin(), totals.end(), rand1);

                    /* TODO: WRITE CODE HERE */

                    // 2e iii. Use std::distance to find the offset of the value
                    // found via lower_bound above. We will use this numeric offset
                    // to point into to gene_pool vector later...

                    auto offset1 = std::distance(totals.begin(), val1);

                    /* TODO: WRITE CODE HERE */

                    // 2e iv. Perform the two above steps again. This time using the
                    // second random number you drew.

                    auto val2 = std::lower_bound(totals.begin(), totals.end(), rand2);
                    auto offset2 = std::distance(totals.begin(), val2);


                    /* TODO: WRITE CODE HERE */

                    // 2e v. Using the two offsets you calculated, find iterators into
                    // the gene pool corresponding to the random values you drew at the
                    // top of the loop.

                    std::vector<schedule>::iterator1 = gene_pool.begin() + offset1;
                    std::vector<schedule>::iterator2 = gene_pool.begin() + offset2;

                    /* TODO: WRITE CODE HERE */

                    // 2e vi. Create a new schedule object by calling cross_over with
                    // the two schedules pointed to by your computed iterators.

                    gene_pool.iterator1 = cross_over(iterator1, gene_pool.iterator2, gen);

                    /* TODO: WRITE CODE HERE */

                    // 2e vii. We need to maintain our sorted invariant. Use std::lower_bound
                    // with an object of type schedule_compare as your custom comparision
                    // to probe the gene pool for the first schedule that is NOT GREATER
                    // than the one we just created via crossover.

                    gene_pool.insert(std::lower_bound(gene_pool.begin(), gene_pool.end(), iterator1, schedule_compare()), iterator1);

                    /* TODO: WRITE CODE HERE */

                    // 2f. End of your for-loop. You are now done performing crossover.

                }   //endfor crossover

            }   //endif crossover check     (3)


            // 3. End your if-statement guarding the crossover code.

            // 4. Crossover is complete. Now we do mutation. Start by generating
            // a random size_t from the mut_dist distribution. Call it num_mutations.

            size_t num_mutations = mut_dist(gen);

            /* TODO: WRITE CODE HERE */

            uniform_int_distribution<size_t> m_sel_dist{0, gene_pool.size() - 1};
            for (size_t j{}; j < num_mutations; ++j)
            {
                // 4a. Sample a solution to mutate from m_sel_dist. Do this by
                // first sampling the distribution, then using the fact that vector's
                // iterators are random access, find the corresponding solution
                // by adding the sampled value to begin(gene_pool).

                std::vector<schedule>::iterator mut_select = gene_pool.begin() + m_sel_dist(gen);

                /* TODO: WRITE CODE HERE */

                // 4b. Now that you have the iterator to the schedule to mutate,
                // call the mutate function you wrote earlier!

                auto new_schedule = mutate(matrix, mut_select, gen);

                /* TODO: WRITE CODE HERE */

                // 4c. Similar to the code you wrote previously that computes the
                // position in the gene pool to insert the crossed-over solution,
                // do the same with the mutated matrix. First call lower_bound.
                // Store the resulting iterator in a variable called pos.

                auto pos = std::lower_bound(gene_pool.begin(), gene_pool.end(), mut_select, schedule_compare());


                /* TODO: WRITE CODE HERE */

                // 4d. Instead of calling insert, use the algorithm std::rotate
                // to change the position of the schedule you mutated.
                //
                // HINT: You need to call std::rotate differently if pos is greater
                // than your iterator pointing to the yet-to-be placed mutated schedule.
                auto bck = gene_pool.begin();
                std::rotate(gene_pool.begin(), pos, gene_pool.end());
                gene_pool.push_back(newsched);
                std::rotate(gene_pool.begin(), bck, gene_pool.end());

                /* TODO: WRITE CODE HERE */
            }
        }

        // Run the simulation for a fixed number of generations. The
        // gene pool is taken in by reference just in case the updates
        // need to be seen in the calling code.
        //
        // Returns the best schedule seen.
        auto run_simulation_n_times(runtime_matrix const& matrix,
                                    vector<schedule>& gene_pool,
                                    size_t const num_generations,
                                    random_generator& gen,
                                    size_t const time_til_convergence = 30)
        {
            double best{};
            size_t how_long_unchanged{};

            if (gene_pool.empty()) return schedule{}; // Should never happen.

            for (size_t i{}; i < num_generations; ++i) {
                run_single_generation(matrix, gene_pool, gen);
                auto& best_schedule = gene_pool.front();

                if (best_schedule.score(matrix) > best) {
                    best = best_schedule.score(matrix);
                    how_long_unchanged = 0;
                }
                else
                    ++how_long_unchanged;
                if (how_long_unchanged > time_til_convergence) break;
            }

            return gene_pool.front();
        }
    }

    schedule run_simulation(runtime_matrix const& matrix,
                            simulation_parameters const& args,
                            random_generator& gen)
    {
        // 1. First we need to check the number of threads.
        // If the number of threads to use is less than 1, throw
        // a std::runtime_error exception with the message
        // "Cannot run on less than 1 thread".

        /* TODO: WRITE CODE HERE */

        // 2. If the number of threads is 1, then we will run the
        // simulation without any complex future stuff. First,
        // write an if-statement to check of the number of threads is 1,
        // and if true, create a gene pool (by calling populate_gene_pool)
        // of size args.pool_size. Then call the function run_simulation_n_times
        // with that gene pool and return the result of that function call.

        /* TODO: WRITE CODE HERE */

        // Otherwise, we're running multithreaded code.

        // 3. Create a vector to hold objects of type future<schedule>. call it
        // future_winners. Each thread will run an independent pool of solutions
        // to the problem and return the best solution. This vector of future schedule
        // objects will hold the best schedule from each thread.

        /* TODO: WRITE CODE HERE */

        // 4. Each thread will get its own random number generator, seeded
        // by the random number generator in this main thread. First create
        // a uniform_int_distribution of size_t's, that samples from the range
        // [0, 100]. We will generate seeds from it for each thread.

        /* TODO: WRITE CODE HERE */

        for (size_t i{}; i < args.threads; ++i)
        {
            // 4a. Now create a vector of size_t objects to store the seeds.
            // Populate the vector by sampling your distribution six times.

            /* TODO: WRITE CODE HERE */

            // 4b. Now we push back into our vector of futures...

            future_winners.push_back(
                    async(
                            launch::async,
                            // This lambda function will execute on a separate thread. We can safely
                            // hold a reference to the arguments struct and the matrix since they
                            // will be read from only. We move the seeds vector into the lambda
                            // (this is a C++14 feature) since they will only be used in the lambda
                            // body and nowhere else. This saves us a copy.
                            //
                            // The return result of the lambda is a schedule object. By passing this
                            // lambda into std::async, it converts it into a std::future<schedule> that
                            // we then store in our vector of future schedules.
                            [&matrix, &args, seeds = move(seeds), i]() -> schedule
                            {
                                // 4b i. Now turn the vector of seeds into a std::seed_seq by using
                                // std::seed_seq's iterator constructor.

                                /* TODO: WRITE CODE HERE */

                                // 4b ii. And then create a random_generator object for this thread.
                                // Pass in your std::seed_seq object to the generator's constructor.

                                /* TODO: WRITE CODE HERE */

                                // Some constants to help us determine the size of
                                // this thread's pool.
                                bool const last_iteration{i == args.threads - 1};
                                bool const even_split{args.pool_size % args.threads == 0};
                                size_t const pool_size
                                        {
                                                !last_iteration || even_split
                                                ? args.pool_size / args.threads
                                                : args.pool_size % args.threads
                                        };

                                //  4b iii. Now create a gene pool for this thread by calling
                                // populate_gene_pool. Pass in the constant pool_size as the
                                // pool size to create. Pass in the random generator that you created
                                // for this thread as the generator.

                                /* TODO: WRITE CODE HERE */

                                // 4b iv. Now call run_simulation_n_times with this thread's pool and
                                // this thread's random generator. Return the result of run_simulation_n_times.

                                /* TODO: WRITE CODE HERE */
                            }
                    )
            );
        }

        // Now we need to collect the schedules from our threads.
        schedule best{};

        // 5. Write a range-based for-loop over the vector of future winning schedules.
        // Inside the loop body, create an rvalue-reference to the winning schedule
        // corresponding to the current future. Use the future schedule's .get()
        // member function to access this. Then, using the schedule_compare object,
        // if this schedule is better than the current best one, assign the best one
        // to the rvalue-reference to the schedule (using std::move).

        /* TODO: WRITE CODE HERE */

        // 6. We now have the best schedule of the best schedules. Return it!

        /* TODO: WRITE CODE HERE */
    }
}

//------------------------------------------------------------------------------
