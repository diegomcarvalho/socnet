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

#include "population.hpp"
#include "statistics.hpp"

std::mt19937_64 my_gen; // Standard mersenne_twister_engine seeded with rd()

inline int
powerlaw(double gamma, double ran)
{
    return static_cast<int>((pow(ran, (-1.0 / gamma))) - 0.5);
}

inline int
find_first(Population& population)
{
    int size = population.size();
    int first = size - 1;

    for (int k = 0; k < size; k++) {
        if (population[k].is_active()) {
            first = k;
            break;
        }
    }
    return first;
}

void
init_module()
{
    // std::cout << "calculate.cc - random setup done." << std::endl;
    my_gen.seed(100);
    return;
}

int
vaccinate(const int individuals,
          const double vaccinated_percentage,
          const double vaccine_efficacy)
{
    int immune_individuals = 0;
    double real_efficacy = vaccinated_percentage * vaccine_efficacy;

    real_uniform_t dis(0.0, 1.0);

    for (auto i = 0; i < individuals; i++) {
        immune_individuals += (dis(my_gen) < real_efficacy);
    }
    return immune_individuals;
}

std::vector<std::vector<double>>
calculate_infection_sample(const int duration,
                           const int susceptible_max_size,
                           const int i0active,
                           const int i0recovered,
                           const int max_transmission_day,
                           const int max_in_quarantine,
                           const double gamma,
                           const double percentage_in_quarantine,
                           real_uniform_t dis,
                           integer_uniform_t i_dis)
{
    int S{ susceptible_max_size - i0active - i0recovered };
    int I{ 0 };

    Population population(S);

    Statistics<double> infected_stat(duration, 0.0);
    Statistics<double> susceptible_stat(duration, 0.0);
    Statistics<double> r_0_stat(duration, 0.0);

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
                    auto available_new_infected{ powerlaw(gamma, dis(my_gen)) };

                    if (!available_new_infected)
                        continue;

                    if (person.is_quarantined())
                        available_new_infected =
                          std::min(max_in_quarantine - person.decendants,
                                   available_new_infected);

                    auto new_infected{ 0 };
                    for (auto ni{ 0 }; ni < available_new_infected; ni++) {
                        // Check if the individual belongs to S, and
                        if ((i_dis(my_gen) < S) && (S > 0)) {
                            new_infected++;
                            S--;
                            population.new_subject(
                              0,
                              ind,
                              day,
                              true,
                              (dis(my_gen) < percentage_in_quarantine));
                        }
                    }
                    person.decendants += new_infected;
                } else {
                    person.clear_active();
                    if (population.first_subject() == ind)
                        population.move_first(ind + 1);
                }
            }
        }

        // /* auto& person = population[ui]; */
        // for (auto ui{ 0u }; ui < population.size(); ui++) {

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

    res.push_back(infected_stat.get_mean());  // 0
    res.push_back(infected_stat.get_m2());    // 1
    res.push_back(infected_stat.get_count()); // 2

    res.push_back(susceptible_stat.get_mean());  // 3
    res.push_back(susceptible_stat.get_m2());    // 4
    res.push_back(susceptible_stat.get_count()); // 5

    res.push_back(r_0_stat.get_mean());  // 6
    res.push_back(r_0_stat.get_m2());    // 7
    res.push_back(r_0_stat.get_count()); // 8

    return res;
}

