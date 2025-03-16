#include <assert.h>

#include <iostream>
enum class OptionType {
  CALL,
  PUT,
};

enum class BuySide {
  LONG,
  SHORT,
};

enum class OptionClass {
  AMERICA,
  EUROPE,
};

typedef struct {
  int year;
  int month;
  int day;
} Date;

class ReturnCurve {
 public:
  ReturnCurve(std::vector<std::pair<float, float>> turning_point_returns)
      : turning_point_returns_(turning_point_returns) {}

  ReturnCurve operator+(ReturnCurve other) {
    assert(turning_point_returns_.begin()->first ==
           other.turning_point_returns_.begin()->first);
    assert(turning_point_returns_.back().first ==
           other.turning_point_returns_.back().first);
    std::vector<std::pair<float, float>> turning_point_returns;
    {
      auto i = 0, j = 0;
      while (i < turning_point_returns_.size() &&
             j < other.turning_point_returns_.size()) {
        if (turning_point_returns_[i].first ==
            other.turning_point_returns_[j].first) {
          turning_point_returns.push_back(
              {turning_point_returns_[i].first,
               turning_point_returns_[i].second +
                   other.turning_point_returns_[j].second});
          i++;
          j++;
        } else if (turning_point_returns_[i].first <
                   other.turning_point_returns_[j].first) {
          turning_point_returns.push_back(
              {turning_point_returns_[i].first,
               turning_point_returns_[i].second +
                   other.EvaluateReturnAtPrice(
                       turning_point_returns_[i].first)});
          i++;
        } else {
          turning_point_returns.push_back(
              {other.turning_point_returns_[j].first,
               other.turning_point_returns_[j].second +
                   EvaluateReturnAtPrice(
                       other.turning_point_returns_[j].first)});
          j++;
        }
      }
      while (i < turning_point_returns_.size()) {
        turning_point_returns.push_back(turning_point_returns_[i++]);
      }
      while (j < other.turning_point_returns_.size()) {
        turning_point_returns.push_back(other.turning_point_returns_[j++]);
      }
    }
    return ReturnCurve(turning_point_returns);
  }

  void Print() {
    for (auto p : turning_point_returns_) {
      std::cout << p.second << "@" << p.first << " ";
    }
    std::cout << std::endl;
  }

 private:
  float EvaluateReturnAtPrice(float price) {
    for (auto i = 0; i < turning_point_returns_.size() - 1; i++) {
      if (turning_point_returns_[i].first <= price &&
          turning_point_returns_[i + 1].first >= price) {
        return turning_point_returns_[i].second +
               (turning_point_returns_[i + 1].second -
                turning_point_returns_[i].second) *
                   (price - turning_point_returns_[i].first) /
                   (turning_point_returns_[i + 1].first -
                    turning_point_returns_[i].first);
      }
    }
    throw std::runtime_error("Evaluation is outside the range");
  }

  std::vector<std::pair<float, float>> turning_point_returns_;
};

template <OptionType OT, BuySide BS, OptionClass OC,
          int /* due date in days */ DD, int /* strike price in cents */ SP,
          int /* the underlying asset's current price */ AP,
          int /* continuous compounding risk-free interest rate */ CCIR>
class Option {
 public:
  constexpr float GetStrikePrice() const { return SP / 100.0; }

  float IntrinsicValue(float current_price) const {
    if constexpr (OT == OptionType::CALL) {
      if constexpr (BS == BuySide::LONG) {
        return std::max(current_price - GetStrikePrice(), 0.f);
      } else {
        return -std::max(current_price - GetStrikePrice(), 0.f);
      }
    } else {
      if constexpr (BS == BuySide::LONG) {
        return std::max(GetStrikePrice() - current_price, 0.f);
      } else {
        return -std::max(GetStrikePrice() - current_price, 0.f);
      }
    }
  }

  float PriceUpperBond() {
    if constexpr (OT == OptionType::CALL) {
      return AP / 100.0;
    } else {
      constexpr auto upper_bound = AP / 100.0;
      if constexpr (OC == OptionClass::AMERICA) {
        constexpr auto ret = std::min(upper_bound, SP / 100.0);
        return ret;
      } else {
        return std::min(upper_bound,
                        (SP / 100.0) * exp(-(CCIR / 100.0) * DD / 360.0));
      }
    }
  }

