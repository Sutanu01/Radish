#include "math_logic.hpp"
#include <cmath>
#include <algorithm>

using namespace std;

namespace RatingMath {

    void FFT::precomp_roots(int logn) {
        w.resize(logn + 1);
        w[0].resize(1, 1);        
        const double PI = acos(-1.0);

        for (int i = 1; i <= logn; i++) {
            int n = (1 << i);
            w[i].resize(n / 2);
            cmplx fw = cmplx(cos(2 * PI / n), sin(2 * PI / n));
            
            for (int j = 0; j < (n >> 1); j++) {
                w[i][j] = w[i - 1][j >> 1];
                if (j & 1) w[i][j] *= fw;
            }
        }
    }

    void FFT::fft_internal(vector<cmplx>& p, bool inv) {
        int n = p.size();
        int logn = 0;
        while ((1 << logn) != n) logn++;
        vector<int> bitrev(n, 0);
        for (int i = 1; i < n; i++) {
            bitrev[i] = (bitrev[i >> 1] >> 1) | ((i & 1) << (logn - 1));
        }
        for (int i = 0; i < n; i++) {
            if (i < bitrev[i]) swap(p[i], p[bitrev[i]]);
        }
        for (int lvl = 1; lvl <= logn; lvl++) {
            int len = (1 << lvl);
            int half = (len >> 1);
            for (int i = 0; i < n; i += len) {
                for (int j = 0; j < half; j++) {
                    cmplx angl = inv ? conj(w[lvl][j]) : w[lvl][j];  
                    cmplx pos = p[i + j] + angl * p[i + j + half];
                    cmplx neg = p[i + j] - angl * p[i + j + half];
                    p[i + j] = pos;
                    p[i + j + half] = neg;
                }
            }
        }    
        if (inv) {
            for (int i = 0; i < n; i++) {
                p[i] /= n;
            }
        }
    }

    void FFT::convolution(const vector<double>& p, const vector<double>& q) {
        int m = p.size() + q.size() - 1;
        int logn = 0;
        while ((1 << logn) <= m) logn++;
        int n = (1 << logn);
        precomp_roots(logn);
        vector<cmplx> rc(n);
        for (int i = 0; i < n; ++i) {
            rc[i] = cmplx(i < (int)p.size() ? p[i] : 0, i < (int)q.size() ? q[i] : 0);
        }
        fft_internal(rc);
        for (int i = 0; i < n; i++) rc[i] *= rc[i];
        fft_internal(rc, true);
        vector<double> res(m);
        for (int i = 0; i < m; i++) {
            res[i] = rc[i].imag() / 2.0;
        }
        S.resize(MAX_RATING + 1, 0);
        for (int i = 0; i <= MAX_RATING; i++) {
            S[i] = (i + MAX_RATING < (int)res.size()) ? res[i + MAX_RATING] : 0;
        }
    }

    double prob_func(int diff) {
        double pwr = diff / 400.0;
        return 1.0 / (1 + powl(10.0, pwr));
    }
    double GetSeed(const vector<double>& S, int rating) {
        return 0.5 + S[rating];
    }
    double GetMeanRank(double seed, int actualRank) {
        double meanSquare = sqrtl(seed*actualRank);
        return meanSquare;
    }
    int FindRatingForSeed(const vector<double>& S, double targetSeed) {
        int l = 0, r = MAX_RATING;
        int targetRating = 0;
        while (l <= r) {
            int mid = (l + r) >> 1;
            double expectedRank = GetSeed(S, mid);
            if (expectedRank >= targetSeed) {
                targetRating = mid;
                l = mid + 1;
            } else {
                r = mid - 1;
            }
        }
        return targetRating;
    }
    int ComputePerformance(const vector<double>& S, int actualRank) {
        return FindRatingForSeed(S, actualRank);
    }
    int ComputeDelta(const vector<double>& S, int currentRating, int actualRank) {
        double seed = GetSeed(S, currentRating);
        double meanRank = GetMeanRank(seed, actualRank);
        int predictedRating = FindRatingForSeed(S, meanRank);
        return (predictedRating - currentRating) / 2;
    }
}