std::vector<std::vector<double>>
calculate_infection_with_vaccine_sample(const int duration,
                                        const int susceptible_max_size,
                                        const int i0active,
                                        const int i0recovered,
                                        const int max_transmission_day,
                                        const int max_in_quarantine,
                                        const double gamma,
                                        const double percentage_in_quarantine,
                                        const double vaccinated_share,
                                        const double vaccine_efficacy,
                                        real_uniform_t dis,
                                        integer_uniform_t i_dis)
{
    int S{ susceptible_max_size - i0active - i0recovered };
    int I{ 0 };

    Population population(susceptible_max_size + i0active + i0recovered);

    Statistics<double> infected_stat(duration, 0.0);
    Statistics<double> susceptible_stat(duration, 0.0);
    Statistics<double> r_0_stat(duration, 0.0);

    population.seed_infected(
      i0active, i0recovered, percentage_in_quarantine, max_transmission_day);

    for (auto day{ 0 }; day < duration; day++) {
        I = population.size();

        infected_stat.add_value(day, double(I));
        susceptible_stat.add_value(day, double(S));

        for (auto ind{ population.first_subject() }; ind < I; ind++) {
            auto& person = population[ind];

            if (person.is_active()) {
                if (person.days_of_infection < max_transmission_day) {
                    person.days_of_infection++;
                    int available_new_infected{ powerlaw(gamma, dis(my_gen)) };
                    available_new_infected -= vaccinate(available_new_infected,
                                                        vaccinated_share,
                                                        vaccine_efficacy);
                    if (!available_new_infected)
                        continue;

                    if (person.is_quarantined())
                        available_new_infected =
                          std::min(max_in_quarantine - person.decendants,
                                   available_new_infected);

                    auto new_infected{ 0 };
                    for (auto ni{ 0 }; ni < available_new_infected; ni++) {
                        // Check if the individual belongs to S, and
                        if ((i_dis(my_gen) < S) && (S > 0)) {
                            new_infected++;
                            S--;
                            population.new_subject(
                              0,
                              ind,
                              day,
                              true,
                              (dis(my_gen) < percentage_in_quarantine));
                        }
                    }
                    person.decendants += new_infected;
                } else {
                    person.clear_active();
                    if (population.first_subject() == ind)
                        population.move_first(ind + 1);
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

    res.push_back(infected_stat.get_mean());  // 0
    res.push_back(infected_stat.get_m2());    // 1
    res.push_back(infected_stat.get_count()); // 2

    res.push_back(susceptible_stat.get_mean());  // 3
    res.push_back(susceptible_stat.get_m2());    // 4
    res.push_back(susceptible_stat.get_count()); // 5

    res.push_back(r_0_stat.get_mean());  // 6
    res.push_back(r_0_stat.get_m2());    // 7
    res.push_back(r_0_stat.get_count()); // 8

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
    real_uniform_t dis(0.0, 1.0);
    integer_uniform_t i_dis(0, susceptible_max_size + i0active + i0recovered);

    Statistics<double> infected_stat(duration, 0.0);
    Statistics<double> susceptible_stat(duration, 0.0);
    Statistics<double> r_0_stat(duration, 0.0);

    const auto div{ std::thread::hardware_concurrency() };

    for (auto k{ 0 }; k < samples / div; k++) {
        std::vector<std::future<std::vector<std::vector<double>>>> fut;

        for (int i = 0; i < div; i++)
            fut.push_back(std::async(calculate_infection_sample,
                                     duration,
                                     susceptible_max_size,
                                     i0active,
                                     i0recovered,
                                     max_transmission_day,
                                     max_in_quarantine,
                                     gamma,
                                     percentage_in_quarantine,
                                     dis,
                                     i_dis));

        for (auto& it : fut) {
            auto ret = it.get();
            for (auto d{ 0 }; d < duration; d++) {
                infected_stat.add_value(d, ret[0][d]);
                susceptible_stat.add_value(d, ret[3][d]);
                r_0_stat.add_value(d, ret[6][d]);
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

    return res;
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
    real_uniform_t dis(0.0, 1.0);
    integer_uniform_t i_dis(0, susceptible_max_size + i0active + i0recovered);

    Statistics<double> infected_stat(duration, 0.0);
    Statistics<double> susceptible_stat(duration, 0.0);
    Statistics<double> r_0_stat(duration, 0.0);

    const auto div{ std::thread::hardware_concurrency() };

    for (int k = 0; k < samples / div; k++) {
        std::vector<std::future<std::vector<std::vector<double>>>> fut;

        for (auto i{ 0 }; i < div; i++)
            fut.push_back(std::async(calculate_infection_with_vaccine_sample,
                                     duration,
                                     susceptible_max_size,
                                     i0active,
                                     i0recovered,
                                     max_transmission_day,
                                     max_in_quarantine,
                                     gamma,
                                     percentage_in_quarantine,
                                     vaccinated_share,
                                     vaccine_efficacy,
                                     dis,
                                     i_dis));

        for (auto& it : fut) {
            auto ret = it.get();
            for (auto d{ 0 }; d < duration; d++) {
                infected_stat.add_value(d, ret[0][d]);
                susceptible_stat.add_value(d, ret[3][d]);
                r_0_stat.add_value(d, ret[6][d]);
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

    return res;
}
