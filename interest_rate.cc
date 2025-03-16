#include <math.h>

#include <iostream>

float ContinuousCompoundingInterestRate(float equivalent_annual_interest_rate,
                                        int compound_frequency) {
  return compound_frequency *
         log(1. + (equivalent_annual_interest_rate / compound_frequency));
}

float CompoundInterestRate(float continuous_compounding_interest_rate,
                           int compound_frequency) {
  return compound_frequency *
         (exp(continuous_compounding_interest_rate / compound_frequency) - 1.);
}

float BondYield(std::vector<std::pair<float, float>> /* pairs of <cash, term> */
                    cash_flows,
                float price) {
  auto calculate_price_at_yield = [&](float yield_test) -> float {
    auto sum_ = 0.0;
    for (auto cash_flow : cash_flows) {
      sum_ += cash_flow.first * exp(-yield_test * cash_flow.second);
    }
    return sum_ - price;
  };
  auto calculate_price_partial_derivative_to_yield =
      [&](float yield_test) -> float {
    auto sum_ = 0.0;
    for (auto cash_flow : cash_flows) {
      sum_ += -cash_flow.first * cash_flow.second *
              exp(-yield_test * cash_flow.second);
    }
    return sum_;
  };
  // Newton-Raphson method.
  float yield_test = 0.1;
  constexpr float epsilon = 1e-5;
  while (abs(calculate_price_at_yield(yield_test)) > epsilon) {
    yield_test = yield_test -
                 calculate_price_at_yield(yield_test) /
                     calculate_price_partial_derivative_to_yield(yield_test);
  }
  return yield_test;
}

float BondDuration(
    std::vector<std::pair<float, float>> /* pairs of <cash, term> */
        cash_flows,
    float continuous_compounding_interest_rate, float price) {
  auto sum_ = 0.0;
  for (auto cash_flow : cash_flows) {
    sum_ += cash_flow.first * cash_flow.second *
            exp(-continuous_compounding_interest_rate * cash_flow.second);
  }
  return sum_ / price;
}

float BondYieldConvexity(
    std::vector<std::pair<float, float>> /* pairs of <cash, term> */
        cash_flows,
    float continuous_compounding_interest_rate, float price) {
  auto sum_ = 0.0;
  for (auto cash_flow : cash_flows) {
    sum_ += cash_flow.first * cash_flow.second * cash_flow.second *
            exp(-continuous_compounding_interest_rate * cash_flow.second);
  }
  return sum_ / price;
}

int main() {
  std::cout << ContinuousCompoundingInterestRate(0.1, 2) << std::endl;
  std::cout << CompoundInterestRate(0.08, 4) << std::endl;
  std::cout << BondYield({{3, 0.5}, {3, 1}, {3, 1.5}, {103, 2}}, 98.39)
            << std::endl;
  std::cout << BondDuration(
                   {{5, 0.5}, {5, 1}, {5, 1.5}, {5, 2}, {5, 2.5}, {105, 3}},
                   BondYield(
                       {{5, 0.5}, {5, 1}, {5, 1.5}, {5, 2}, {5, 2.5}, {105, 3}},
                       94.213),
                   94.213)
            << std::endl;
  std::cout << BondYieldConvexity(
                   {{5, 0.5}, {5, 1}, {5, 1.5}, {5, 2}, {5, 2.5}, {105, 3}},
                   BondYield(
                       {{5, 0.5}, {5, 1}, {5, 1.5}, {5, 2}, {5, 2.5}, {105, 3}},
                       94.213),
                   94.213)
            << std::endl;
  return 0;
}