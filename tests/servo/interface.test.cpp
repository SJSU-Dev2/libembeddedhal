#include <boost/ut.hpp>
#include <libhal/servo/interface.hpp>

#include "../ostreams.hpp"

namespace hal {
namespace {
constexpr percent expected_value = percent::from_ratio(1, 2);

class test_servo : public hal::servo
{
public:
  percent m_passed_position;

private:
  status driver_position(percent p_position) noexcept override
  {
    m_passed_position = p_position;
    return {};
  }
};
}  // namespace

boost::ut::suite servo_test = []() {
  using namespace boost::ut;
  // Setup
  test_servo test;

  // Exercise
  auto result = test.position(expected_value);

  // Verify
  expect(bool{ result });
  expect(that % expected_value == test.m_passed_position);
};
}  // namespace hal
