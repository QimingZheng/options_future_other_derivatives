#include <math.h>

#include <iostream>

float ForwardPrice(float asset_spot_price,
                   float risk_free_continuous_compounding_interest_rate,
                   float term) {
  return asset_spot_price *
         exp(risk_free_continuous_compounding_interest_rate * term);
}

float ForexForwadPrice(
    float currency_spot_price,
    float currency_continuous_compounding_risk_free_interest_rate,
    float dollar_continuous_compounding_risk_free_interest_rate, float term) {
  float currency_forward_price =
      currency_spot_price *
      exp((dollar_continuous_compounding_risk_free_interest_rate -
           currency_continuous_compounding_risk_free_interest_rate) *
          term);
  return currency_forward_price;
}

int main() {
  std::cout << ForwardPrice(40.0, 0.05, 0.25) << std::endl;
  std::cout << ForexForwadPrice(.75, 0.03, 0.01, 2) << std::endl;
  return 0;
}