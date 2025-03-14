
#include <assert.h>
#include <math.h>

#include <iostream>
#include <vector>

float HedgeRatio(std::vector<float>& delta_spot,
                 std::vector<float>& delta_future) {
  auto mean = [=](std::vector<float>& x) {
    float _sum = 0.;
    for (auto e : x) _sum += e;
    return _sum * 1.0 / x.size();
  };
  auto covariance = [=](std::vector<float>& a, std::vector<float>& b) -> float {
    assert(a.size() == b.size());
    auto vec = std::vector<float>(a.size());
    for (auto i = 0; i < vec.size(); i++) vec[i] = a[i] * b[i];
    return mean(vec) - mean(a) * mean(b);
  };
  auto stdev = [=](std::vector<float>& x) -> float {
    auto _mean = mean(x);
    float _sum = 0.;
    for (auto e : x) _sum += (e - _mean) * (e - _mean);
    return sqrt(_sum * 1.0 / x.size());
  };
  auto correlation = [=](std::vector<float>& a,
                         std::vector<float>& b) -> float {
    return covariance(a, b) / (stdev(a) * stdev(b));
  };
  return correlation(delta_spot, delta_future) * stdev(delta_spot) /
         stdev(delta_future);
}

int main() {
  std::vector<float> future_deltas = {.021,  .035,  -.046, .001,  .044,
                                      -.029, -.026, -.029, .048,  -.006,
                                      -.036, -.011, .019,  -.027, .029};
  std::vector<float> spot_deltas = {.029,  .020,  -.044, .008,  .026,
                                    -.019, -.010, -.007, .043,  .011,
                                    -.036, -.018, .009,  -.032, .023};
  std::cout << HedgeRatio(spot_deltas, future_deltas) << std::endl;

  return 0;
}