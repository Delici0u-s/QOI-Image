#pragma once
#include <cstdint>
#include <string>
#include <string_view>

namespace QOID {
using ui = unsigned int;
using color = uint8_t;
using p_color = uint32_t;
using str = std::string;
using strv = std::string_view;

inline color min{0};
inline color max{255};

} // namespace QOID

// assumes little endian if not big endian
#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#else
#endif