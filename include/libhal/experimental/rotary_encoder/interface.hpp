#pragma once

#include "../error.hpp"
#include "unit.hpp"

namespace hal {
/**
 * @brief Hardware abstraction interface for devices that report rotational
 * position of an actuator, knob, or some other system.
 *
 */
class rotary_encoder
{
public:
  /**
   * @brief Return the current angular position from the feedback device.
   *
   * The angular position can be relative, meaning that they initialize
   * to an angular position of 0 and does not have information of its current
   * orientation. The angular position can also be absolute, meaning that the
   * encoder knows its current orientation at start and can return value a
   * non-zero value at initialization. This interface does not distinguish
   * between these two encoder schemes.
   *
   * The fastest practical motor velocity for this API is 100,000 RPM and at
   * this velocity, it would take 10.19 days to overflow the underlying int64_t
   * of data type angular_position. If the use of this api requires more
   * stored rotations than allowed, then external support will need to be added
   * to allow for an additional counter of storage.
   *
   * @return result<microrotation> - current angular position
   * reading or an error.
   */
  [[nodiscard]] result<microrotation> read() noexcept
  {
    return driver_read();
  }

private:
  virtual result<microrotation> driver_read() noexcept = 0;
};
}  // namespace hal