#include "exchanger.hpp"

namespace vmg_system::exchanger {
  int
  Exchanger::setPort(uint16_t port)
  {
    if (!std::holds_alternative<ExchangerTCP>(_exchanger)) {
      ESP_LOGE(kLogTag, "Couldn't set port, because of wrong exchanger type!");
      return -1;
    }
    std::get<ExchangerTCP>(_exchanger).setPort(port);
  }
  int
  Exchanger::setAddr(uint32_t addr)
  {
    if (!std::holds_alternative<ExchangerTCP>(_exchanger)) {
      ESP_LOGE(kLogTag, "Couldn't set addr, because of wrong exchanger type!");
      return -1;
    }
    std::get<ExchangerTCP>(_exchanger).setAddr(addr);
  }
  int
  Exchanger::setSocketType(int type)
  {
    if (!std::holds_alternative<ExchangerTCP>(_exchanger)) {
      ESP_LOGE(kLogTag, "Couldn't set type, because of wrong exchanger type!");
      return -1;
    }
    std::get<ExchangerTCP>(_exchanger).setSocketType(type);
  }

  bool
  Exchanger::setExchangerType(ExchangerType type)
  {
    switch (type) {
      case ExchangerType::TCP:
        _exchanger = ExchangerTCP{};
      default:
        ESP_LOGE("Couldn't set type, %d", static_cast<uint16_t>(type));
    }
  }
  int64_t
  Exchanger::send()
  {
    size_t msg_len = _writeBuffer.get_size();
    char* data     = _writeBuffer.get_data();

    int64_t res{};
    while (res != -1 && msg_len != 0) {
      std::advance(data, res);
      msg_len -= static_cast<size_t>(res);

      std::visit([&res, data, len = _writeBuffer.getSize()](
                     auto& exchanger) { res = exchanger.send(data, len); },
                 _exchanger);
    }
    _writeBuffer.clear();
    return res;
  }
  int64_t
  Exchanger::recv()
  {
    _readBuffer.clear();
    int64_t res{};
    std::visit(
        [&res, data = _readBuffer.get_data(), len = _readBuffer.get_max_size()](
            auto& exchanger) { res = exchanger.recv(data, len); },
        _exchanger);
    res > 0 ? _readBuffer.set_bytes_written(res) : void();
    return res;
  }
  void
  Exchanger::connectClient()
  {
    std::visit([](auto& exchanger) { exchanger.connectClient(); }, _exchanger);
  };
  void
  Exchanger::disconnectClient()
  {
    std::visit([](auto& exchanger) { exchanger.disconnectClient(); },
               _exchanger);
  }
  void
  Exchanger::startExchanger()
  {
    std::visit([](auto& exchanger) { exchanger.start(); }, _exchanger);
  }

  bool
  Exchanger::isMessageReady()
  {
    return _readBuffer.get_size() >= ClientMessage::max_serialized_size();
  }

  bool
  Exchanger::processClientMessage()
  {
    return _client_message.deserialize(_readBuffer) ==
           EmbeddedProto::Error::NO_ERRORS;
  }

  bool
  Exchanger::encodeStandMessage()
  {
    return _stand_message.serialize(_writeBuffer) ==
           EmbeddedProto::Error::NO_ERRORS;
  }

  ClientMessage&
  Exchanger::getClientMessage()
  {
    return _client_message;
  }
  ClientMessage&
  Exchanger::getStandMessage()
  {
    return _stand_message;
  }
}  // namespace vmg_system::exchanger
