#include "population.hpp"

extern std::mt19937_64 my_gen;

int
powerlaw(double a, double b)
{
    return 0;
}

class InfectionDynamics
{
  public:
    virtual int operator()(const int day, const double gamma, const double ran)
    {
        return static_cast<int>((pow(ran, (-1.0 / gamma))) - 0.5);
    }
};

class VaccineInfectionDynamics : public InfectionDynamics
{
    const double real_efficacy;
    real_uniform_t dis;

  public:
    VaccineInfectionDynamics(const double vs, const double ve)
      : real_efficacy(vs * ve)
      , dis(0.0, 1.0)
    {}
    int operator()(const int day, const double gamma, const double ran)
    {
        auto immune_individuals{ 0 };
        auto individuals{ static_cast<int>((pow(ran, (-1.0 / gamma))) - 0.5) };

        for (auto i = 0; i < individuals; i++) {
            immune_individuals += static_cast<int>(real_efficacy < dis(my_gen));
        }
        return immune_individuals;
    }
};
