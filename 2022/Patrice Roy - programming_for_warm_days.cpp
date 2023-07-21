// Patrice Roy
// patricer@gmail.com
// Patrice.Roy@USherbrooke.ca

#include <iostream>
#include <string_view>
#include <concepts>
#include <type_traits>
using namespace std; // I know, I'm lazy, don't do this

template <floating_point T>
constexpr auto threshold = static_cast<T>(0.000001);

template <class T>
constexpr T absolute(T val) { return val < 0 ? -val : val; }

constexpr bool close_enough(integral auto a, integral auto b) {
   // return a == b;
   // note : someone (thanks!) has given me a case where
   //        operator== was wrong with mixed signedness,
   //        specifically -1 == numeric_limits<unsigned int>::max
   //        so cmp_equal is necessary (alternatively, we could
   //        have decided not to allow mixed signedness and we
   //        could have written a version for signed_integral
   //        and another for unsigned_integral)
   return cmp_equal(a, b); // from <utility>

}
constexpr bool close_enough(floating_point auto a, floating_point auto b) {
   using type = common_type_t<decltype(a), decltype(b)>;
   return absolute(a - b) <= threshold<type>; // a - b is evil...
}


class Celsius {};
class Fahrenheit {};
class Kelvin {};

template <class> struct temperature_traits;

template <> struct temperature_traits<Celsius> {
   static constexpr auto name = "C"sv;
   static constexpr double freezes_at = 0.0;
   static constexpr double to_neutral(double val) { return val; }
   static constexpr double from_neutral(double val) { return val; }
};

template <> struct temperature_traits<Fahrenheit> {
   static constexpr auto name = "F"sv;
   static constexpr double freezes_at = 32.0;
   static constexpr double to_neutral(double val) {
      return (val - freezes_at) * 5.0 / 9.0;
   }
   static constexpr double from_neutral(double val) {
      return val * 9.0 / 5.0 + freezes_at;
   }
};

template <> struct temperature_traits<Kelvin> {
   static constexpr auto name = "K"sv;
   static constexpr double freezes_at = 273.0; // kinda
   static constexpr double to_neutral(double val) {
      return val - freezes_at;
   }
   static constexpr double from_neutral(double val) {
      return val + freezes_at;
   }
};

template <class D, class S>
constexpr double temperature_cast(double src) {
   return temperature_traits<D>::from_neutral(
      temperature_traits<S>::to_neutral(src)
   );
}




template <class R>
class Temperature {
   double val = temperature_traits<R>::freezes_at;
public:
   Temperature() = default;
   constexpr double value() const { return val; }
   // note : explicit might be a good idea here, as
   //        has been suggested during my talk by a
   //        nice member of the audience, but if you
   //        do so, know that you will have to make
   //        the Temperature<R> type explicit in several
   //        places in this file
   constexpr /* explicit */ Temperature(double val) : val{val} {
   }
   template <class T>
   constexpr Temperature(const Temperature<T>& other)
      : val{ temperature_cast<R, T>(other.value()) } {
   }
   constexpr bool operator==(const Temperature& other) const {
      return close_enough(value(), other.value());
   }
   template <class T>
   constexpr bool operator==(const Temperature<T>& other) const {
      return *this == Temperature<R>{ other };
   }
   constexpr Temperature<R> operator-() const {
      return { -value() };
   }
};

constexpr Temperature<Celsius> operator "" _C(long double val) {
   return { static_cast<double>(val) };
}
constexpr Temperature<Celsius> operator "" _C(unsigned long long val) {
   return { static_cast<double>(val) };
}
constexpr Temperature<Fahrenheit> operator "" _F(long double val) {
   return { static_cast<double>(val) };
}
constexpr Temperature<Fahrenheit> operator "" _F(unsigned long long val) {
   return { static_cast<double>(val) };
}
constexpr Temperature<Kelvin> operator "" _K(long double val) {
   return { static_cast<double>(val) };
}
constexpr Temperature<Kelvin> operator "" _K(unsigned long long val) {
   return { static_cast<double>(val) };
}

template <class R>
ostream& operator<<(ostream& os, const Temperature<R>& temp) {
   return os << temp.value() << temperature_traits<R>::name;
}

int main() {
   static_assert(41_F == 5_C);
   auto fahr = -41.0_F;
   // Temperature<Fahrenheit> fahr = 41.0;
   Temperature<Celsius> cels = fahr;
   cout << fahr << " == " << cels << '\n'; // 41F == 5C
   //double fahr = 41.0;
   //double cels = FAHR_2_CELS(fahr);
   //cout << cels << "C\n"; // 5C
}