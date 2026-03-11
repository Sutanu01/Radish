#pragma once

#include <vector>
#include <complex>

namespace RatingMath {

    constexpr int MAX_RATING = 4500;

    using cmplx = std::complex<double>;

    class FFT {
    private:
        std::vector<std::vector<cmplx>> w;
        void precomp_roots(int logn);
        void fft_internal(std::vector<cmplx>& p, bool inv = false);
    public:
        std::vector<double> S;
         // the summation value we get after convolution for each rating 
        void convolution(const std::vector<double>& p, const std::vector<double>& q);
    };
    double prob_func(int diff);

    double GetSeed(const std::vector<double>& S, int rating);

    double GetMeanRank(double seed, int actualRank);

    int FindRatingForSeed(const std::vector<double>& S, double targetSeed);

    int ComputePerformance(const std::vector<double>& S, int actualRank);

    int ComputeDelta(const std::vector<double>& S, int currentRating, int actualRank);
}