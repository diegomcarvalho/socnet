#pragma once

#include <cmath>
#include <vector>
#define NDEBUG
#include <cassert>

template<typename T>
class Statistics
{
    std::vector<T> mean;
    std::vector<T> m2;
    std::vector<T> count;
    unsigned int sz;

  public:
    Statistics(const int n, T val)
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
    std::vector<T> get_mean() noexcept { return mean; }
    std::vector<T> get_m2() noexcept { return m2; }
    std::vector<T> get_count() noexcept { return count; }

    void add_value(const unsigned int id, const T value) noexcept
    {
        assert(id < sz);

        T delta = value - mean[id];
        count[id] += 1.0;
        mean[id] += delta / count[id];
        T delta2 = value - mean[id];
        m2[id] += delta * delta2;

        return;
    }
};