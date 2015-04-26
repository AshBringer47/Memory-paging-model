#include "rand.h"

#include <chrono>

std::mt19937 Rand::randomNumberGenerator( Rand::getSeed());

bool Rand::boolean()
{
    std::bernoulli_distribution uid;
    return uid(randomNumberGenerator);
}

bool Rand::trueWithChance(double chance)
{
    if(chance <= 0) {
        return false;
    } else if(chance >= 1) {
        return true;
    }

    return std::generate_canonical<double, 20>(randomNumberGenerator) < chance;
}

int Rand::intNumber(range<int> r)
{
    return intNumber(r.min, r.max);
}

int Rand::intNumber(int min, int max)
{
    std::uniform_int_distribution<int> uid(min, max);
    return uid(randomNumberGenerator);
}

double Rand::doubleNumber(double min, double max)
{
    std::uniform_real_distribution<double> urd(min, max);
    return urd(randomNumberGenerator);
}

double Rand::normalNumber(double mean, double stddev)
{
    std::normal_distribution<double> nd(mean, stddev);
    return nd(randomNumberGenerator);
}

std::random_device::result_type Rand::getSeed()
{
    return std::random_device()();
//    return std::chrono::high_resolution_clock::now().time_since_epoch().count();
}
