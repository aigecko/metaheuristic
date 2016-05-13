#pragma once

#include <cstdint>
#include <cmath>
#include <iostream>
#include <limits>
#include <random>
#include <algorithm>
#include <chrono>
#include <deque>
#include <valarray>

// Declarations
namespace MH {

    // All algorithms will use Solution to store the solution encoding and evaluation result (score).
    // Template parameter Encoding is the type of the encoding suppose to use.
    template <typename Encoding>
    struct Solution {
        friend bool operator<(Solution<Encoding> &a, Solution<Encoding> &b) {
            return a.score < b.score;
        }
        // Solution encoding
        Encoding encoding;
        // Evaluation result
        double score;
        // Three types of constructor
        Solution();
        Solution(Encoding &);
        Solution(Encoding &, double);
    };

    namespace Trajectory {

        // A trajectory instance object specifies generation limit, neighborhood functions and evaluation function
        // there is also an optional "inf" field to provide additional information to evaluation function
        template <typename Encoding>
        struct Instance {
            uint64_t generation_limit;
            // user provide neighborhood generate function
            // aceept an encoding and return a vector of neighborhood encodings
            std::vector<Encoding> (*neighbors)(Encoding &);
            // optional information provide to evaluate
            void *inf;
            // evaluate function, accept an encoding and additional information from instance.inf as (void *)
            // return a real number which is suppose to be minimize.
            double (*evaluate)(Encoding &, void *);
        };


        template <typename Strategy>
        struct II{
            Strategy strategy;
        };

        // These class is used as temeplate parameter for II
        class II_BestImproving {};
        class II_FirstImproving {};
        class II_Stochastic {};

        struct SA{
            double init_temperature;
            int64_t epoch_length;
            double (*cooling)(double);
            double temperature;
            int64_t epoch_count;
        };

        template <typename Encoding, typename TraitType>
        struct TS{
            int8_t length;
            // the trait function accept an encoding and transform it into traits to store in tabu list.
            TraitType (*trait)(Encoding&, void *);
            std::deque<TraitType> queue;
        };

        // Random search is aim to compare with others
        struct RS{};
        struct BFS{};
        struct DFS{};

        // aliases
        template <typename Strategy>
        using IterativeImprovement = II<Strategy>;
        using SimulatedAnnealing = SA;
        template <typename Encoding, typename TraitType>
        using TabuSearch = TS<Encoding, TraitType>;
        using RandomSearch = RS;

        // Function declarations
        template <typename Encoding, typename AlgorithmType>
        Solution<Encoding> search(Instance<Encoding> &,
                                          AlgorithmType &,
                                          Encoding &);
        template <typename Encoding, typename Strategy>
        void initialize(Instance<Encoding> &, II<Strategy> &, Encoding &);
        template <typename Encoding>
        void initialize(Instance<Encoding> &, SA &, Encoding &);
        template <typename Encoding, typename TraitType>
        void initialize(Instance<Encoding> &, TS<Encoding, TraitType> &, Encoding &);
        template <typename Encoding>
        void initialize(Instance<Encoding> &, RS &, Encoding &);
        template <typename Encoding, typename Strategy>
        Solution<Encoding>& select(Instance<Encoding> &,
                                   Solution<Encoding> &,
                                   std::vector<Solution<Encoding>> &,
                                   II<Strategy> &);
        template <typename Encoding>
        Solution<Encoding>& select(Instance<Encoding> &,
                                   Solution<Encoding> &,
                                   std::vector<Solution<Encoding>> &,
                                   SA &);
        template <typename Encoding, typename Trait>
        Solution<Encoding>& select(Instance<Encoding> &,
                                   Solution<Encoding> &,
                                   std::vector<Solution<Encoding>> &,
                                   TS<Encoding, Trait> &);
        template <typename Encoding>
        Solution<Encoding>& select(Instance<Encoding> &,
                                   Solution<Encoding> &,
                                   std::vector<Solution<Encoding>> &,
                                   RS &);
        template <typename Encoding>
        Solution<Encoding>& select_II(Instance<Encoding> &,
                                      Solution<Encoding> &,
                                      std::vector<Solution<Encoding>> &,
                                      II_BestImproving &);
        template <typename Encoding>
        Solution<Encoding>& select_II(Instance<Encoding> &,
                                      Solution<Encoding> &,
                                      std::vector<Solution<Encoding>> &,
                                      II_FirstImproving &);
        template <typename Encoding>
        Solution<Encoding>& select_II(Instance<Encoding> &,
                                      Solution<Encoding> &,
                                      std::vector<Solution<Encoding>> &,
                                      II_Stochastic &);
        template <typename Encoding>
        Solution<Encoding>& select_SA(double,
                                      Solution<Encoding> &,
                                      std::vector<Solution<Encoding>> &);
    }

