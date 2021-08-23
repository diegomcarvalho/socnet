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

using real_uniform_t = std::uniform_real_distribution<>;
using integer_uniform_t = std::uniform_int_distribution<>;

class Population
{
  private:
    std::shared_ptr<std::mt19937_64> my_gen;
    uint32_t first_ind;
    std::vector<Subject> population;

  public:
    Population(std::shared_ptr<std::mt19937_64> gen,
               const int expected_size = 1000)
      : my_gen(gen)
      , first_ind(0)
    {
        population.reserve(expected_size);
    }

    ~Population() { reset_population(); }

    Subject& operator[](const int index) { return population[index]; }

    auto begin() const { return population.begin(); }
    auto end() const { return population.end(); }

    void clear_active(const int ind)
    {
        this->population[ind].clear_active();
        if (this->first_subject() == ind)
            this->move_first(ind + 1);
    }

    auto new_subject(const int day,
                     const int parent,
                     const int cDay,
                     const bool active,
                     const bool quarantine)
    {
        auto ind = population.size();
        population.push_back(Subject(day, parent, cDay, active, quarantine));
        return ind;
    }

    void seed_subject(const bool active, const bool quarantine)
    {
        population.push_back(Subject(active, quarantine));
    }

    void reset_population();

    void seed_infected(const int i0active,
                       const int i0recovered,
                       const double percentage,
                       const int max_transmission_day);

    void seed_infected(const std::vector<int>& i0active,
                       const std::vector<int>& i0recovered,
                       const double percentage,
                       const int max_transmission_day);

    auto size() const { return population.size(); }

    uint32_t first_subject() const { return first_ind; }

    void move_first(const int id) { first_ind = id; }
};