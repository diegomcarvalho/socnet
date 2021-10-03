#include "population.hpp"

void
Population::seed_infected(const int i0active,
                          const int i0recovered,
                          const double percentage,
                          const int max_transmission_day)
{
    real_uniform_t dis(0.0, 1.0);

    for (auto i{ 0 }; i < i0recovered; i++) {
        this->population.emplace_back(
          14, -1, 0, true, (dis(*my_gen) < percentage));
    }

    integer_uniform_t i_dis(1, max_transmission_day);

    for (auto i{ 0 }; i < i0active; i++) {
        this->population.emplace_back(
          i_dis(*my_gen), -1, 0, true, (dis(*my_gen) < percentage));
    }
    return;
}