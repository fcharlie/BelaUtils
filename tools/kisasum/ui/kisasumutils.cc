///
#include "kisasumutils.hpp"
#include <charconv>

namespace kisasum {

// RGB(1,2,3)
bool RgbColorExpand(std::string_view scr, std::uint32_t &cr) {
  if (!kisasum::ConsumePrefix(&scr, "RGB(") ||
      !kisasum::ConsumeSuffix(&scr, ")")) {
    return false;
  }
  uint32_t r = 0;
  uint32_t g = 0;
  uint32_t b = 0;
  auto end = scr.data() + scr.size();
  auto begin = scr.data();
  auto ec = std::from_chars(begin, end, r);
  if (ec.ec != std::errc{} || ec.ptr + 1 >= end) {
    return false;
  }
  begin = ec.ptr + 1;
  ec = std::from_chars(begin, end, g);
  if (ec.ec != std::errc{} || ec.ptr + 1 >= end) {
    return false;
  }
  begin = ec.ptr + 1;
  ec = std::from_chars(begin, end, b);
  if (ec.ec != std::errc{} || ec.ptr + 1 >= end) {
    return false;
  }
  return true;
}

bool InitializeColorValue(std::string_view scr, std::uint32_t &cr) {
  if (scr.size() < 4) {
    return false;
  }
  if (scr.front() == '#') {
    scr.remove_prefix(1);
    auto ec = std::from_chars(scr.data(), scr.data() + scr.size(), cr, 16);
    if (ec.ec != std::errc{}) {
      return false;
    }
    return true;
  }
  return RgbColorExpand(scr, cr);
}

} // namespace kisasum