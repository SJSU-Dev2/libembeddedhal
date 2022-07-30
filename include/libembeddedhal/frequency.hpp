#pragma once

#include <algorithm>
#include <chrono>
#include <cinttypes>
#include <cstdint>
#include <limits>
#include <ratio>
#include <span>

#include "math.hpp"
#include "percent.hpp"
#include "units.hpp"

namespace hal {
/**
 * @addtogroup utility
 * @{
 */
/**
 * @brief Structure containing cycle count for the high and low side of a signal
 * duty cycle.
 *
 */
struct duty_cycle
{
  /// Number of cycles the signal will stay in the HIGH voltage state
  std::uint32_t high = 0;
  /// Number of cycles the signal will stay in the LOW voltage state
  std::uint32_t low = 0;

  /**
   * @brief == operator
   *
   * @param p_cycle - other duty scale to compare to
   * @return constexpr auto - true if the duty cycles have the exact same
   * values.
   */
  [[nodiscard]] constexpr auto operator==(
    const duty_cycle& p_cycle) const noexcept
  {
    return (high == p_cycle.high) && (low == p_cycle.low);
  }

  /**
   * @brief Conversion from duty cycle to percentage
   *
   * @return constexpr percentage - Percentage of high cycles to the total cycle
   * count.
   */
  [[nodiscard]] explicit constexpr operator percent() const noexcept
  {
    std::uint32_t total_cycles = 0;
    std::uint32_t scaled_high = 0;
    // Convert high & low to int64_t to prevent overflow
    std::uint64_t overflow_total = std::uint64_t{ high } + std::uint64_t{ low };

    // If overflow_total is greater than what can fit within an uint32_t, then
    // we can shift the values over by 1 to get them to fit. This effectively
    // divides the two values in half, but the ratio will stay the same.
    if (overflow_total > std::numeric_limits<decltype(high)>::max()) {
      total_cycles = static_cast<std::uint32_t>(overflow_total >> 1);
      scaled_high = high >> 1;
    } else {
      total_cycles = static_cast<std::uint32_t>(overflow_total);
      scaled_high = high;
    }

    return percent::from_ratio(scaled_high, total_cycles);
  }
};

/**
 * @brief Represents the frequency of a signal. It consists of a single integer
 * 64-bit number that presents the integer representation of a signal frequency.
 *
 */
struct frequency
{
  std::uint32_t value_hz = 100'000;

