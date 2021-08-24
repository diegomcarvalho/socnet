///////////////////////////////////////////////////////////////////////////////
/// The SlotMachine Class
///     Implements several random number generators for a multithreaded
///     simulation.
/// @file slotmachine.hpp
/// @brief The SlotMachine Class
/// @author Diego Carvalho - d.carvalho@ieee.org
/// @date 2021-08-21
/// @version 1.0 2021/08/21
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <memory>
#include <random>

using generator_t = std::mt19937_64;
using real_uniform_t = std::uniform_real_distribution<>;
using integer_uniform_t = std::uniform_int_distribution<>;
using normal_t = std::normal_distribution<>;

class SlotMachine
{
  private:
    std::vector<std::shared_ptr<generator_t>> generator_pool;

  public:
    SlotMachine(const int pool_size)
    {
        for (auto i{ 0 }; i < pool_size; i++) {
            auto gen_ptr{ std::make_shared<generator_t>() };
            gen_ptr->seed(5489ULL + i);
            generator_pool.push_back(gen_ptr);
        }
    }

    auto get_random(const int id) noexcept { return generator_pool[id]; }
};