    namespace Evolutionary {

        class DE_Random {};
        class DE_Best {};
        class DE_CurrentToRandom {};
        class DE_CurrentToBest {};

        class DE_None {};
        class DE_Binomial {};
        class DE_Exponential {};

        // Encoding of DE is restrict to vector of real numbers (float | double | long double)
        template <typename SelectionStrategy, typename CrossoverStrategy>
        struct DifferentialEvolution {
            double crossover_rate;
            double current_factor;
            double scaling_factor;
            uint8_t num_of_diff_vectors;
            SelectionStrategy selection_strategy;
            CrossoverStrategy crossover_strategy;
        };

        template <typename FP>
        struct _DE_INF_WRAPPER {
            double (*original_evaluate)(std::vector<FP> &, void *);
            void *original_inf;
        };

        template <typename Encoding>
        struct Instance {
            uint64_t generation_limit;
            void *inf;
            double (*evaluate)(Encoding &, void *);
        };

        template <typename FP>
        class _valarray_eq_binder {
        public:
            _valarray_eq_binder(std::valarray<FP> &array) : _array(array) {}
            _valarray_eq_binder() {}
            bool operator()(std::valarray<FP> &array) {
                return std::equal(std::begin(array), std::end(array), std::begin(_array));
            }
            void set(std::valarray<FP> &array) { _array = array; }
        private:
            std::valarray<FP> _array;
        };

        // aliases
        template <typename SelectionStrategy, typename CrossoverStrategy>
        using DE = DifferentialEvolution<SelectionStrategy, CrossoverStrategy>;

        // Function definitions
        template <typename FP, typename SelectionStrategy, typename CrossoverStrategy>
        Solution<std::vector<FP>> evolution(Instance<std::vector<FP>> &,
                                            DifferentialEvolution<SelectionStrategy, CrossoverStrategy> &,
                                            std::vector<std::vector<FP>> &);
        template <typename Encoding, typename AlgorithmType>
        Solution<Encoding> evolution(Instance<Encoding> &,
                                     AlgorithmType &,
                                     std::vector<Encoding> &);
        template <typename Encoding, typename SelectionStrategy, typename CrossoverStrategy>
        void initialize(Instance<Encoding> &,
                        DE<SelectionStrategy, CrossoverStrategy> &,
                        std::vector<Encoding> &);
        template <typename Encoding>
        std::vector<Solution<Encoding>> initialize_popultion(Instance<Encoding> &,
                                                                     std::vector<Encoding> &);
        template <typename Encoding, typename SelectionStrategy, typename CrossoverStrategy>
        void generate(Instance<Encoding> &,
                      std::vector<Solution<Encoding>> &,
                      DE<SelectionStrategy, CrossoverStrategy> &);

        template <typename Encoding, typename SelectionStrategy, typename CrossoverStrategy> 
        Encoding DE_mate(Encoding &,
                             std::vector<Solution<Encoding>> &,
                             DE<SelectionStrategy, CrossoverStrategy> &);
        template <typename Encoding, typename SelectionStrategy, typename CrossoverStrategy>
        Encoding DE_mate_select(std::vector<Encoding> &select_pool,
                                std::vector<Solution<Encoding>> &population,
                                DE<SelectionStrategy, CrossoverStrategy> &,
                                DE_Best &);
        template <typename Encoding>
        Encoding DE_mutation(std::vector<Encoding> &,
                             std::vector<Solution<Encoding>> &,
                             double,
                             uint8_t);
        template <typename Encoding>
        Encoding DE_crossover(Encoding &,
                              Encoding &,
                              double,
                              DE_None &);
        template <typename FP>
        double _DE_EVALUATE_WRAPPER(std::valarray<FP> &, void *);
    }
}

// Definitions

template <typename Encoding>
inline MH::Solution<Encoding>::Solution() : encoding(), score(0){}
template <typename Encoding>
inline MH::Solution<Encoding>::Solution(Encoding &e) : encoding(e), score(0){}
template <typename Encoding>
inline MH::Solution<Encoding>::Solution(Encoding &e, double s) : encoding(e), score(s){}

