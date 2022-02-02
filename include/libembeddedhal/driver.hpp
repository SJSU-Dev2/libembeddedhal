#pragma once

#include "enum.hpp"
#include "error.hpp"

#include <type_traits>

namespace embed {
/**
 * @brief Used for defining static_asserts that should always fail, but only if
 * the static_assert line is hit via `if constexpr` control block. Prefer to NOT
 * use this directly but to use `invalid_option` instead
 *
 * @tparam options ignored by the application but needed to create a non-trivial
 * specialization of this class which allows its usage in static_assert.
 */
template<auto... options>
struct invalid_option_t : std::false_type
{};

/**
 * @brief Helper definition to simplify the usage of invalid_option_t.
 *
 * @tparam options ignored by the application but needed to create a non-trivial
 * specialization of this class which allows its usage in static_assert.
 */
template<auto... options>
inline constexpr bool invalid_option = invalid_option_t<options...>::value;

/**
 * @brief An empty settings structure used to indicate that a driver or
 * interface does not have generic settings.
 *
 */
struct no_settings
{};

/**
 * @brief The basis class for all peripheral, device and system drivers in
 * libembeddedhal
 *
 * @tparam - settings_t generic settings for the driver. For example, generic
 * settings for a uart driver would have baud rate, stop bits and parity. This
 * is expected of all UART devices and as such is part of the systems API.
 */
template<class settings_t = no_settings>
class driver
{
public:
  /// default constructor
  driver() = default;
  /// Explicitly delete copy constructor to prevent slicing
  driver(const driver& p_other) = delete;
  /// Explicitly delete assignment operator to prevent slicing
  driver& operator=(const driver& p_other) = delete;
  /// Explicitly delete move constructor
  driver(const driver&& p_other) = delete;
  /// Destroy the object
  virtual ~driver() = default;

  /**
   * @brief Initialize the driver, apply the setting as defined in the
   * settings_t structure and enable it. Calling this function after it has
   * already been initialized will return false. In order to run initialization
   * again, reset() must be called. After initialization, the settings are
   * committed and saved into another settings structure. This settings can be
   * looked up and inspected by the application.
   *
   * @return boost::leaf::result<void> - returns an error if initialization of
   * the driver failed.
   */
  boost::leaf::result<void> initialize()
  {
    auto on_error = embed::error::setup();

    EMBED_CHECK(driver_initialize());

    m_initialized_settings = m_settings;
    m_initialized = true;

    return {};
  }
  /**
   * @brief Reset the driver in order to run initialize again. This is helpful
   * if the application needs to change the settings of a driver after it is
   * first initialized, like baud rate for serial or pull resistor for a pin.
   *
   */
  void set_as_uninitialized() noexcept { m_initialized = false; }
  /**
   * @brief Determine if the driver has been initialized
   *
   * @return true the driver is initialized
   * @return false the driver has not been initialized or has been reset
   */
  [[nodiscard]] bool is_initialized() const noexcept { return m_initialized; }
  /**
   * @brief Get access to uncommitted driver settings.
   *
   * @return settings_t& reference to the uncommitted driver settings. When
   * initialize runs successful, the uncommitted settings will be saved to the
   * initialize_settings().
   */
  [[nodiscard]] settings_t& settings() noexcept { return m_settings; }
  /**
   * @brief Get access to the settings that were used in the latest
   * initialization. These settings only get updated when a successful
   * initialize() has occurred.
   *
   * @return const settings_t& the current settings of the driver if it is
   * initialized. If the driver is not initialized, then the contents of this
   * structure should be ignored as they may not represent the current of the
   * driver.
   */
  [[nodiscard]] const settings_t& initialized_settings() const noexcept
  {
    return m_initialized_settings;
  }

protected:
  /**
   * @brief Implementation of driver initialize by the inherited driver
   *
   * @return true driver initialized successfully
   * @return false driver initialization failed
   */
  virtual boost::leaf::result<void> driver_initialize() = 0;

private:
  /// Mutable settings
  settings_t m_settings{};
  /// Saved version of the settings at initialization
  settings_t m_initialized_settings{};
  /// Determines if the driver has been initialized
  bool m_initialized = false;
};
}  // namespace embed
