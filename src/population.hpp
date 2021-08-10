#pragma once

#include <memory>
#include <random>
#include <vector>

constexpr uint8_t fACTIVE = 0x01;
constexpr uint8_t fQUARANTINE = 0x01 << 1;

using real_uniform_t = std::uniform_real_distribution<>;
using integer_uniform_t = std::uniform_int_distribution<>;

class Subject
{
  private:
    uint8_t flags;

  public:
    uint8_t days_of_infection;
    uint32_t parent;
    uint16_t contamination_day;
    uint8_t decendants;

    inline const bool is_active() { return this->flags & fACTIVE; }

    inline void set_active() { this->flags ^= fACTIVE; }

    inline void clear_active() { this->flags &= ~fACTIVE; }

    inline const bool is_quarantined() { return this->flags & fQUARANTINE; }

    inline void set_quarantined() { this->flags ^= fQUARANTINE; }

    inline void clear_quarantined() { this->flags &= ~fQUARANTINE; }

    inline void set_active_and_quarantine(bool a, bool q)
    {
        this->flags = uint8_t(a) | (uint8_t(q) << 1);
    }

    Subject(const int doi = 0,
            const int p = -1,
            const int c = 0,
            const bool a = false,
            const bool q = false)
      : flags(uint8_t(a) | (uint8_t(q) << 1))
      , days_of_infection(doi)
      , parent(p)
      , contamination_day(c)
      , decendants(0)
    {}

    Subject(const bool a, const bool q)
      : flags(uint8_t(a) | (uint8_t(q) << 1))
      , days_of_infection(0)
      , parent(-1)
      , contamination_day(0)
      , decendants(0)
    {}
};

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

    auto begin() { return population.begin(); }
    auto end() { return population.end(); }

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

    int first_subject() const { return first_ind; }

    void move_first(const int id) { first_ind = id; }
};