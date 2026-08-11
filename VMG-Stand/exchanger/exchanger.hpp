#pragma once
#include <cstdint>
#include <variant>

#include "../client_message.h"
#include "../stand_message.h"
#include "tcp_exchanger.hpp"

namespace vmg_system::exchanger {
  enum class ExchangerType : uint8_t {
    TCP   = 0,
    UNSET = 1
  };

  class Exchanger {
    static constexpr char const* kLogTag = "Exchanger Container";
    static constexpr size_t kMessagePoolSize{5};
    static constexpr size_t kRepeatedMax{16};

    using StandMessage  = vmg_protocol::v1::StandMessage;
    using ClientMessage = vmg_protocol::v1::ClientMessage<kRepeatedMax>;

    static constexpr size_t kClientBufSize{
        ClientMessage::max_serialized_size() * kMessagePoolSize};
    static constexpr size_t kStandBufSize{StandMessage::max_serialized_size() *
                                          kMessagePoolSize};

   public:
    int setPort(uint16_t port);
    int setAddr(uint32_t addr);
    int setSocketType(int type);

    bool setExchangerType(ExchangerType type);
    int64_t send();
    int64_t recv();

    void connectClient();
    void disconnectClient();
    void startExchanger();
    bool isMessageReady();
    bool processClientMessage();
    bool encodeStandMessage();

    ClientMessage& getClientMessage();
    ClientMessage& getStandMessage();

   private:
    std::variant<ExchangerTCP> _exchanger;
    EmbeddedProto::WriteBufferFixedSize<kStandBufSize> _writeBuffer;
    EmbeddedProto::ReadBufferFixedSize<kClientBufSize> _readBuffer;
    ClientMessage _client_message;
    StandMessage _stand_message;
  };

}  // namespace vmg_system::exchanger
