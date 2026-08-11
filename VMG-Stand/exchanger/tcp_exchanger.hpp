#pragma once
#include "exchanger_base.hpp"
namespace vmg_system::exchanger {
  inline constexpr uint16_t kDefaultPort{9500};
  class ExchangerTCP : public ExchangerBase {
    static constexpr char const* kLogTag{"TCP Exchanger"};
    friend class ExchangerBase;
    int64_t sendImpl(char const* data, size_t len);
    int64_t recvImpl(char const* data, size_t max_len);
    int64_t startImpl();

    bool connectClientImpl();
    void disconnectClientImpl();

   public:
    void closeSocket(int err = 0);
    void setPort(uint16_t port);
    void setAddr(uint32_t addr);
    void setSocketType(int type);
    ~ExchangerTCP();

   private:
    sockaddr_in _addr_info{.sin_addr.s_addr = htonl(INADDR_ANY),
                           .sin_family      = AF_INET,
                           .sin_port        = htons(kDefaultPort)};
    int _type{};

    int _socket{};
    int _client{};
  };
}  // namespace vmg_system::exchanger
