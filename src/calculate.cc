///////////////////////////////////////////////////////////////////////////////
/// The Social Network pandemic contamination model (SOCNET)
///     Models an infected population.
/// @file population.hpp
/// @brief The Population Class
/// @author Diego Carvalho - d.carvalho@ieee.org
/// @date 2021-08-21
/// @version 1.0 2021/08/21
///////////////////////////////////////////////////////////////////////////////

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>

#include <algorithm>
#include <cmath>
#include <future> // std::async, std::future
#include <iostream>
#include <memory>
#include <random>

#include <cstdlib>
#include <string>

#include "dynamics.hpp"
#include "population.hpp"
#include "slotmachine.hpp"
#include "statistics.hpp"

// TODO:
// Move the initialization routines to a specific file
// Create a namespace (and?? pack global variables into a struct/class)
// Check all mersenne twister engines

int number_of_threads = 1;

auto
calculate_infection_sample(const unsigned int duration,
                           const unsigned int population_max_size,
                           const unsigned int i0active,
                           const unsigned int i0recovered,
                           const unsigned int max_transmission_day,
                           const unsigned int max_in_quarantine,
                           const double gamma,
                           const double percentage_in_quarantine,
                           std::shared_ptr<std::mt19937_64> gen,
                           std::shared_ptr<InfectionDynamics> inf_dyn) noexcept
{
    Statistics<double> infected_stat(duration, 0.0);
    Statistics<double> susceptible_stat(duration, 0.0);
    Statistics<double> recovered_stat(duration, 0.0);
    Statistics<double> r_0_stat(duration, 0.0);

    real_uniform_t dis(0.0, 1.0);
    integer_uniform_t i_dis(0, population_max_size);

    auto S{ population_max_size - i0active - i0recovered };
    auto I{ i0active };
    auto R{ i0recovered };

    Population population(gen, population_max_size);

    population.seed_infected(
      i0active, i0recovered, percentage_in_quarantine, max_transmission_day);

    for (auto day{ 0 }; day < duration; day++) {
        I = population.count_active();
        R = population.count_recovered();

        infected_stat.add_value(day, static_cast<double>(I));
        susceptible_stat.add_value(day, static_cast<double>(S));
        recovered_stat.add_value(day, static_cast<double>(R));

        for (auto it = population.first(); it != population.end(); it++) {

            auto& person = *it;

            if (person.is_active()) {
                auto ind{ population.id(person) };
                if (person.days_of_infection < max_transmission_day) {
                    person.days_of_infection++;
                    auto available_new_infected{ inf_dyn->infected(
                      day, ind, gen) };

                    if (!available_new_infected)
                        continue;

                    if (person.is_quarantined())
                        available_new_infected =
                          std::min(max_in_quarantine - person.decendants,
                                   available_new_infected);

                    auto new_infected{ 0 };
                    for (auto ni{ 0 }; ni < available_new_infected; ni++) {
                        // Check if the individual belongs to S, and
                        if ((i_dis(*gen) < S) && (S > 0)) {
                            new_infected++;
                            S--;
                            auto ns_id = population.new_subject(
                              0,
                              ind,
                              day,
                              true,
                              (dis(*gen) < percentage_in_quarantine));
                            population[ns_id].set_tag(inf_dyn->tag(ns_id));
                        }
                    }
                    person.decendants += new_infected;
                } else {
                    population.clear_active(ind);
                }
            }
        }

        int kp{ 0 }, dp{ 0 };
        for (auto& person : population) {
            if ((person.parent == -1) ||
                (person.days_of_infection < max_transmission_day))
                continue;
            kp++;
            dp += person.decendants;
        }
        if (kp)
            r_0_stat.add_value(day, double(dp) / double(kp));
    }

    std::vector<std::vector<double>> res;
    Statistics<double> inf_dyn_stat{ inf_dyn->statistics() };

    res.push_back(infected_stat.get_mean());     // 0
    res.push_back(infected_stat.get_variance()); // 1
    res.push_back(infected_stat.get_count());    // 2

    res.push_back(susceptible_stat.get_mean());     // 3
    res.push_back(susceptible_stat.get_variance()); // 4
    res.push_back(susceptible_stat.get_count());    // 5

    res.push_back(recovered_stat.get_mean());     // 6
    res.push_back(recovered_stat.get_variance()); // 7
    res.push_back(recovered_stat.get_count());    // 8

    res.push_back(r_0_stat.get_mean());     // 9
    res.push_back(r_0_stat.get_variance()); // 10
    res.push_back(r_0_stat.get_count());    // 11

    res.push_back(inf_dyn_stat.get_mean());     // 12
    res.push_back(inf_dyn_stat.get_variance()); // 13
    res.push_back(inf_dyn_stat.get_count());    // 14

    return res;
}

auto
calculate_infection_parallel(const int duration,
                             const int population_max_size,
                             const int i0active,
                             const int i0recovered,
                             const int samples,
                             const int max_transmission_day,
                             const int max_in_quarantine,
                             const double gamma,
                             const double percentage_in_quarantine,
                             std::shared_ptr<InfectionDynamics> inf_dyn)
{
    Statistics<double> infected_stat(duration, 0.0);
    Statistics<double> susceptible_stat(duration, 0.0);
    Statistics<double> recovered_stat(duration, 0.0);
    Statistics<double> r_0_stat(duration, 0.0);
    Statistics<double> inf_dyn_stat(duration, 0.0);

    const auto div{ number_of_threads };

    SlotMachine gen_pool(div);

    for (auto k{ 0 }; k < samples / div; k++) {
        std::vector<std::future<std::vector<std::vector<double>>>> fut;

        for (auto i{ 0 }; i < div; i++) {
            auto gen{ gen_pool.get_random(i) };
            fut.push_back(std::async(calculate_infection_sample,
                                     duration,
                                     population_max_size,
                                     i0active,
                                     i0recovered,
                                     max_transmission_day,
                                     max_in_quarantine,
                                     gamma,
                                     percentage_in_quarantine,
                                     gen,
                                     inf_dyn));
        }
        for (auto& it : fut) {
            auto ret = it.get();
            for (auto d{ 0 }; d < duration; d++) {
                infected_stat.add_value(d, ret[0][d]);
                susceptible_stat.add_value(d, ret[3][d]);
                recovered_stat.add_value(d, ret[6][d]);
                r_0_stat.add_value(d, ret[9][d]);
                inf_dyn_stat.add_value(d, ret[12][d]);
            }
        }
    }

    std::vector<std::vector<double>> res;

    res.push_back(infected_stat.get_mean());     // 0
    res.push_back(infected_stat.get_variance()); // 1
    res.push_back(infected_stat.get_count());    // 2

    res.push_back(susceptible_stat.get_mean());     // 3
    res.push_back(susceptible_stat.get_variance()); // 4
    res.push_back(susceptible_stat.get_count());    // 5

    res.push_back(recovered_stat.get_mean());     // 6
    res.push_back(recovered_stat.get_variance()); // 7
    res.push_back(recovered_stat.get_count());    // 8

    res.push_back(r_0_stat.get_mean());     // 9
    res.push_back(r_0_stat.get_variance()); // 10
    res.push_back(r_0_stat.get_count());    // 11

    res.push_back(inf_dyn_stat.get_mean());     // 12
    res.push_back(inf_dyn_stat.get_variance()); // 13
    res.push_back(inf_dyn_stat.get_count());    // 15

    return res;
}
