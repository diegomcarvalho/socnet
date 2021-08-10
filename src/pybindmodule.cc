#include "version.h"
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>

namespace py = pybind11;

void
init_module();

std::vector<std::vector<double>>
calculate_infection(const int duration,
                    const int susceptible_max_size,
                    const int i0active,
                    const int i0recovered,
                    const int samples,
                    const int max_transmission_day,
                    const int max_in_quarantine,
                    const double gamma,
                    const double percentage_in_quarantine);

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
                                 const double vaccine_efficacy);

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
          " ret[6]: R0 (mean)\n"
          " ret[7]: R0 (standard deviation)\n"
          " ret[8]: R0 (count per day)\n");

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
      " ret[6]: R0 (mean)\n"
      " ret[7]: R0 (standard deviation)\n"
      " ret[8]: R0 (count per day)\n");
}
