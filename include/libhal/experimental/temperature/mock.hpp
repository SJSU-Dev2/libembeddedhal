#pragma once

#include <queue>

#include "interface.hpp"

namespace hal::mock {
/**
 * @brief Mock temperature sensor implementation for use in unit tests and
 * simulations.
 *
 */
struct temperature_sensor : public hal::temperature_sensor
{
  /**
   * @brief Queues the temperature values to be returned for read()
   *
   * @param p_temperatures - queue of temperatures
   */
  void set(std::queue<microkelvin>& p_temperatures)
  {
    m_temperatures = p_temperatures;
  }

private:
  result<microkelvin> driver_read() noexcept
  {
    if (m_temperatures.size() == 0) {
      return hal::new_error(std::out_of_range("temperatures queue is empty!"));
    }
    auto m_current_value = m_temperatures.front();
    m_temperatures.pop();
    return m_current_value;
  }

  std::queue<microkelvin> m_temperatures{};
};
}  // namespace hal::mock
