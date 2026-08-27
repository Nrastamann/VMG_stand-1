#pragma once
#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstring>
#include <ranges>
#include <utility>
#include <valarray>

#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "driver/spi_master.h"
#include "esp_err.h"

namespace vmg_system::hal {
  enum class GPIOOperation : uint8_t {
    GET_LEVEL,
    SET_LEVEL,
    SET_INTR,
    REMOVE_INTR
  };

  enum class I2COperation : uint8_t {
    READ,
    WRITE,
    WRITE_AND_READ
  };

  struct GPIOTransaction {
    union _data {
      struct {
        gpio_isr_t _ptr;
        void* _arg;
      } _isr;
      bool _level;
    };
    GPIOOperation _operation_type;
  };

  struct I2CTransaction {
    uint8_t* _rx_buf;
    uint8_t* _tx_buf;

    uint16_t _rx_size;
    uint16_t _tx_size;

    I2COperation _operation_type;
  };

  struct transaction_result {
    uint32_t value;
    bool _is_healthy;
  };

  using transaction_data =
      std::variant<spi_transaction_t, I2CTransaction, GPIOTransaction>;

  enum class BusType : uint8_t {
    I2C        = 0,
    SPI        = 1,
    GPIO       = 2,
    BORDERLINE = 3,
    UART,
    UNIQUE
  };
  static constexpr size_t kI2CIdx{static_cast<size_t>(BusType::I2C)};
  static constexpr size_t kGPIOIdx{static_cast<size_t>(BusType::GPIO)};
  static constexpr size_t kSPIIdx{static_cast<size_t>(BusType::SPI)};

  enum class ProcessingType : uint8_t {
    PARALLEL   = 0,
    INDIVIDUAL = 1
  };

  struct HALSettings {
   private:
    enum class SPI_PINS {
      MISO,
      MOSI,
      SCLK
    };
    using id_type = uint16_t;

    static constexpr std::array<size_t, 3> kSPI2Pins{12, 13, 14};
    static constexpr std::array<size_t, 3> kSPI3Pins{19, 23, 18};
    static constexpr id_type kBusSize{10};
    static constexpr id_type kNBusImplemented =
        static_cast<id_type>(BusType::BORDERLINE);

    template <typename T>
    using PerBusContainer = std::array<T, kBusSize>;

    template <typename T>
    using BusMetadataContainer = std::array<T, kNBusImplemented>;

   public:
    HALSettings()
    {
      setI2CDefault();
      setSPIDefault();
      setGPIODefault();
    }

    void
    setDevDefault(id_type idx, BusType bus)
    {
      auto bus_id = static_cast<id_type>(bus);
      assert(idx < _bus_sizes[bus_id]);
      switch (bus) {
        case BusType::I2C:
          std::memset(&_i2c_settings[idx], 0, sizeof(i2c_device_config_t));
          _i2c_settings[idx] = {
              .dev_addr_length = I2C_ADDR_BIT_LEN_7,
              .device_address  = 0x00,
              .scl_speed_hz    = 100000,
          };
          break;
        case BusType::SPI:
          std::memset(&_spi_settings[idx].first, 0,
                      sizeof(spi_device_interface_config_t));
          _spi_settings[idx].first = {.command_bits = 4,
                                      .address_bits = 4,
                                      .dummy_bits   = 4,
                                      .mode         = 0,
                                      .spics_io_num = 0,
                                      .queue_size   = 0};

          break;
        case BusType::GPIO:
          std::memset(&_gpio_settings[idx], 0, sizeof(gpio_config_t));
          break;
        default:
          assert(-1 == 1, "NOT IMPLEMENTED YET!");
      }
    }

    void
    setI2CDefault()
    {
      _i2c_master_cfg.port                           = I2C_NUM_0;
      _i2c_master_cfg.sda_io_num                     = 21;
      _i2c_master_cfg.scl_io_num                     = 22;
      _i2c_master_cfg.clk_source                     = I2C_CLK_SRC_DEFAULT;
      _i2c_master_cfg.glitch_ignore_cnt              = 7;
      //_i2c_master_cfg.intr_priority= 7;
      // _i2c_master_cfg.trans_queue_depth= 7;
      _i2c_master_cfg.flags.enable_internal_pullup   = 1;
      _i2c_master_cfg.flags.allow_pd                 = 0;

      _bus_sizes[static_cast<id_type>(BusType::I2C)] = 0;
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
      _bus_sizes[kSPIIdx] = 0;
    }

