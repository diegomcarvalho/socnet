///////////////////////////////////////////////////////////////////////////////
/// The Infection Dynamics Class
///     Models of infections.
/// @file dynamics.hpp
/// @brief The InfectionDynamics Class
/// @author Diego Carvalho - d.carvalho@ieee.org
/// @date 2021-08-21
/// @version 1.0 2021/08/21
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include "population.hpp"
#include "slotmachine.hpp"

class InfectionDynamics
{
  protected:
    const double gamma;
    real_uniform_t rgu;

  public:
    InfectionDynamics(const double g)
      : gamma(g)
      , rgu(0.0, 1.0)
    {}
    virtual ~InfectionDynamics() {}
    virtual unsigned int infected(const int day,
                                  const int ind,
                                  std::shared_ptr<std::mt19937_64> gen)
    {
        return static_cast<int>((pow(rgu(*gen), (-1.0 / gamma))) - 0.5);
    }
    virtual uint8_t tag(const uint32_t ind) { return 0; }
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
                          std::shared_ptr<std::mt19937_64> gen) override
    {
        auto immune_individuals{ 0 };
        auto individuals{ static_cast<int>(
          (pow(rgu(*gen), (-1.0 / this->gamma))) - 0.5) };

        for (auto i = 0; i < individuals; i++) {
            immune_individuals +=
              static_cast<int>(this->real_efficacy > dis(*gen));
        }
        return immune_individuals;
    }
    virtual uint8_t tag(const uint32_t ind) override { return 0; }
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
                          std::shared_ptr<std::mt19937_64> gen) override
    {
        auto immune_individuals{ 0 };
        auto factor{ static_cast<double>(day) / this->simulation_range };
        auto individuals{ static_cast<int>(
          (pow(rgu(*gen), (-1.0 / this->gamma))) - 0.5) };

        for (auto i = 0; i < individuals; i++) {
            immune_individuals +=
              static_cast<int>((factor * this->real_efficacy) > dis(*gen));
        }
        return immune_individuals;
    }
    virtual uint8_t tag(const uint32_t ind) override { return 0; }
};
