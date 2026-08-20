#pragma once
#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <iterator>
#include <optional>
#include <utility>

// #include "../sensors/sensor_base.hpp"
#include "esp_err.h"
#include "~/.espressif/v6.0.2/esp-idf/components/esp_driver_i2c/include/driver/i2c_master.h"
#include "~/.espressif/v6.0.2/esp-idf/components/esp_driver_i2c/include/driver/i2c_types.h"

namespace vmg_system::hal {
  enum class BusType : uint8_t {
    I2C,
    SPI,
    UART,
    GPIO,
    UNIQUE
  };

  enum class ProcessingType : uint8_t {
    PARALLEL   = 0,
    INDIVIDUAL = 1
  };

  static constexpr size_t kI2CSize{20};
  static constexpr size_t kSPISize{10};
  static constexpr size_t kGPIOSize{10};

  struct HALSettings {
   private:
    enum class SPI_PINS {
      MISO,
      MOSI,
      SCLK
    };
    static constexpr std::array<size_t, 3> kSPI2Pins{12, 13, 14};
    static constexpr std::array<size_t, 3> kSPI3Pins{19, 23, 18};

   public:
    void
    setDevDefault(size_t idx, BusType type)
    {
      switch (type) {
        case BusType::I2C:
          assert(idx < kI2CSize);
          _i2c_settings[idx] = {
              .dev_addr_length = I2C_ADDR_BIT_LEN_7,
              .device_address  = 0x00,
              .scl_speed_hz    = 100000,
          };
          break;
        case BusType::SPI:
          assert(idx < kSPISize);
          _spi_settings[idx] = {
              .dev_addr_length = I2C_ADDR_BIT_LEN_7,
              .device_address  = 0x00,
              .scl_speed_hz    = 100000,
          };
          break;
        case BusType::GPIO:
          gpio_reset_pin(_gpio_settings) assert(idx < kGPIOSize);
          break;
        case BusType::UART:
        case BusType::UNIQUE:
          assert(-1 == 1, "NOT IMPLEMENTED YET!");
      }
    }

    void
    setI2CDefault()
    {
      _i2c_master_cfg.port                         = I2C_NUM_0;
      _i2c_master_cfg.sda_io_num                   = 21;
      _i2c_master_cfg.scl_io_num                   = 22;
      _i2c_master_cfg.clk_source                   = I2C_CLK_SRC_DEFAULT;
      _i2c_master_cfg.glitch_ignore_cnt            = 7;
      //_i2c_master_cfg.intr_priority= 7;
      // _i2c_master_cfg.trans_queue_depth= 7;
      _i2c_master_cfg.flags.enable_internal_pullup = 1;
      _i2c_master_cfg.flags.allow_pd               = 0;

      _i2c_size                                    = 0;
    }
    void
    setSPIDefault()
    {
      setSPINum(SPI_HOST2);

      _spi_bus_config.data_io_default_level = true;
      _spi_bus_config.max_transfer_sz       = 64;
      _spi_bus_config.dma_burst_size        = 0;
      _spi_bus_config.flags =
          SPICOMMON_BUSFLAG_MASTER | SPICOMMON_BUSFLAG_IOMUX_PINS;
      //_spi_bus_config.isr_cpu_id;
      //_spi_bus_config.intr_flags;
      _spi_size = 0;
    }
    void
    setSPINum(spi_host_device_t spi_num)
    {
      assert(spi_num != SPI1_HOST && spi_num != SPI_HOST_MAX, "WRONG SPI NUM");
      _spi_num = spi_num;

      std::fill(std::vector<int>::iterator(&_spi_bus_config.iocfg[0]),
                std::vector<int>::iterator(&_spi_bus_config.iocfg[9]), -1);
      switch (spi_num) {
        case SPI2_HOST:
          _spi_bus_config.mosi_io_num =
              kSPI2Pins[static_cast<size_t>(SPI_PINS::MOSI)];
          _spi_bus_config.miso_io_num =
              kSPI2Pins[static_cast<size_t>(SPI_PINS::MISO)];
          _spi_bus_config.sclk_io_num =
              kSPI2Pins[static_cast<size_t>(SPI_PINS::SCLK)];
          break;
        case SPI3_HOST:
          _spi_bus_config.mosi_io_num =
              kSPI3Pins[static_cast<size_t>(SPI_PINS::MOSI)];
          _spi_bus_config.miso_io_num =
              kSPI3Pins[static_cast<size_t>(SPI_PINS::MISO)];
          _spi_bus_config.sclk_io_num =
              kSPI3Pins[static_cast<size_t>(SPI_PINS::SCLK)];
          break;
        default:
          assert(-1 == 1);
      }
    }