    void
    setGPIODefault()
    {
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
    addSensor(id_type id, ProcessingType type, BusType busType)
    {
      auto bus_id = static_cast<id_type>(busType);
      assert(_bus_sizes[bus_id] != kBusSize);
      _dev_ids[bus_id][_bus_sizes[bus_id]++]{id, type};
    }

    id_type
    getIdx(id_type id, id_type bus_id)
    {
      size_t diff = std::distance(
          _dev_ids[bus_id].begin(),
          std::ranges::find_if(_dev_ids[bus_id],
                               [id](auto& pair) { return pair.first == id; }));
      assert(diff < kBusSize);
      return diff;
    }

    void
    removeSensor(id_type id, BusType bus)
    {
      assert(_bus_sizes[bus_id] != 0);
      auto bus_id = static_cast<size_t>(bus);
      auto idx    = getIdx(id, bus);

      std::swap(_dev_ids[bus_id][idx], _dev_ids[bus_id][--_bus_sizes[bus_id]]);
    }

    PerBusContainer<std::pair<spi_device_interface_config_t>> _spi_settings{};
    PerBusContainer<i2c_device_config_t> _i2c_settings{};
    PerBusContainer<gpio_config_t> _gpio_settings;

    BusMetadataContainer<PerBusContainer<std::pair<id_type, ProcessingType>>>
        _dev_ids;

    spi_bus_config_t _spi_bus_config{};
    i2c_master_bus_config_t _i2c_master_cfg{};

    BusMetadataContainer<id_type> _bus_sizes;

    spi_host_device_t _spi_num{};
  };  // namespace vmg_system::hal

  class HAL {
    template <typename T>
    using PerBusContainer = std::array<T, kBusSize>;

    template <typename T>
    using BusMetadataContainer = std::array<T, kNBusImplemented>;
    static constexpr bool kAssertFail{true};

   public:
    HAL(const HAL&)            = default;
    HAL(HAL&&)                 = delete;
    HAL& operator=(const HAL&) = default;
    HAL& operator=(HAL&&)      = delete;

    HAL(HALSettings& settings) :
        _bus_sizes(std::move(settings._bus_sizes)), _spi_num(settings._spi_num)
    {
      std::ranges::transform(settings._gpio_settings, _gpio_pins.begin(),
                             [](auto& setting) {
                               auto i = std::log2(setting.pin_bit_mask);
                               assert(std::powl(2, i) == setting.pin_bit_mask);
                               return i;
                             });

      for (auto const& [idx, vec] :
           settings._dev_ids | std::views::enumerate |
               std::views::filter([&_bus_sizes](auto const& pair) {
                 return _bus_sizes[std::get<0>(pair)] != 0;
               })) {
        std::ranges::transform(
            vec, _dev_ids[idx].begin(),
            [idx](auto const& element) { return element.first; });
      }

      if (_i2c_size != 0) {
        ESP_ERROR_CHECK(
            i2c_new_master_bus(&settings._i2c_master_cfg, &_i2c_bus));

        auto* it = _i2c_handles.begin();
        size_t i = 0;
        for (auto& device_cfg :
             settings._i2c_settings | std::views::take(_bus_sizes[kI2CIdx])) {
          ESP_ERROR_CHECK(i2c_master_bus_add_device(_i2c_bus, &device_cfg,
                          *(it++));
        }
      }
      if (_spi_size != 0) {
        ESP_ERROR_CHECK(spi_bus_initialize(_spi_num, &settings._spi_bus_config,
                                           SPI_DMA_DISABLED));
        auto* it = _spi_handles.begin();
        for (auto& [device_cfg, _] :
             settings._spi_settings | std::views::take(_bus_sizes[kSPIIdx])) {
          ESP_ERROR_CHECK(spi_bus_add_device(_spi_num), device_cfg,
                          (it++).base());
        }
      }
      if (_gpio_size != 0) {
        for (auto& device_cfg :
             settings._gpio_settings | std::views::take(_bus_sizes[kGPIOIdx])) {
          ESP_ERROR_CHECK(gpio_config(&device_cfg));
        }
      }
    }

    ~HAL()
    {
      if (_bus_sizes[kSPIIdx] > 0) {
        for (auto& handle :
             _spi_handles | std::views::take(_bus_sizes[kSPIIdx])) {
          ESP_ERROR_CHECK(spi_bus_remove_device(handle));
        }
        ESP_ERROR_CHECK(spi_bus_free(_spi_num));
      }

      if (_bus_sizes[kI2CIdx] > 0) {
        for (auto& handle :
             _i2c_handles | std::views::take(_bus_sizes[kI2CIdx])) {
          ESP_ERROR_CHECK(i2c_master_bus_rm_device(handle));
        }
        ESP_ERROR_CHECK(i2c_del_master_bus(_i2c_bus));
      }
      if (_bus_sizes[kGPIOIdx] > 0) {
        for (auto pin : _gpio_pins | take(_bus_sizes[kGPIOIdx])) {
          ESP_ERROR_CHECK(gpio_reset_pin(pin));
        }
      }
    }

    transaction_result
    spi_transaction(id_type id, spi_transaction_t const& transaction)
    {
      esp_err_t res =
          spi_device_polling_transmit(_spi_handles[id], transaction);

      return {res, res == ESP_OK};
    }

    transaction_result
    i2c_transaction(id_type id, I2COperation const& transaction)
    {
      esp_err_t res{};
      switch (transaction._operation_type) {
        case READ:
          res = i2c_master_receive(_i2c_handles[id], transaction._rx_buf,
                                   transaction._rx_size,
                                   I2CTransaction::kTimeout);
          return {res, res == ESP_OK};

        case WRITE:
          res = i2c_master_transmit(_i2c_handles[id], transaction._tx_buf,
                                    transaction._tx_size,
                                    I2CTransaction::kTimeout);
          return {res, res == ESP_OK};

        case WRITE_AND_READ:
          res = i2c_master_transmit_receive(
              _i2c_handles[id], transaction._tx_buffer, transaction._tx_size,
              transaction._rx_buf, transaction._rx_size,
              I2CTransaction::kTimeout);
          return {res, res == ESP_OK};

        default:
          assert(-1 == 1, "Not implemented yet!");
      }
    }

    transaction_result
    gpio_transaction(id_type id, GPIOOperation const& transaction)
    {
      esp_err_t res{};
      switch (transaction) {
        case GPIOOperation::GET_LEVEL:
          return {gpio_get_level(_gpio_pins[id]), true};
        case GPIOOperation::SET_LEVEL:
          res = 
              gpio_set_level(_gpio_pins[id], transaction._data._level));
          return {res, res == ESP_OK};
        case GPIOOperation::SET_INTR:
          res = gpio_isr_handler_add(_gpio_pins[id],
                                               transaction._data._isr._ptr,
                                               transaction._data._isr._arg));
          return {res, res == ESP_OK};
        case GPIOOperation::REMOVE_INTR:
          res = gpio_isr_handler_remove(_gpio_pins[id]));
          return {res, res == ESP_OK};
        default:
          assert(-1 == 1, "Not implemented yet!");
      }
    }