// The main search framework for trajectory-based algorithms
template <typename Encoding, typename AlgoType>
MH::Solution<Encoding>
MH::Trajectory::search(MH::Trajectory::Instance<Encoding> &instance,
                       AlgoType &algorithm,
                       Encoding &init) {

    MH::Trajectory::initialize(instance, algorithm, init);
    auto current = Solution<Encoding>(init, instance.evaluate(init, instance.inf));
    auto min = current;

    for(uint64_t generation_count = 0;
        generation_count < instance.generation_limit;
        ++generation_count) {

        auto neighbors_encoding = instance.neighbors(current.encoding);
        std::vector<Solution<Encoding>> neighbors(neighbors_encoding.size());
        // evaluate each encoding, and store to vector of Solution
        std::transform(neighbors_encoding.begin(),
                       neighbors_encoding.end(),
                       neighbors.begin(),
                       [&](auto &e) {
                           return Solution<Encoding>(e, instance.evaluate(e, instance.inf));
                       });

        // each algorithm is different on there selection
        current = MH::Trajectory::select(instance, current, neighbors, algorithm);
        //std::cout << "Generation " << generation_count << " : " << current.score << std::endl;
        if(current < min) {
            min = current;
        }
    }
    std::cout << "Final value = " << min.score << std::endl;
    return min;
}

// initialize II, nothing to do here
template <typename Encoding, typename Strategy>
inline void
MH::Trajectory::initialize(MH::Trajectory::Instance<Encoding> &,
                           MH::Trajectory::II<Strategy> &,
                           Encoding &) {
    std::cout << "Starting Iterative Improvement ..." << std::endl;
}

// initialize SA, set the temperature and epoch
template <typename Encoding>
inline void
MH::Trajectory::initialize(MH::Trajectory::Instance<Encoding> &,
                           MH::Trajectory::SA &sa,
                           Encoding &) {
    std::cout << "Starting Simulated Annealing ..." << std::endl;
    sa.temperature = sa.init_temperature;
    sa.epoch_count = 0;
}

// initialize TS, fill the tabu list (queue)
template <typename Encoding, typename TraitType>
inline void
MH::Trajectory::initialize(MH::Trajectory::Instance<Encoding> &instance,
                           MH::Trajectory::TS<Encoding, TraitType> &ts,
                           Encoding & init) {
    std::cout << "Starting Tabu Search ..." << std::endl;
    ts.queue.resize(ts.length);
    std::fill(ts.queue.begin(), ts.queue.end(), ts.trait(init, instance.inf));
}

// initialize RS, nothing to do here.
template <typename Encoding>
inline void
MH::Trajectory::initialize(MH::Trajectory::Instance<Encoding> &,
                           MH::Trajectory::RS &,
                           Encoding &) {
    std::cout << "Starting Random Search ..." << std::endl;
}

// II selection, call select_II based on II strategy
template <typename Encoding, typename Strategy>
inline MH::Solution<Encoding> &
MH::Trajectory::select(MH::Trajectory::Instance<Encoding> &instance,
                       MH::Solution<Encoding> &current,
                       std::vector<MH::Solution<Encoding>> &neighbors,
                       MH::Trajectory::II<Strategy>& ii) {
    return MH::Trajectory::select_II(instance, current, neighbors, ii.strategy);
}

// SA selection, call select_SA and handle the cooling schedule
template <typename Encoding>
inline MH::Solution<Encoding> &
MH::Trajectory::select(MH::Trajectory::Instance<Encoding > &,
                       MH::Solution<Encoding> &current,
                       std::vector<MH::Solution<Encoding>> &neighbors,
                       MH::Trajectory::SA &sa) {
    auto &result = MH::Trajectory::select_SA(sa.temperature, current, neighbors);
    ++sa.epoch_count;
    if(sa.epoch_count == sa.epoch_length) {
        sa.temperature = sa.cooling(sa.temperature);
        sa.epoch_count = 0;
    }
    return result;
}

// TS selection, compare neighbors with tabu list, choose the minimum not in the list
// and replace the oldest with the new solution
template <typename Encoding, typename TraitType>
inline MH::Solution<Encoding> &
MH::Trajectory::select(MH::Trajectory::Instance<Encoding > &instance,
                       MH::Solution<Encoding> &,
                       std::vector<MH::Solution<Encoding>> &neighbors,
                       MH::Trajectory::TS<Encoding, TraitType> &ts) {
    auto &min = neighbors.front();
    for(auto &neighbor : neighbors) {
        if(std::find(ts.queue.begin(),
                     ts.queue.end(),
                     ts.trait(neighbor.encoding, instance.inf)) == ts.queue.end() &&
           neighbor < min) {
            min = neighbor;
        }
    }
    ts.queue.pop_front();
    ts.queue.push_back(ts.trait(min.encoding, instance.inf));
    return min;
}

