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

#include <cmath>
#include <concepts>
#include <type_traits>
#include <vector>
#define NDEBUG
#include <cassert>

template<typename T>
concept Real = std::is_floating_point_v<T>;

template<Real N>
class Statistics
{
    std::vector<N> mean;
    std::vector<N> m2;
    std::vector<N> count;
    unsigned int sz;

  public:
    Statistics(const int n, N val)
      : sz(n)
    {
        for (auto i{ 0u }; i < sz; i++) {
            mean.push_back(val);
            m2.push_back(val);
            count.push_back(val);
        }
        return;
    }

    const int size() noexcept { return sz; }
    std::vector<N> get_mean() noexcept { return mean; }
    std::vector<N> get_m2() noexcept { return m2; }
    std::vector<N> get_count() noexcept { return count; }

    void add_value(const unsigned int id, const N value) noexcept
    {
        assert(id < sz);

        N delta = value - mean[id];
        count[id] += 1.0;
        mean[id] += delta / count[id];
        N delta2 = value - mean[id];
        m2[id] += delta * delta2;

        return;
    }
};