    template <bool assertFail = false>
    size_t
    find_by_id(id_type id, size_t bus_id)
    {
      auto result = std::distance(_dev_ids[bus_id].begin(),
                                  std::ranges::find(_dev_ids[bus_id], id));

      if constexpr (assertFail) {
        assert(result != _dev_ids[bus_id].size());
        return result;
      }
      else {
        return result;
      }
    }

    std::pair<size_t, size_t>
    find_by_id(id_type id)
    {
      for (auto [i, _] : _bus_sizes | std::views::enumerate |
                             std::views::filter([](auto const& bus_size) {
                               return std::get<1>(bus_size) != 0;
                             })) {
        if (auto res = find_by_id(id, i) != _bus_sizes[i].size()) {
          return std::make_pair(i, res);
        }
      }

      assert(-1 == 1);
      return std::make_pair(-1, -1);
    }

    transaction_result
    transaction(id_type dev_id, transaction_data const& data,
                BusType bus = BusType::BORDERLINE)
    {
      if (bus != BusType::BORDERLINE) {
        switch (bus) {
          case BusType::I2C:
            return i2c_transaction(find_by_id<kAssertFail>(dev_id, bus)
                                       transaction_info);
          case BusType::GPIO:
            return gpio_transaction(find_by_id<kAssertFail>(dev_id, bus),
                                    transaction_info);
          case BusType::SPI:
            return spi_transaction(
                find_by_id<kAssertFail>(dev_id, bus),
                std::get<spi_transaction_t>(transaction_info));
          default:
            assert(-1 == 1, "Not implemented yet!");
        }
      }

      auto [bus, idx] = find_by_id(dev_id);
      switch (static_cast<BusType>(bus)) {
        case BusType::I2C:
          return i2c_transaction(idx, std::get<>(transaction_info));
        case BusType::SPI:
          return spi_transaction(idx,
                                 std::get<spi_transaction_t>(transaction_info));
        case BusType::GPIO:
          return gpio_transaction(idx, std::get<>(transaction_info));
        default:
          assert(-1 == 1, "Not implemented yet!");
      }

      return -1;
    }

   private:
    i2c_master_bus_handle_t _i2c_bus{};

    PerBusContainer<i2c_master_dev_handle_t> _i2c_handles{};
    PerBusContainer<spi_device_handle_t> _spi_handles{};
    PerBusContainer<gpio_num_t> _gpio_pins{};

    BusMetadataContainer<PerBusContainer<id_type>> _dev_ids{};

    BusMetadataContainer<PerBusContainer<id_type>> _bus_sizes;

    spi_host_device_t _spi_num{};
  };
}  // namespace vmg_system::hal
