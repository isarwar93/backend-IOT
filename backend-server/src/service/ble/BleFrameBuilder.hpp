#ifndef BLE_FRAME_BUILDER_HPP
#define BLE_FRAME_BUILDER_HPP

#include <cstdint>
#include <vector>
#include <cstring>
#include <cmath>

class BleFrameBuilder {
  // --- decode helpers ---
  static uint16_t rd_u16_le(const uint8_t* p) {
      return uint16_t(p[0]) | (uint16_t(p[1]) << 8);
  }
  static float rd_f32_le(const uint8_t* p) {
      uint32_t u = uint32_t(p[0]) | (uint32_t(p[1]) << 8) |
                  (uint32_t(p[2]) << 16) | (uint32_t(p[3]) << 24);
      float f; std::memcpy(&f, &u, 4); return f;
  }

  // decode IEEE-11073 16-bit SFLOAT
  static float decode_sfloat_11073(uint16_t raw) {
    int16_t mant = raw & 0x0FFF;
    if (mant & 0x0800) mant |= 0xF000; // sign extend
    int8_t exp = (raw >> 12) & 0x0F;
    if (exp & 0x08) exp |= 0xF0; // sign extend
    if (mant == 0x07FF) return NAN;
    if (mant == 0x07FE) return INFINITY;
    if (mant == 0x0802) return -INFINITY;
    return mant * std::pow(10.0f, exp);
  }

public:
  // 1) PoX (SpO2 + Pulse) from 11073 SFLOATs
std::vector<float> makePoX(const uint8_t* data, size_t len) {
    std::vector<float> out;
    if (len >= 6) {
      uint16_t spo2_raw = rd_u16_le(&data[2]);
      uint16_t pr_raw   = rd_u16_le(&data[4]);
      out.push_back(decode_sfloat_11073(spo2_raw));
      out.push_back(decode_sfloat_11073(pr_raw));
    }
    return out;
  }

  // 2) Heart Rate Measurement (flags + 8/16-bit BPM)
std::vector<float> makeHeart(const uint8_t* data, size_t len) {
    std::vector<float> out;
    if (len < 1) return out;
    uint8_t flags = data[0];
    if (flags & 0x01) { // 16-bit HR
      if (len >= 3) out.push_back(rd_u16_le(&data[1]));
    } else {            // 8-bit HR
      out.push_back(static_cast<uint8_t>(data[1]));
    }
    return out;
  }

  // 3) Custom characteristic: 5 floats (20 bytes, little-endian f32)
std::vector<float> makeCustom5(const uint8_t* data, size_t len) {
    std::vector<float> out;
      for (int i = 0; i < len/sizeof(float); ++i) {
        out.push_back(rd_f32_le(&data[i*sizeof(float)]));
      }
    return out;
  }
};

#endif