#pragma once

namespace vmg_system::exchanger {
  class ExchangerBase {
   public:
    int64_t
    send(this auto& self, char const* data, size_t len)
    {
      return self.sendImpl(data, len);
    }

    int64_t
    recv(this auto& self, char const* data, size_t max_len)
    {
      return self.recv(data, max_len);
    }
    int64_t
    start(this auto& self)
    {
      return self.startImpl();
    }

    void
    connectClient(this auto& self)
    {
      self.connectClientImpl();
    };
    void
    disconnectClient(this auto& self)
    {
      self.disconnectClientImpl();
    }
  };
}  // namespace vmg_system::exchanger
