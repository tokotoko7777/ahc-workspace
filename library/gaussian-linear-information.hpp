#pragma once
#include <span>
#include <utility>

namespace gaussian_linear_information {
struct Reduction {
    double signal_variance;
    double trace_drop;
};
// Covariance is a symmetric PSD, row-major n-by-n matrix;
// feature holds unique valid indices with their nonzero coefficients.
// A hypothetical observation has independent additive noise variance r>0.
// Does not change the covariance. Cost O(n * feature.size()).
inline Reduction expected_reduction(int n, std::span<const double> covariance,
                                    std::span<const std::pair<int,double>> feature,
                                    double noise_variance) {
    double signal = 0.0, squared = 0.0;
    for (auto [j, x] : feature) {
        double ax = 0.0;
        for (auto [k, y] : feature) ax += covariance[j * n + k] * y;
        signal += x * ax;
    }
    for (int i = 0; i < n; ++i) {
        double ax = 0.0;
        for (auto [j, x] : feature) ax += covariance[i * n + j] * x;
        squared += ax * ax;
    }
    return {signal, squared / (noise_variance + signal)};
}
// Sparse linear target coefficients can be signed (e.g. one route minus
// another). Given y = feature^T theta + noise, return how much one
// observation reduces Var(target^T theta). No dense matrix update is needed.
inline double target_variance_drop(int n, std::span<const double> covariance,
                                   std::span<const std::pair<int,double>> feature,
                                   std::span<const std::pair<int,double>> target,
                                   double noise_variance) {
    auto dot_row = [&](int i) {
        double value = 0.0;
        for (auto [j, x] : feature) value += covariance[i * n + j] * x;
        return value;
    };
    double variance = noise_variance, cross = 0.0;
    for (auto [i, x] : feature) variance += x * dot_row(i);
    for (auto [i, d] : target) cross += d * dot_row(i);
    return cross * cross / variance;
}

}
