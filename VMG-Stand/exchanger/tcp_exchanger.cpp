#include "tcp_exchanger.hpp"

#include "esp_wifi.h"

namespace vmg_system::exchanger {
  int64_t
  ExchangerTCP::sendImpl(char const* data, size_t len)
  {
    int to_write{len};

    while (to_write > 0) {
      int res = send(_client, data + (len - to_write), to_write, 0);
      if (to_write < 0 && errno != EINPROGRESS && errno != EAGAIN &&
          errno != EWOULDBLOCK) {
        ESP_LOGE(kLogTag, "Error! %d", errno);
        return -1;
      }
      to_write -= res;
    }
    return len;
  }

  int64_t
  ExchangerTCP::recvImpl(char const* data, size_t max_len)
  {
    int len = recv(_client, data, max_len, 0);
    if (len < 0) {
      ESP_LOGE(kLogTag, "Possible error! %d", errno);
      return (errno == EINPROGRESS || errno == EAGAIN || errno == EWOULDBLOCK)
                 ? 0
                 : -1;
    }

    return len;
  }

  bool
  ExchangerTCP::connectClientImpl()
  {
    _client = accept(_socket, nullptr, nullptr);

    if (_client > 0) {
      int flags = fcntl(_client, F_GETFL);
      return fcntl(_client, F_SETFL, flags | O_NONBLOCK) != -1);
    }

    ESP_LOGE(kLogTag, "Client is not connected %d", errno);
    return false;
  }

  void
  ExchangerTCP::closeSocket(int err)
  {
    ESP_LOGI(kLogTag, "Socket is closing with error %d", err);
    close(_socket);
    _socket = err;
  }

  ExchangerTCP::~ExchangerTCP() { closeSocket(); }
  void
  ExchangerTCP::disconnectClientImpl()
  {
    ESP_LOGI(kLogTag, "Client was disconnected");
    close(_client);
  }

  int64_t
  ExchangerTCP::startImpl()
  {
    _socket = socket(_addr_info.sin_family, _type, 0);
    if (_socket < 0) {
      ESP_LOGE(kLogTag, "Couldn't create socket");
      return;
    }

    ESP_LOGI(kLogTag, "Created socket");
    int err = bind(_socket, &_addr_info, sizeof(_addr_info));

    if (err != 0) {
      ESP_LOGE(kLogTag, "Couldn't bind");
      closeSocket(err);
      return;
    }

    ESP_LOGI(kLogTag, "Socket bound");

    err = listen(_socket, 1);

    if (err != 0) {
      ESP_LOGE(kLogTag, "Couldn't listen");
      closeSocket(err);
      return;
    }
    ESP_LOGI(kLogTag, "Socket is listening");
  }

  void
  setPort(uint16_t port)
  {
    _addr_info.sin_port = htons(port);
  }

  void
  setAddr(uint32_t addr)
  {
    _addr_info.sin_addr.s_addr = htonl(addr);
  }

  void
  setSocketType(int type)
  {
    _type = type;
  }
}  // namespace vmg_system::exchanger
