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
    std::mt19937_64 my_gen;
    std::vector<Subject> population;
    int first_ind;

  public:
    Population(const int expected_size = 1000)
      : first_ind(0)
    {
        population.reserve(expected_size);
    }

    ~Population() { reset_population(); }

    Subject& operator[](const int index) { return population[index]; }

    auto begin() const { return population.begin(); }
    auto end() const { return population.end(); }

    void clear_active(int ind)
    {
        this->population[ind].clear_active();
        if (this->first_subject() == ind)
            this->move_first(ind + 1);
    }

    void new_subject(const int day,
                     const int parent,
                     const int cDay,
                     const bool active,
                     const bool quarantine)
    {
        population.push_back(Subject(day, parent, cDay, active, quarantine));
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

    unsigned int size() const { return population.size(); }

    unsigned int first_subject() const { return first_ind; }

    void move_first(const int id) { first_ind = id; }
};