  float PriceLowerBound() {
    if constexpr (OT == OptionType::CALL) {
      if constexpr (OC == OptionClass::AMERICA) {
        return std::max(
            AP / 100.0 - (SP / 100.0) * exp(-(CCIR / 100.0) * DD / 360.0), 0.);
      } else {
        return std::max(
            AP / 100.0 - (SP / 100.0) * exp(-(CCIR / 100.0) * DD / 360.0), 0.);
      }
    } else {
      if constexpr (OC == OptionClass::AMERICA) {
        constexpr auto ret = std::max(SP / 100.0 - AP / 100.0, 0.0);
        return ret;
      } else {
        return std::max(
            (SP / 100.0) * exp(-(CCIR / 100.0) * DD / 360.0) - AP / 100.0, 0.);
      }
    }
  }

  ReturnCurve GetReturnCurve() {
    constexpr auto RANGE_MAX = 10000.;
    if constexpr (OT == OptionType::CALL) {
      if constexpr (BS == BuySide::LONG) {
        return ReturnCurve({{0., 0.},
                            {SP / 100.0, 0.},
                            {RANGE_MAX, RANGE_MAX - (SP / 100.0)}});
      } else {
        return ReturnCurve({{0., 0.},
                            {SP / 100.0, 0.},
                            {RANGE_MAX, -RANGE_MAX + (SP / 100.0)}});
      }
    } else {
      if constexpr (BS == BuySide::LONG) {
        return ReturnCurve(
            {{0., SP / 100.0}, {SP / 100.0, 0.}, {RANGE_MAX, 0.}});
      } else {
        return ReturnCurve(
            {{0., -SP / 100.0}, {SP / 100.0, 0.}, {RANGE_MAX, 0.}});
      }
    }
  }
};

int main() {
  auto opt = Option<OptionType::CALL, BuySide::LONG, OptionClass::AMERICA, 30,
                    1800, 2000, 10>();
  std::cout << opt.GetStrikePrice() << std::endl;
  std::cout << opt.IntrinsicValue(11.0) << std::endl;
  std::cout << opt.IntrinsicValue(21.0) << std::endl;
  auto opt2 = Option<OptionType::PUT, BuySide::LONG, OptionClass::AMERICA, 30,
                     1800, 2000, 10>();
  std::cout << opt2.PriceUpperBond() << std::endl;
  auto opt3 = Option<OptionType::CALL, BuySide::LONG, OptionClass::EUROPE, 360,
                     1800, 2000, 10>();
  std::cout << opt3.PriceLowerBound() << std::endl;
  auto opt4 = Option<OptionType::PUT, BuySide::LONG, OptionClass::EUROPE, 180,
                     4000, 3700, 5>();
  std::cout << opt4.PriceLowerBound() << std::endl;

  // Spreads
  {
    // Bull Spread
    constexpr auto doesnt_matter_param = -1;
    auto opt1 = Option<OptionType::CALL, BuySide::LONG, OptionClass::EUROPE, 90,
                       3000, doesnt_matter_param, doesnt_matter_param>();
    auto opt2 = Option<OptionType::CALL, BuySide::SHORT, OptionClass::EUROPE,
                       90, 3500, doesnt_matter_param, doesnt_matter_param>();
    auto return_curve = opt1.GetReturnCurve() + opt2.GetReturnCurve();
    return_curve.Print();
  }

  {
    // Bear Spread
    constexpr auto doesnt_matter_param = -1;
    auto opt1 = Option<OptionType::PUT, BuySide::LONG, OptionClass::EUROPE, 90,
                       3500, doesnt_matter_param, doesnt_matter_param>();
    auto opt2 = Option<OptionType::PUT, BuySide::SHORT, OptionClass::EUROPE, 90,
                       3000, doesnt_matter_param, doesnt_matter_param>();
    auto return_curve = opt1.GetReturnCurve() + opt2.GetReturnCurve();
    return_curve.Print();
  }

  {
    // Butterfly Spread
    constexpr auto doesnt_matter_param = -1;
    auto opt1 = Option<OptionType::CALL, BuySide::LONG, OptionClass::EUROPE, 90,
                       5500, doesnt_matter_param, doesnt_matter_param>();
    auto opt2 = Option<OptionType::CALL, BuySide::LONG, OptionClass::EUROPE, 90,
                       6500, doesnt_matter_param, doesnt_matter_param>();
    auto opt3 = Option<OptionType::CALL, BuySide::SHORT, OptionClass::EUROPE,
                       90, 6000, doesnt_matter_param, doesnt_matter_param>();
    auto opt4 = Option<OptionType::CALL, BuySide::SHORT, OptionClass::EUROPE,
                       90, 6000, doesnt_matter_param, doesnt_matter_param>();
    auto return_curve = opt1.GetReturnCurve() + opt2.GetReturnCurve() +
                        opt3.GetReturnCurve() + opt4.GetReturnCurve();
    return_curve.Print();
  }

  return 0;
}
