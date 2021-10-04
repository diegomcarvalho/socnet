///////////////////////////////////////////////////////////////////////////////
/// The Statistics Template
///     Statistics accumulation template
/// @file statistics.hpp
/// @brief The Statistics Template
/// @author Diego Carvalho - d.carvalho@ieee.org
/// @date 2021-08-21
/// @version 1.0 2021/08/21
///////////////////////////////////////////////////////////////////////////////
#pragma once

#define _DOSNT_HAS_CONCEPTS_

#include <cassert>
#include <cmath>
#include <type_traits>
#include <vector>

#ifdef _HAS_CONCEPTS_
#include <concepts>
template<typename T>
concept Real = std::is_floating_point_v<T>;

template<Real N>
#else
template<typename N>
#endif
class Statistics
{
    std::vector<N> first_momentum;
    std::vector<N> second_momentum;
    std::vector<N> count;
    unsigned int vector_size;

  public:
    Statistics(const int internal_size, N init_value)
      : vector_size(internal_size)
    {
        for (auto i{ 0u }; i < vector_size; i++) {
            first_momentum.push_back(init_value);
            second_momentum.push_back(init_value);
            count.push_back(init_value);
        }
        return;
    }
    ~Statistics() noexcept = default;

    inline const int size() noexcept { return this->vector_size; }
    inline const std::vector<N>& get_mean() noexcept
    {
        return this->first_momentum;
    }
    inline const std::vector<N>& get_variance() noexcept
    {
        return this->second_momentum;
    }
    inline const std::vector<N>& get_count() noexcept { return this->count; }

    inline void add_value(const unsigned int id, const N current_value) noexcept
    {
        assert(id < vector_size);

        N delta = current_value - this->first_momentum[id];
        this->count[id] += 1.0;
        this->first_momentum[id] += delta / this->count[id];
        N delta2 = current_value - this->first_momentum[id];
        this->second_momentum[id] += delta * delta2;

        return;
    }
};
