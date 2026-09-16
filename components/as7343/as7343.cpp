#include "as7343.h"
#include "esphome/core/log.h"

namespace esphome {
namespace as7343 {

static const char *const TAG = "as7343";

// AS7343 Hardware Registers (verified on physical hardware)
static const uint8_t AS7343_REG_ENABLE   = 0x80; // PON, SP_EN
static const uint8_t AS7343_REG_ATIME    = 0x81; // Integration time steps
static const uint8_t AS7343_REG_STATUS2  = 0x90; // Data ready (bit 6: AVALID), saturation (bits 3,4)
static const uint8_t AS7343_REG_STATUS   = 0x93; // Interrupt status
static const uint8_t AS7343_REG_ASTATUS  = 0x94; // Shadow register latch trigger
static const uint8_t AS7343_REG_CH0_DATA = 0x95; // Burst read start (36 bytes for 18 channels)
static const uint8_t AS7343_REG_CFG0     = 0xBF; // Bank switch: 0x10 = Bank 1, 0x00 = Bank 0
static const uint8_t AS7343_REG_GAIN     = 0xC6; // Gain register in Bank 0
static const uint8_t AS7343_REG_ASTEP_L  = 0xD4; // Integration step length LSB
static const uint8_t AS7343_REG_ASTEP_H  = 0xD5; // Integration step length MSB
static const uint8_t AS7343_REG_CFG20    = 0xD6; // Auto-SMUX mode (0x60 = 18-channel mode)
static const uint8_t AS7343_REG_CHIP_ID  = 0x5A; // Chip Part ID in Bank 1 (expected 0x81)

bool AS7343Component::set_bank_(bool bank1) {
  uint8_t val = bank1 ? 0x10 : 0x00;
  return this->write_byte(AS7343_REG_CFG0, val);
}

void AS7343Component::setup() {
  ESP_LOGCONFIG(TAG, "Setting up ams-OSRAM AS7343 14-Channel Spectral Sensor...");

  // 1. Read Chip ID from Bank 1
  this->set_bank_(true);
  uint8_t chip_id = 0;
  if (!this->read_byte(AS7343_REG_CHIP_ID, &chip_id)) {
    ESP_LOGE(TAG, "Failed to communicate with AS7343 at I2C address 0x39!");
    this->mark_failed();
    return;
  }
  this->set_bank_(false);

  if ((chip_id & 0xFC) != 0x80 && chip_id != 0x81) {
    ESP_LOGW(TAG, "Unexpected AS7343 Chip ID: 0x%02X (expected 0x81)", chip_id);
  } else {
    ESP_LOGI(TAG, "AS7343 successfully detected! Part ID: 0x%02X", chip_id);
  }

  // 2. Power ON (PON = 1)
  this->write_byte(AS7343_REG_ENABLE, 0x01);
  delay(10);

  // 3. Configure Auto-SMUX 18-channel mode (CFG20 0xD6 = 0x60)
  this->write_byte(AS7343_REG_CFG20, 0x60);

  // 4. Set Hardware Gain (0xC6)
  this->write_byte(AS7343_REG_GAIN, (uint8_t)this->gain_);

  // 5. Integration Timing (ATIME, ASTEP)
  this->write_byte(AS7343_REG_ATIME, this->atime_);
  this->write_byte(AS7343_REG_ASTEP_L, (uint8_t)(this->astep_ & 0xFF));
  this->write_byte(AS7343_REG_ASTEP_H, (uint8_t)((this->astep_ >> 8) & 0xFF));

  this->initialized_ = true;
  ESP_LOGI(TAG, "AS7343 initialized with Auto-SMUX 18-ch mode. Gain=%d, ATIME=%u, ASTEP=%u",
           (int)this->gain_, this->atime_, this->astep_);
}

void AS7343Component::update() {
  if (this->is_failed() || !this->initialized_) {
    return;
  }

  // Ensure Bank 0
  this->set_bank_(false);

  // Clear any pending interrupt status
  uint8_t st = 0;
  this->read_byte(AS7343_REG_STATUS, &st);
  this->write_byte(AS7343_REG_STATUS, st);
  uint8_t clr_astatus = 0;
  this->read_byte(AS7343_REG_ASTATUS, &clr_astatus);

  // Start 18-channel spectral measurement cycle (PON=1, SP_EN=1)
  this->write_byte(AS7343_REG_ENABLE, 0x03);

  // Wait for measurement to complete (ATIME=29, ASTEP=599 takes ~250ms for 3 Auto-SMUX cycles)
  // We poll STATUS2 (0x90) bit 6 (AVALID)
  bool ready = false;
  for (int retry = 0; retry < 35; retry++) {
    delay(10);
    uint8_t status2 = 0;
    if (this->read_byte(AS7343_REG_STATUS2, &status2) && (status2 & 0x40)) {
      ready = true;
      if (status2 & 0x18) {
        ESP_LOGW(TAG, "AS7343 sensor saturation detected (STATUS2=0x%02X) - consider lowering Gain", status2);
      }
      break;
    }
  }

  if (!ready) {
    ESP_LOGW(TAG, "AS7343 measurement timeout (AVALID not set)");
    return;
  }

  // Latch shadow registers by reading ASTATUS (0x94)
  uint8_t astatus = 0;
  this->read_byte(AS7343_REG_ASTATUS, &astatus);

  // Continuous burst read of all 36 bytes (18 channels * 2 bytes) from 0x95
  uint8_t raw[36];
  if (!this->read_bytes(AS7343_REG_CH0_DATA, raw, 36)) {
    ESP_LOGW(TAG, "Burst read of AS7343 spectral data failed");
    return;
  }

  // Stop measurement after reading (SP_EN=0, PON=1)
  this->write_byte(AS7343_REG_ENABLE, 0x01);

  // Convert Little-Endian 16-bit counts
  uint16_t ch[18];
  for (int i = 0; i < 18; i++) {
    ch[i] = (uint16_t)raw[i * 2] | ((uint16_t)raw[i * 2 + 1] << 8);
    // Hardware glitch filter: discard corrupted frames where any channel exceeds ADC limit
    if (ch[i] > 18000) {
      ESP_LOGW(TAG, "Rejected corrupted I2C frame (ch[%d]=%u > 18000)", i, ch[i]);
      return;
    }
  }

  // Exact 18-channel Auto-SMUX physical channel mapping verified on live hardware:
  float fz_val    = ch[0];   // 450nm Deep Blue (Chlorophyll B)
  float fy_val    = ch[1];   // 555nm Wide Green (Photopic peak)
  float fxl_val   = ch[2];   // 600nm Orange
  float nir_val   = ch[3];   // 855nm Near-Infrared
  float clear_val = ch[4];   // Clear / Visible VIS
  float f2_val    = ch[6];   // 425nm Indigo
  float f3_val    = ch[7];   // 475nm Cyan-Blue
  float f4_val    = ch[8];   // 515nm Cyan
  float f6_val    = ch[9];   // 640nm Red
  float f1_val    = ch[12];  // 405nm Violet
  float f7_val    = ch[13];  // 690nm Deep Red (Chlorophyll A)
  float f8_val    = ch[14];  // 745nm Far-Red (Phytochrome Pfr)
  float f5_val    = ch[15];  // 550nm Green

  // Publish raw counts to sensors
  if (this->f1_sensor_ != nullptr)    this->f1_sensor_->publish_state(f1_val);
  if (this->f2_sensor_ != nullptr)    this->f2_sensor_->publish_state(f2_val);
  if (this->fz_sensor_ != nullptr)    this->fz_sensor_->publish_state(fz_val);
  if (this->f3_sensor_ != nullptr)    this->f3_sensor_->publish_state(f3_val);
  if (this->f4_sensor_ != nullptr)    this->f4_sensor_->publish_state(f4_val);
  if (this->f5_sensor_ != nullptr)    this->f5_sensor_->publish_state(f5_val);
  if (this->fy_sensor_ != nullptr)    this->fy_sensor_->publish_state(fy_val);
  if (this->fxl_sensor_ != nullptr)   this->fxl_sensor_->publish_state(fxl_val);
  if (this->f6_sensor_ != nullptr)    this->f6_sensor_->publish_state(f6_val);
  if (this->f7_sensor_ != nullptr)    this->f7_sensor_->publish_state(f7_val);
  if (this->f8_sensor_ != nullptr)    this->f8_sensor_->publish_state(f8_val);
  if (this->nir_sensor_ != nullptr)   this->nir_sensor_->publish_state(nir_val);
  if (this->clear_sensor_ != nullptr) this->clear_sensor_->publish_state(clear_val);

  // Derived Photobiological Metrics
  // R:FR Ratio (F7 / F8) - Shade avoidance balance
  float r_fr = (f8_val > 5.0f) ? (f7_val / f8_val) : 0.0f;
  // B:R Ratio (FZ / F7) - Vegetative compactness balance
  float b_r = (f7_val > 5.0f) ? (fz_val / f7_val) : 0.0f;
  // PAR Proxy (Sum of 400nm - 700nm wavebands)
  float par_proxy = f1_val + f2_val + fz_val + f3_val + f4_val + f5_val + fy_val + fxl_val + f6_val + f7_val;
  float ppfd = par_proxy * this->ppfd_factor_;

  if (this->r_fr_sensor_ != nullptr)      this->r_fr_sensor_->publish_state(r_fr);
  if (this->b_r_sensor_ != nullptr)       this->b_r_sensor_->publish_state(b_r);
  if (this->par_proxy_sensor_ != nullptr) this->par_proxy_sensor_->publish_state(par_proxy);
  if (this->ppfd_sensor_ != nullptr)      this->ppfd_sensor_->publish_state(ppfd);
}

void AS7343Component::dump_config() {
  ESP_LOGCONFIG(TAG, "ams-OSRAM AS7343 Multi-Channel Spectral Sensor:");
  LOG_I2C_DEVICE(this);
  ESP_LOGCONFIG(TAG, "  Gain: %d", (int)this->gain_);
  ESP_LOGCONFIG(TAG, "  ATIME: %u", this->atime_);
  ESP_LOGCONFIG(TAG, "  ASTEP: %u", this->astep_);
  ESP_LOGCONFIG(TAG, "  PPFD Factor: %.4f", this->ppfd_factor_);
}

}  // namespace as7343
}  // namespace esphome