    void
    addSensor(size_t id, ProcessingType type, BusType BusType)
    {
      switch (BusType) {
        case BusType::I2C:
          assert(_i2c_size != kI2CSize);
          _i2c_sensors_id[_i2c_size++] = std::make_pair{id, type};
          break;
        case BusType::SPI:
          assert(_spi_size != kSPISize);
          _spi_sensors_id[_spi_size++] = std::make_pair{id, type};
          break;
        case BusType::GPIO:
          assert(_gpio_size != kGPIOSize);
          _gpio_sensors_id[_gpio_size++] = std::make_pair{id, type};
          break;
        case BusType::UART:
        case BusType::UNIQUE:
          assert(-1 == 1, "NOT IMPLEMENTED YET!");
      }
    }
    void
    removeSensor(size_t id)
    {
      if (auto it =
              std::find_if(_gpio_sensors_id.begin(), _gpio_sensors_id.end(),
                           [id](auto& pair) { return pair.first == id; });
          it != _gpio_sensors_id.end()) {
        std::swap(it, std::next(_gpio_sensors_id.begin(), --_gpio_size));
        return;
      }
      if (auto it = std::find_if(_i2c_sensors_id.begin(), _i2c_sensors_id.end(),
                                 [id](auto& pair) { return pair.first == id; });
          it != _i2c_sensors_id.end()) {
        std::swap(it, std::next(_i2c_sensors_id.begin(), --_i2c_size));
        return;
      }
      if (auto it = std::find_if(_spi_sensors_id.begin(), _spi_sensors_id.end(),
                                 [id](auto& pair) { return pair.first == id; });
          it != _spi_sensors_id.end()) {
        std::swap(it, std::next(_spi_sensors_id.begin(), --_spi_size));
        return;
      }

      assert(-1 == 1);
    }

    std::array<spi_device_interface_config_t, kSPISize> _spi_settings{};
    std::array<i2c_device_config_t, kI2CSize> _i2c_settings{};
    std::array<size_t, kGPIOSize> _gpio_pins;

    std::array<std::pair<size_t, ProcessingType>, kI2CSize> _i2c_sensors_id{};
    std::array<std::pair<size_t, ProcessingType>, kSPISize> _spi_sensors_id{};
    std::array<std::pair<size_t, ProcessingType>, kGPIOSize> _gpio_sensors_id{};

    spi_bus_config_t _spi_bus_config{};
    i2c_master_bus_config_t _i2c_master_cfg{};
    gpio_config_t _gpio_settings{};

    size_t _i2c_size{};
    size_t _spi_size{};
    size_t _gpio_size{};

    spi_host_device_t _spi_num{};
  };

  class HAL {
   public:
    HAL(const HAL&)            = default;
    HAL(HAL&&)                 = delete;
    HAL& operator=(const HAL&) = default;
    HAL& operator=(HAL&&)      = delete;
    static std::optional<HAL>
    getHal(HALSettings& settings)
    {
      auto hal = HAL(settings);

      if (hal._is_healthy) {
        return hal;
      }

      return {};
    }
    ~HAL() {}

   private:
    HAL(HALSettings& settings) :
        _i2c_size(settings._i2c_size), _spi_size(settings._spi_size)
    {
      is_healthy = false;
    }

    i2c_master_bus_handle_t _i2c_bus{};
    std::array<i2c_master_dev_handle_t, kI2CSize> _i2c_handles{};
    std::array<spi_device_handle_t, kSPISize> _spi_handles{};
    std::array<spi_transaction_t, kSPISize> _spi_transactions{};

    spi_host_device_t _spi_num{};
    size_t _i2c_size{};
    size_t _spi_size{};
    size_t _gpio_size{};
    bool _is_healthy{true};
  };
}  // namespace vmg_system::hal
