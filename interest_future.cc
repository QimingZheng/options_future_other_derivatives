#include <iostream>
// Price sensitivity hedge ratio.
float DurationBasedHedgeRatio(float interest_future_contract_price,
                              float duration_of_future_asset_on_due_day,
                              float future_value_of_hedged_bond_combinations,
                              float duration_of_hedged_bond_combinations) {
  return future_value_of_hedged_bond_combinations *
         duration_of_hedged_bond_combinations /
         (interest_future_contract_price * duration_of_future_asset_on_due_day);
}

int main() {
  std::cout << DurationBasedHedgeRatio(93062.5, 9.2, 10000000,  6.8)
            << std::endl;
  return 0;
}