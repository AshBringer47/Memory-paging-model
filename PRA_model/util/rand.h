#ifndef RAND_H
#define RAND_H

#include "range.h"

#include <random>

class Rand
{
    Rand();

public:
    static bool boolean();

    static bool trueWithChance(double chance);

    // [min; max], inclusive
    static int intNumber(range<int> r);
    static int intNumber(int min, int max);

    static double doubleNumber(double min = 0, double max = 1);

    static double normalNumber(double mean, double stddev);

private:
    static std::random_device::result_type getSeed();

    static std::mt19937 randomNumberGenerator;
};

#endif // RAND_H
