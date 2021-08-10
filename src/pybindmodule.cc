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
          "Simulate the Social Network Model for SIRE dynamics.");

    m.def(
      "calculate_infection_with_vaccine",
      &calculate_infection_with_vaccine,
      "Simulate the Social Network Model for SIRE dynamics with vaccination.");
}
