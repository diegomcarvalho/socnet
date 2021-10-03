#include "version.h"
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>

#include <cstdlib>
#include <string>
#include <thread>

#include "dynamics.hpp"

namespace py = pybind11;

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
                             std::shared_ptr<InfectionDynamics> inf_dyn);

extern int number_of_threads;

inline bool
is_number(const std::string& s)
{
    return !s.empty() && std::all_of(s.begin(), s.end(), ::isdigit);
}

auto
init_module()
{
    if (const char* env_p = std::getenv("SOCNET_NUM_THREADS")) {
        if (std::string snt(env_p); snt == "CPU_MAX") {
            number_of_threads = std::thread::hardware_concurrency();
        } else if (is_number(snt)) {
            number_of_threads = std::stoi(snt);
        }
    }
    return;
}

auto
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

auto
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

PYBIND11_MODULE(socnet, m)
{
    m.doc() = "socnet implemented in C++ - v2.0"; // optional module docstring

    m.def(
      "init_module", &init_module, "Initialize the Random Number Generator.");

    m.def("calculate_infection",
          &calculate_infection,
          "Simulate the Social Network Model for SIRE dynamics.\n"
          "Parameters:\n"
          " arg0: const int duration,\n"
          " arg1: const int susceptible_max_size,\n"
          " arg2: const int i0active,\n"
          " arg3: const int i0recovered,\n"
          " arg4: const int samples,\n"
          " arg5: const int max_transmission_day,\n"
          " arg6: const int max_in_quarantine,\n"
          " arg7: const double gamma,\n"
          " arg8: const double percentage_in_quarantine\n"
          "Return value: ret = list(list())\n"
          " ret[0]: infected (mean)\n"
          " ret[1]: infected (standard deviation)\n"
          " ret[2]: infected (count per day)\n"
          " ret[3]: susceptible (mean)\n"
          " ret[4]: susceptible (standard deviation)\n"
          " ret[5]: susceptible (count per day)\n"
          " ret[6]: recovered (mean)\n"
          " ret[7]: recovered (standard deviation)\n"
          " ret[8]: recovered (count per day)\n"
          " ret[9]: R0 (mean)\n"
          " ret[10]: R0 (standard deviation)\n"
          " ret[11]: R0 (count per day)\n",
          " ret[12]: inf_dyn_stat (mean)\n"
          " ret[13]: inf_dyn_stat (standard deviation)\n"
          " ret[14]]: inf_dyn_stat (count per day)\n");

    m.def(
      "calculate_infection_with_vaccine",
      &calculate_infection_with_vaccine,
      "Simulate the Social Network Model for SIRE dynamics with vaccination."
      "Parameters:\n"
      " arg0: const int duration,\n"
      " arg1: const int susceptible_max_size,\n"
      " arg2: const int i0active,\n"
      " arg3: const int i0recovered,\n"
      " arg4: const int samples,\n"
      " arg5: const int max_transmission_day,\n"
      " arg6: const int max_in_quarantine,\n"
      " arg7: const double gamma,\n"
      " arg8: const double percentage_in_quarantine\n"
      " arg9: const double vaccinated_share\n"
      " arg10:const double vaccine_efficacy\n"
      "Return value: ret = list(list())\n"
      " ret[0]: infected (mean)\n"
      " ret[1]: infected (standard deviation)\n"
      " ret[2]: infected (count per day)\n"
      " ret[3]: susceptible (mean)\n"
      " ret[4]: susceptible (standard deviation)\n"
      " ret[5]: susceptible (count per day)\n"
      " ret[6]: recovered (mean)\n"
      " ret[7]: recovered (standard deviation)\n"
      " ret[8]: recovered (count per day)\n"
      " ret[9]: R0 (mean)\n"
      " ret[10]: R0 (standard deviation)\n"
      " ret[11]: R0 (count per day)\n",
      " ret[12]: inf_dyn_stat (mean)\n"
      " ret[13]: inf_dyn_stat (standard deviation)\n"
      " ret[14]]: inf_dyn_stat (count per day)\n");
}