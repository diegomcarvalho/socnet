#include "population.hpp"

extern std::mt19937_64 my_gen;

class InfectionDynamics
{
  protected:
    const double gamma;

  public:
    InfectionDynamics(const double g)
      : gamma(g)
    {}
    virtual ~InfectionDynamics() {}
    virtual unsigned int infected(const int day,
                                  const int ind,
                                  const double ran)
    {
        return static_cast<int>((pow(ran, (-1.0 / gamma))) - 0.5);
    }
};

class VaccineInfectionDynamics : public InfectionDynamics
{
  protected:
    const double real_efficacy;
    real_uniform_t dis;

  public:
    VaccineInfectionDynamics(const double g, const double vs, const double ve)
      : InfectionDynamics(g)
      , real_efficacy(vs * ve)
      , dis(0.0, 1.0)
    {}
    ~VaccineInfectionDynamics() override {}
    unsigned int infected(const int day,
                          const int ind,
                          const double ran) override
    {
        auto immune_individuals{ 0 };
        auto individuals{ static_cast<int>((pow(ran, (-1.0 / this->gamma))) -
                                           0.5) };

        for (auto i = 0; i < individuals; i++) {
            immune_individuals +=
              static_cast<int>(this->real_efficacy > dis(my_gen));
        }
        return immune_individuals;
    }
};

class ProgressiveVaccineInfectionDynamics : public InfectionDynamics
{
  protected:
    const double real_efficacy;
    const double simulation_range;
    real_uniform_t dis;

  public:
    ProgressiveVaccineInfectionDynamics(const double g,
                                        const double vs,
                                        const double ve,
                                        const int sr)
      : InfectionDynamics(g)
      , real_efficacy(vs * ve)
      , simulation_range(sr)
      , dis(0.0, 1.0)
    {}
    ~ProgressiveVaccineInfectionDynamics() override {}
    unsigned int infected(const int day,
                          const int ind,
                          const double ran) override
    {
        auto immune_individuals{ 0 };
        auto factor{ static_cast<double>(day) / this->simulation_range };
        auto individuals{ static_cast<int>((pow(ran, (-1.0 / this->gamma))) -
                                           0.5) };

        for (auto i = 0; i < individuals; i++) {
            immune_individuals +=
              static_cast<int>((factor * this->real_efficacy) > dis(my_gen));
        }
        return immune_individuals;
    }
};