  /**
   * @brief Default operators for <, <=, >, >= and ==
   *
   * @return auto - result of the comparison
   */
  [[nodiscard]] constexpr auto operator<=>(const frequency&) const noexcept =
    default;
};

/**
 * @brief Scale down a frequency by an integer factor
 *
 * @param p_lhs - the frequency to be scaled
 * @param p_rhs - the integer value to scale the frequency by
 * @return constexpr frequency
 */
[[nodiscard]] constexpr frequency operator/(const frequency& p_lhs,
                                            std::uint32_t p_rhs) noexcept
{
  return frequency{ rounding_division(p_lhs.value_hz, p_rhs) };
}

/**
 * @brief Calculate the divider required to reach the target frequency from the
 * source frequency.
 *
 * @param p_source - the input frequency to be divided down to the target
 * frequency with an integer divide
 * @param p_target - the target frequency to reach via an integer divide
 * @return constexpr std::uint32_t - frequency divide value representing
 * the number of cycles in the input that constitute one cycle in the target
 * frequency.
 */
[[nodiscard]] constexpr std::uint32_t operator/(
  const frequency& p_source,
  const frequency& p_target) noexcept
{
  return rounding_division(p_source.value_hz, p_target.value_hz);
}

/**
 * @brief Scale up a frequency by an integer factor
 *
 * @tparam Integer - type of unsigned integer
 * @param p_source - frequency to scale
 * @param p_scalar - the value to scale the frequency up to
 * @return constexpr frequency
 */
template<std::unsigned_integral Integer>
[[nodiscard]] inline boost::leaf::result<frequency> operator*(
  const frequency& p_source,
  Integer p_scalar) noexcept
{
  return frequency{ BOOST_LEAF_CHECK(multiply(p_source.value_hz, p_scalar)) };
}

/**
 * @brief Generate a duty_cycle object based on the percent value and
 * the input count value. The count value is split based on the ratio within
 * percent
 *
 * @param p_cycles - the number of cycles to be split into a duty cycle
 * @param p_percent - the target duty cycle percentage
 * @return constexpr duty_cycle - the duty cycle cycle counts
 */
[[nodiscard]] constexpr duty_cycle calculate_duty_cycle(
  std::uint32_t p_cycles,
  percent p_percent) noexcept
{
  // Scale down value based on the integer percentage value in percent
  std::uint32_t high = p_cycles * p_percent;
  // p_cycles will always be larger than or equal to high
  std::uint32_t low = p_cycles - high;

  return duty_cycle{
    .high = high,
    .low = low,
  };
}

/**
 * @brief Calculate the number of cycles of this frequency within the time
 * duration. This function is meant for timers to determine how many count
 * cycles are needed to reach a particular time duration at this frequency.
 *
 * @param p_source - source frequency
 * @param p_duration - the amount of time to convert to cycles
 * @return std::int64_t - number of cycles
 */
[[nodiscard]] constexpr std::int64_t cycles_per(
  const frequency& p_source,
  hal::time_duration p_duration) noexcept
{
  // Full Equation:
  // =========================================================================
  //
  //                              / ratio_num \_
  //   frequency_hz * |period| * | ----------- |  = cycles
  //                              \ ratio_den /
  //
  // std::chrono::nanoseconds::period::num == 1
  // std::chrono::nanoseconds::period::den == 1,000,000

  constexpr std::int64_t numerator = decltype(p_duration)::period::num;
  constexpr std::int64_t denominator = decltype(p_duration)::period::den;
  // Storing 32-bit value in a std::uint64_t for later computation, no
  // truncation possible.
  const std::int64_t duration = absolute_value(p_duration.count());
  // Duration contains at most a 64-bit number, value() is a
  // 32-bit number, and numerator is always the value 1.
  //
  // To contain the maximum resultant possible requires storage within an
  // integer of size 64-bits, and thus will fit within a std::uint64_t
  // variable, no overflow checks required.
  const std::int64_t count = duration * p_source.value_hz * numerator;
  const std::int64_t cycle_count = rounding_division(count, denominator);

  return cycle_count;
}

/**
 * @brief Calculates and returns the wavelength in seconds.
 *
 * @tparam Period - desired period (defaults to std::femto for femtoseconds).
 * @param p_source - source frequency to convert to wavelength
 * @return std::chrono::duration<int64_t, Period> - time based wavelength of
 * the frequency.
 */
template<typename Period = std::femto>
constexpr std::chrono::duration<int64_t, Period> wavelength(
  const frequency& p_source)
{
  // Full Equation (based on the equation in cycles_per()):
  // =========================================================================
  //
  //                /  cycle_count * ratio_den \_
  //   |period| =  | ---------------------------|
  //                \ ratio_num * frequency_hz /
  //
  // let cycle_count = 1
  // --> Wavelength is the length of a single cycle
  //
  // let ratio_num = 1
  // --> Smallest frequency can only be as small as 1Hz. No wavelengths with
  //     numerators

  static_assert(Period::num == 1,
                "Period::num of 1 are allowed for this function.");
  static_assert(Period::den <= 1000000000000000000,
                "Period::den cannot exceed 1000000000000000000.");

  std::uint64_t numerator = Period::den;
  std::uint64_t denominator = p_source.value_hz;
  std::uint64_t duration = rounding_division(numerator, denominator);
  return std::chrono::duration<int64_t, Period>(duration);
}

/**
 * @brief Calculate the amount of time it takes a frequency to oscillate a
 * number of cycles.
 *
 * @param p_source - the frequency to compute the cycles from
 * @param p_cycles - number of cycles within the time duration
 * @return std::chrono::nanoseconds - time duration based on this frequency
 * and the number of cycles
 */
[[nodiscard]] constexpr std::chrono::nanoseconds duration_from_cycles(
  const frequency& p_source,
  std::int32_t p_cycles) noexcept
{
  // Full Equation (based on the equation in cycles_per()):
  // =========================================================================
  //
  //                /    cycles * ratio_den    \_
  //   |period| =  | ---------------------------|
  //                \ frequency_hz * ratio_num /
  //
  constexpr auto time_duration_den = std::chrono::nanoseconds::period::den;
  constexpr auto time_duration_num = std::chrono::nanoseconds::period::num;
  constexpr auto scale_den = std::int64_t{ time_duration_den };
  constexpr auto scale_num = std::int64_t{ time_duration_num };

  std::int64_t numerator = std::int64_t{ p_cycles } * scale_den;
  std::int64_t denominator = p_source.value_hz * scale_num;
  std::int64_t nanoseconds = rounding_division(numerator, denominator);

  return std::chrono::nanoseconds(nanoseconds);
}

/**
 * @brief Calculate a duty cycle based on a source frequency, target wavelength
 * and a percentage.
 *
 * @param p_source_clock - clock frequency controlling the duty cycle
 * @param p_duration - target wavelength to reach
 * @param p_percent - ratio of the duty cycle high time
 * @return constexpr duty_cycle
 */
[[nodiscard]] inline boost::leaf::result<duty_cycle> calculate_duty_cycle(
  const frequency& p_source_clock,
  hal::time_duration p_duration,
  percent p_percent) noexcept
{
  std::uint64_t cycles = cycles_per(p_source_clock, p_duration);
  if (cycles > std::numeric_limits<std::uint32_t>::max()) {
    return boost::leaf::new_error(std::errc::value_too_large);
  }
  return calculate_duty_cycle(static_cast<uint32_t>(cycles), p_percent);
}

/**
 * @brief Divider selection mode for achieving a target frequency.
 */
enum class divider_rule
{
  /// Restrict dividers to achieve frequencies above target frequency.
  higher,
  /// Restrict dividers to achieve frequencies below target frequency.
  lower,
  /// Do not restrict dividers.
  closest
};

/**
 * @brief Calculate divider for the closest resulting frequency to target.
 *
 * @tparam InputIt - iterator type from the container.
 * @param p_source - the source frequency .
 * @param p_first - iterator to the first item in the search space of
 * available frequency dividers.
 * @param p_last - iterator after the last item in search space of available
 * frequency dividers.
 * @param p_target - the ideal frequency to be achieved by selecting one of
 * the available frequency dividers.
 * @param p_divider_rule - the selection mode for dividers. This can
 * restrict the dividers to result in a frequency less than or equal to the
 * target, higher than or equal to the target or not restrict the dividers at
 * all and get the closest value to the target.
 * @return InputIt - an iterator pointing to the resulting best divider if a
 * solution is found. If no solution is found, p_last is returned.
 */
template<typename InputIt>
[[nodiscard]] constexpr InputIt closest(const frequency& p_source,
                                        InputIt p_first,
                                        InputIt p_last,
                                        frequency p_target,
                                        divider_rule p_divider_rule)
{
  auto cost = [p_target](frequency& p_candidate) -> std::uint32_t {
    return distance(p_candidate.value_hz, p_target.value_hz);
  };

  auto is_applicable = [p_target,
                        p_divider_rule](frequency& p_candidate) -> bool {
    switch (p_divider_rule) {
      case divider_rule::lower:
        return p_candidate <= p_target;
      case divider_rule::higher:
        return p_candidate >= p_target;
      case divider_rule::closest:
        return true;
      default:
        return false;
    }
  };

  InputIt best = p_last;
  auto best_cost = std::numeric_limits<uint32_t>::max();

  for (InputIt it = p_first; it != p_last; it++) {
    frequency candidate{ p_source / *it };
    if (is_applicable(candidate) && cost(candidate) < best_cost) {
      best = it;
      best_cost = cost(candidate);
    }
  }
  return best;
}

namespace literals {
/**
 * @brief user defined literals for making frequencies: 1337_Hz
 *
 * Example range: 1337_Hz == 1,337 Hz
 *
 * @param p_value - frequency in hertz
 * @return consteval frequency - frequency in hertz
 */
[[nodiscard]] consteval frequency operator""_Hz(
  unsigned long long p_value) noexcept
{
  return frequency{ static_cast<std::uint32_t>(p_value) };
}

/**
 * @brief user defined literals for making frequencies in the kilohertz.
 *
 * Example range: 20_kHz == 20,000 Hz
 *
 * @param p_value - frequency in kilohertz
 * @return consteval frequency - frequency in kilohertz
 */
[[nodiscard]] consteval frequency operator""_kHz(
  unsigned long long p_value) noexcept
{
  const auto value = p_value * std::kilo::num;
  return frequency{ static_cast<std::uint32_t>(value) };
}

/**
 * @brief user defined literals for making frequencies in the megahertz.
 *
 * Example range: 42_MHz == 42,000,000 Hz
 *
 * @param p_value - frequency in megahertz
 * @return consteval frequency - frequency in megahertz
 */
[[nodiscard]] consteval frequency operator""_MHz(
  unsigned long long p_value) noexcept
{
  const auto value = p_value * std::mega::num;
  return frequency{ static_cast<std::uint32_t>(value) };
}
}  // namespace literals
/** @} */
}  // namespace hal
