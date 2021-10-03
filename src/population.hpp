///////////////////////////////////////////////////////////////////////////////
/// The Population Class
///     Models an infected population.
/// @file population.hpp
/// @brief The Population Class
/// @author Diego Carvalho - d.carvalho@ieee.org
/// @date 2021-08-21
/// @version 1.0 2021/08/21
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <memory>
#include <random>
#include <vector>

#include "subject.hpp"

// Statistics distribution types used on the code
using real_uniform_t = std::uniform_real_distribution<>;
using integer_uniform_t = std::uniform_int_distribution<>;

///////////////////////////////////////////////////////////////////////////////
/// class Population
///     a collective of single human beings that belongs to the experiment
///     this provides bookeeping for infection, quarantine, and tagging.
///////////////////////////////////////////////////////////////////////////////
class Population
{
  private:
    /// Random number generator
    std::shared_ptr<std::mt19937_64> my_gen;
    /// Subect vector
    std::vector<Subject> population;

  public:
    /// Ctor
    /// @param gen - random number generator shared_prt
    /// @param expected_size - maximum population size (affect pre allocation)
    Population(std::shared_ptr<std::mt19937_64> gen,
               const int expected_size = 1000)
      : my_gen(gen)
    {
        this->population.reserve(expected_size);
    }

    /// Dctor
    ~Population() { this->population.clear(); }

    /// index operator
    /// @param index - subject position
    Subject& operator[](const int index) { return this->population[index]; }

    /// begin Subject iterator
    auto begin() const { return this->population.begin(); }

    auto first()
    {
        return std::find_if(population.begin(), population.end(), [](auto& p) {
            return p.is_active();
        });
    }

    auto end() const { return population.end(); }

    auto count_active()
    {
        return std::count_if(population.begin(), population.end(), [](auto& p) {
            return p.is_active();
        });
    }

    auto count_recovered()
    {
        return std::count_if(population.begin(), population.end(), [](auto& p) {
            return !p.is_active();
        });
    }

    /// clear the active flag and move fist_ind if needed
    /// @param ind - individual index
    void clear_active(const int ind) { this->population[ind].clear_active(); }

    /// emplace a new Subject in the population (full Subject Ctor)
    /// @param day - days of infection since the infection day
    /// @param parent - parent index
    /// @param cDay - the day from Zero-day when the cantamination occurs
    /// @param active - true if the subject is infecting
    /// @param quarantine - true if the subject is in quarantine
    auto new_subject(const int day,
                     const int parent,
                     const int cDay,
                     const bool active,
                     const bool quarantine)
    {
        auto ind = population.size();
        population.emplace_back(day, parent, cDay, active, quarantine);
        return ind;
    }

    /// emplace a new Subject in the population
    /// @param active - true if the subject is infecting
    /// @param quarantine - true if the subject is in quarantine
    auto seed_subject(const bool active, const bool quarantine)
    {
        population.emplace_back(active, quarantine);
    }

    /// reset the population, clearing the vector<Subject>
    auto reset_population() { this->population.clear(); }

    /// seed the population with I and R members
    /// @param i0active
    /// @param i0recovered
    /// @param percentage
    /// @param max_transmission_day
    void seed_infected(const int i0active,
                       const int i0recovered,
                       const double percentage,
                       const int max_transmission_day);

    auto size() const { return population.size(); }

    auto id(const Subject& s) const { return &s - &(this->population[0]); }
};