// Best improving II, select the minimum among neighbors
template <typename Encoding>
inline MH::Solution<Encoding> &
MH::Trajectory::select_II(MH::Trajectory::Instance<Encoding> &,
                          MH::Solution<Encoding> &current,
                          std::vector<MH::Solution<Encoding>> &neighbors,
                          II_BestImproving &) {
    auto &min = *std::min(neighbors.begin(), neighbors.end()); 
    return (min < current) ? min : current;
}

// First improving II, select the first solution that is better than current one during the iteration
template <typename Encoding>
inline MH::Solution<Encoding> &
MH::Trajectory::select_II(MH::Trajectory::Instance<Encoding> &,
                          MH::Solution<Encoding> &current,
                          std::vector<MH::Solution<Encoding>> &neighbors,
                          II_FirstImproving &) {
    for(auto &neighbor : neighbors) {
        if(neighbor < current) {
            return neighbor;
        }
    }
    return current;
}

// Stochastic II, not implement yet
template <typename Encoding>
inline MH::Solution<Encoding> &
MH::Trajectory::select_II(MH::Trajectory::Instance<Encoding> &,
                          MH::Solution<Encoding> &current,
                          std::vector<MH::Solution<Encoding>> &,
                          II_Stochastic &) {
    return current;
}

// SA selection
template <typename Encoding>
inline MH::Solution<Encoding> &
MH::Trajectory::select_SA(double temperature,
                           MH::Solution<Encoding> &current,
                           std::vector<MH::Solution<Encoding>> &neighbors) {
    // uniform random number generators
    static std::default_random_engine eng(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
    static std::uniform_real_distribution<double> uniform;
    for(auto &neighbor : neighbors) {
        if((neighbor < current) ||
           (exp(current.score - neighbor.score) / temperature > uniform(eng)) ) {
            return neighbor;
        }
    }
    return current;
}

template <typename Encoding, typename AlgorithmType>
MH::Solution<Encoding>
MH::Evolutionary::evolution(MH::Evolutionary::Instance<Encoding> &instance,
                            AlgorithmType &algorithm,
                            std::vector<Encoding> &init) {
    MH::Evolutionary::initialize(instance, algorithm, init);
    auto population = MH::Evolutionary::initialize_popultion(instance, init);
    for(auto generation_count = 0UL;
        generation_count < instance.generation_limit;
        ++generation_count) {
        // generate will mutate the population
        MH::Evolutionary::generate(instance, population, algorithm);
    }
    auto min = *std::min(population.begin(), population.end());
    std::cout << "Final value = " << min.score << std::endl;
    return min;
}

// Since DE will convert vector to valarray as an underlying type for performance,
// we need this wrapper to convert the initial population and restore the returned valarray
template <typename FP, typename SelectionStrategy, typename CrossoverStrategy>
MH::Solution<std::vector<FP>>
MH::Evolutionary::evolution(Instance<std::vector<FP>> &instance,
                            DifferentialEvolution<SelectionStrategy, CrossoverStrategy> &de,
                            std::vector<std::vector<FP>> &init) {
    // Underly instance set to valarray type
    // this wrapper wrap original evaluate function pointer and inf to new instance'inf
    auto wrapper = new _DE_INF_WRAPPER<FP>();
    wrapper->original_evaluate = instance.evaluate;
    wrapper->original_inf = instance.inf;
    auto Uinstance = MH::Evolutionary::Instance<std::valarray<FP>>();
    // this evaluation function wrapper will restore original evaluator from inf
    Uinstance.evaluate = _DE_EVALUATE_WRAPPER;
    Uinstance.generation_limit = instance.generation_limit;
    Uinstance.inf = reinterpret_cast<void *>(wrapper);
    // convert real vector to valarray
    std::vector<std::valarray<FP>> valarray_init(init.size());
    std::transform(init.begin(),
                   init.end(),
                   valarray_init.begin(),
                   [&](auto &s) {
                       return std::valarray<FP>(s.data(), s.size());
                   });
    auto result = MH::Evolutionary::evolution(Uinstance, de, valarray_init);
    // convert vallarray back to real vector
    std::vector<FP> vec_result(std::begin(result.encoding), std::end(result.encoding));
    delete wrapper;
    return MH::Solution<std::vector<FP>>(vec_result, result.score);
}

template <typename Encoding, typename SelectionStrategy, typename CrossoverStrategy>
inline void
MH::Evolutionary::initialize(MH::Evolutionary::Instance<Encoding> &,
                             MH::Evolutionary::DE<SelectionStrategy, CrossoverStrategy> &,
                             std::vector<Encoding> &) {
    std::cout << "Starting Differential Evolution ... " << std::endl;
}


template <typename Encoding>
std::vector<MH::Solution<Encoding>>
MH::Evolutionary::initialize_popultion(MH::Evolutionary::Instance<Encoding> &instance,
                                       std::vector<Encoding> &init) {
    std::vector<MH::Solution<Encoding>> population(init.size());
    std::transform(init.begin(),
                   init.end(),
                   population.begin(),
                   [&](auto &s) {
                       return MH::Solution<Encoding>(s, instance.evaluate(s, instance.inf));
                   });
    return population;
}

template <typename Encoding, typename SelectionStrategy, typename CrossoverStrategy>
void
MH::Evolutionary::generate(Instance<Encoding> &instance,
                           std::vector<Solution<Encoding>> &population,
                           MH::Evolutionary::DE<SelectionStrategy, CrossoverStrategy> &de) {
    for(auto i = 0UL; i < population.size(); ++i) {
        auto target_vec = population[i].encoding;
        auto trial_vec = MH::Evolutionary::DE_mate(target_vec, population, de);
        // environment selection
        auto trial_score = instance.evaluate(trial_vec, instance.inf);
        if(trial_score < population[i].score) {
            population[i] = MH::Solution<Encoding>(trial_vec, trial_score);
        }
    }
}


template <typename Encoding, typename SelectionStrategy, typename CrossoverStrategy> 
Encoding
MH::Evolutionary::DE_mate(Encoding &target_vec,
                          std::vector<Solution<Encoding>> &population,
                          MH::Evolutionary::DE<SelectionStrategy, CrossoverStrategy> &de) {
    std::vector<Encoding> select_pool;
    select_pool.push_back(target_vec);
    auto mutant_vec = MH::Evolutionary::DE_mate_select(select_pool, population, de, de.selection_strategy);
    mutant_vec += MH::Evolutionary::DE_mutation(select_pool, population, de.scaling_factor, de.num_of_diff_vectors);
    auto trial_vec = MH::Evolutionary::DE_crossover(target_vec, mutant_vec, de.crossover_rate, de.crossover_strategy);
    return target_vec;
}

template <typename Encoding, typename SelectionStrategy, typename CrossoverStrategy>
Encoding
MH::Evolutionary::DE_mate_select(std::vector<Encoding> &select_pool,
                                 std::vector<MH::Solution<Encoding>> &population,
                                 MH::Evolutionary::DE<SelectionStrategy, CrossoverStrategy> &,
                                 MH::Evolutionary::DE_Best &) {
    select_pool.push_back( (*std::min(population.begin(), population.end())).encoding );
    return select_pool.back();
}

template <typename Encoding>
Encoding
MH::Evolutionary::DE_mutation(std::vector<Encoding> &select_pool,
                         std::vector<MH::Solution<Encoding>> &population,
                         double scaling_factor,
                         uint8_t diff_vecs) {
    auto sol1 = select_pool.front();
    auto sol2 = select_pool.front();
    auto mutant_vec = Encoding(sol1.size());

    static std::default_random_engine eng(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
    static std::uniform_int_distribution<size_t> uniform(0, population.size() - 1);
    for(auto i = 0U; i < diff_vecs; ++i) {
        _valarray_eq_binder<double> valarr_eq;
        do {
            auto rand_idx = uniform(eng);
            sol1 = population[rand_idx].encoding;
            valarr_eq.set(sol1);
        } while(std::find_if(select_pool.begin(), select_pool.end(), valarr_eq) != select_pool.end());
        select_pool.push_back(sol1);
        do {
            auto rand_idx = uniform(eng);
            sol2 = population[rand_idx].encoding;
            valarr_eq.set(sol2);
        } while(std::find_if(select_pool.begin(), select_pool.end(), valarr_eq) != select_pool.end());
        select_pool.push_back(sol2);
        mutant_vec += sol2 - sol1;
    }
    return scaling_factor * mutant_vec;
}

template <typename Encoding>
Encoding
MH::Evolutionary::DE_crossover(Encoding &,
                               Encoding &mutant_vec,
                               double,
                               MH::Evolutionary::DE_None &) {
    return mutant_vec;
}

template <typename FP>
inline double
MH::Evolutionary::_DE_EVALUATE_WRAPPER(std::valarray<FP> &sol, void *inf) {
    std::vector<FP> vsol(std::begin(sol), std::end(sol));
    auto wrapper = *reinterpret_cast<MH::Evolutionary::_DE_INF_WRAPPER<FP> *>(inf);
    return wrapper.original_evaluate(vsol, wrapper.original_inf);
}
