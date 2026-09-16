#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/i2c/i2c.h"

namespace esphome {
namespace as7343 {

enum AS7343Gain {
  AS7343_GAIN_0_5X = 0,
  AS7343_GAIN_1X   = 1,
  AS7343_GAIN_2X   = 2,
  AS7343_GAIN_4X   = 3,
  AS7343_GAIN_8X   = 4,
  AS7343_GAIN_16X  = 5,
  AS7343_GAIN_32X  = 6,
  AS7343_GAIN_64X  = 7,
  AS7343_GAIN_128X = 8,
  AS7343_GAIN_256X = 9,
  AS7343_GAIN_512X = 10,
};

class AS7343Component : public PollingComponent, public i2c::I2CDevice {
 public:
  void setup() override;
  void update() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::DATA; }

  void set_gain(AS7343Gain gain) { gain_ = gain; }
  void set_atime(uint8_t atime) { atime_ = atime; }
  void set_astep(uint16_t astep) { astep_ = astep; }
  void set_ppfd_factor(float factor) { ppfd_factor_ = factor; }

  void set_f1_sensor(sensor::Sensor *s) { f1_sensor_ = s; }
  void set_f2_sensor(sensor::Sensor *s) { f2_sensor_ = s; }
  void set_fz_sensor(sensor::Sensor *s) { fz_sensor_ = s; }
  void set_f3_sensor(sensor::Sensor *s) { f3_sensor_ = s; }
  void set_f4_sensor(sensor::Sensor *s) { f4_sensor_ = s; }
  void set_f5_sensor(sensor::Sensor *s) { f5_sensor_ = s; }
  void set_fy_sensor(sensor::Sensor *s) { fy_sensor_ = s; }
  void set_fxl_sensor(sensor::Sensor *s) { fxl_sensor_ = s; }
  void set_f6_sensor(sensor::Sensor *s) { f6_sensor_ = s; }
  void set_f7_sensor(sensor::Sensor *s) { f7_sensor_ = s; }
  void set_f8_sensor(sensor::Sensor *s) { f8_sensor_ = s; }
  void set_nir_sensor(sensor::Sensor *s) { nir_sensor_ = s; }
  void set_clear_sensor(sensor::Sensor *s) { clear_sensor_ = s; }

  void set_r_fr_sensor(sensor::Sensor *s) { r_fr_sensor_ = s; }
  void set_b_r_sensor(sensor::Sensor *s) { b_r_sensor_ = s; }
  void set_par_proxy_sensor(sensor::Sensor *s) { par_proxy_sensor_ = s; }
  void set_ppfd_sensor(sensor::Sensor *s) { ppfd_sensor_ = s; }

 protected:
  bool set_bank_(bool bank1);
  bool read_channel_data_();

  AS7343Gain gain_{AS7343_GAIN_1X};
  uint8_t atime_{29};
  uint16_t astep_{599};
  float ppfd_factor_{0.150f};
  bool initialized_{false};

  sensor::Sensor *f1_sensor_{nullptr};
  sensor::Sensor *f2_sensor_{nullptr};
  sensor::Sensor *fz_sensor_{nullptr};
  sensor::Sensor *f3_sensor_{nullptr};
  sensor::Sensor *f4_sensor_{nullptr};
  sensor::Sensor *f5_sensor_{nullptr};
  sensor::Sensor *fy_sensor_{nullptr};
  sensor::Sensor *fxl_sensor_{nullptr};
  sensor::Sensor *f6_sensor_{nullptr};
  sensor::Sensor *f7_sensor_{nullptr};
  sensor::Sensor *f8_sensor_{nullptr};
  sensor::Sensor *nir_sensor_{nullptr};
  sensor::Sensor *clear_sensor_{nullptr};

  sensor::Sensor *r_fr_sensor_{nullptr};
  sensor::Sensor *b_r_sensor_{nullptr};
  sensor::Sensor *par_proxy_sensor_{nullptr};
  sensor::Sensor *ppfd_sensor_{nullptr};
};

}  // namespace as7343
}  // namespace esphome
