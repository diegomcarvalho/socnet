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

#include <thread>

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

// std::mt19937_64 my_gen; // Standard mersenne_twister_engine seeded with rd()
static int number_of_threads = 1;

bool
is_number(const std::string& s)
{
    return !s.empty() && std::all_of(s.begin(), s.end(), ::isdigit);
}

void
init_module()
{
    // std::cout << "calculate.cc - random setup done." << std::endl;

    if (const char* env_p = std::getenv("SOCNET_NUM_THREADS")) {
        if (std::string snt(env_p); snt == "CPU_MAX") {
            number_of_threads = std::thread::hardware_concurrency();
        } else if (is_number(snt)) {
            number_of_threads = std::stoi(snt);
        }
    }
    return;
}

inline int
find_first(Population& population)
{
    // auto size{ population.size() };
    // auto first{ size - 1 };
    auto first{ 0 };
    auto find_function = [&first](auto& p) {
        first++;
        return p.is_active();
    };

    std::find_if(population.begin(), population.end(), find_function);

    return population.size() == first ? first - 1 : first;
}

std::vector<std::vector<double>>
calculate_infection_sample(const unsigned int duration,
                           const unsigned int susceptible_max_size,
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
    Statistics<double> r_0_stat(duration, 0.0);

    real_uniform_t dis(0.0, 1.0);
    integer_uniform_t i_dis(0, susceptible_max_size + i0active + i0recovered);

    auto S{ susceptible_max_size - i0active - i0recovered };
    auto I{ 0u };

    Population population(gen, S);

    population.seed_infected(
      i0active, i0recovered, percentage_in_quarantine, max_transmission_day);

    for (int day = 0; day < duration; day++) {
        I = population.size();

        infected_stat.add_value(day, static_cast<double>(I));
        susceptible_stat.add_value(day, static_cast<double>(S));

        for (auto ind{ population.first_subject() }; ind < I; ind++) {
            auto& person = population[ind];

            if (person.is_active()) {
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

    res.push_back(infected_stat.get_mean());  // 0
    res.push_back(infected_stat.get_m2());    // 1
    res.push_back(infected_stat.get_count()); // 2

    res.push_back(susceptible_stat.get_mean());  // 3
    res.push_back(susceptible_stat.get_m2());    // 4
    res.push_back(susceptible_stat.get_count()); // 5

    res.push_back(r_0_stat.get_mean());  // 6
    res.push_back(r_0_stat.get_m2());    // 7
    res.push_back(r_0_stat.get_count()); // 8

    res.push_back(inf_dyn_stat.get_mean());  // 9
    res.push_back(inf_dyn_stat.get_m2());    // 10
    res.push_back(inf_dyn_stat.get_count()); // 11

    return res;
}

std::vector<std::vector<double>>
calculate_infection_parallel(const int duration,
                             const int susceptible_max_size,
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
                                     susceptible_max_size,
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
                r_0_stat.add_value(d, ret[6][d]);
                inf_dyn_stat.add_value(d, ret[9][d]);
            }
        }
    }

    std::vector<std::vector<double>> res;

    res.push_back(infected_stat.get_mean());  // 0
    res.push_back(infected_stat.get_m2());    // 1
    res.push_back(infected_stat.get_count()); // 2

    res.push_back(susceptible_stat.get_mean());  // 3
    res.push_back(susceptible_stat.get_m2());    // 4
    res.push_back(susceptible_stat.get_count()); // 5

    res.push_back(r_0_stat.get_mean());  // 6
    res.push_back(r_0_stat.get_m2());    // 7
    res.push_back(r_0_stat.get_count()); // 8

    res.push_back(inf_dyn_stat.get_mean());  // 9
    res.push_back(inf_dyn_stat.get_m2());    // 10
    res.push_back(inf_dyn_stat.get_count()); // 11

    return res;
}

std::vector<std::vector<double>>
calculate_infection(const int duration,
                    const int susceptible_max_size,
                    const int i0active,
                    const int i0recovered,
                    const int samples,
                    const int max_transmission_day,
                    const int max_in_quarantine,
                    const double gamma,
                    const double percentage_in_quarantine)
{
    auto inf_dyn = std::make_shared<InfectionDynamics>(gamma, duration);

    return calculate_infection_parallel(duration,
                                        susceptible_max_size,
                                        i0active,
                                        i0recovered,
                                        samples,
                                        max_transmission_day,
                                        max_in_quarantine,
                                        gamma,
                                        percentage_in_quarantine,
                                        inf_dyn);
}

std::vector<std::vector<double>>
calculate_infection_with_vaccine(const int duration,
                                 const int susceptible_max_size,
                                 const int i0active,
                                 const int i0recovered,
                                 const int samples,
                                 const int max_transmission_day,
                                 const int max_in_quarantine,
                                 const double gamma,
                                 const double percentage_in_quarantine,
                                 const double vaccinated_share,
                                 const double vaccine_efficacy)
{
    auto inf_dyn = std::make_shared<VaccineInfectionDynamics>(
      gamma, duration, vaccinated_share, vaccine_efficacy);

    return calculate_infection_parallel(duration,
                                        susceptible_max_size,
                                        i0active,
                                        i0recovered,
                                        samples,
                                        max_transmission_day,
                                        max_in_quarantine,
                                        gamma,
                                        percentage_in_quarantine,
                                        inf_dyn);
}
