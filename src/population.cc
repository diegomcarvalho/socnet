#include "population.hpp"

void
Population::reset_population()
{
    population.clear();
    return;
}

void
Population::seed_infected(const int i0active,
                          const int i0recovered,
                          const double percentage,
                          const int max_transmission_day)
{
    real_uniform_t dis(0.0, 1.0);

    for (auto i{ 0 }; i < i0recovered; i++) {
        new_subject(14, -1, 0, false, (dis(my_gen) < percentage));
    }

    integer_uniform_t i_dis(1, max_transmission_day);

    for (auto i{ 0 }; i < i0active; i++) {
        new_subject(i_dis(my_gen), -1, 0, true, (dis(my_gen) < percentage));
    }

    this->first_ind = i0recovered;

    return;
}

void
Population::seed_infected(const std::vector<int>& i0active,
                          const std::vector<int>& i0recovered,
                          const double percentage,
                          const int max_transmission_day)
{
    real_uniform_t dis(0.0, 1.0);

    for (auto& n : i0recovered) {
        for (int i = 0; i < n; i++) {
            seed_subject(false, (dis(my_gen) < percentage));
        }
    }

    integer_uniform_t i_dis(1, max_transmission_day);

    for (auto& n : i0active) {
        for (int i = 0; i < n; i++) {
            new_subject(i_dis(my_gen), -1, 0, true, (dis(my_gen) < percentage));
        }
    }

    this->first_ind = i0recovered.size();

    return;
}
