#include "tcp_connection.h"

static void log_socket_error(const char *tag, const int sock, const int err, const char *message)
{
    ESP_LOGE(tag, "[sock=%d]: %s\n"
                  "error=%d: %s",
             sock, message, err, strerror(err));
}

static int try_receive(const char *tag, const int sock, char *data, size_t max_len)
{
    int len = recv(sock, data, max_len, 0);
    if (len < 0)
    {
        if (errno == EINPROGRESS || errno == EAGAIN || errno == EWOULDBLOCK)
        {
            return 0; // Not an error
        }
        if (errno == ENOTCONN)
        {
            ESP_LOGW(tag, "[sock=%d]: Connection closed", sock);
            return -2; // Socket has been disconnected
        }
        log_socket_error(tag, sock, errno, "Error occurred during receiving");
        return -1;
    }

    return len;
}

static int socket_send(const char *tag, const int sock, const char *data, const size_t len)
{
    int to_write = len;
    while (to_write > 0)
    {
        int written = send(sock, data + (len - to_write), to_write, 0);
        if (written < 0 && errno != EINPROGRESS && errno != EAGAIN && errno != EWOULDBLOCK)
        {
            log_socket_error(tag, sock, errno, "Error occurred during sending");
            return -1;
        }
        to_write -= written;
    }
    return len;
}

class TCP : public Transmission_protocols
{
    void send_data(void *pvParameters) final
    {

        // byte a; doesn't work, maybe just use char or 8bit smth idk
        static char rx_buffer[128];

        int len_msg = 0;
        int flags = 0;
        static const char *TAG_TCP_SOCKET = "nonblocking-socket-client";

        xTaskNotify(VOLTAGE_TASK_HANDLE, 0, eSetValueWithOverwrite);
        xTaskNotify(CURRENT_TASK_HANDLE, 0, eSetValueWithOverwrite);
        xTaskNotify(DISTURBNCE_TASK_HANDLE, 0, eSetValueWithOverwrite);

        xTaskNotify(TEMPERATURE1_TASK_HANDLE, 0, eSetValueWithOverwrite);
        xTaskNotify(TEMPERATURE2_TASK_HANDLE, 0, eSetValueWithOverwrite);
        xTaskNotify(TEMPERATURE3_TASK_HANDLE, 0, eSetValueWithOverwrite);

        xTaskNotify(WEIGHT_TASK_HANDLE, 0, eSetValueWithOverwrite);
        xTaskNotify(RPM_TASK_HANDLE, 0, eSetValueWithOverwrite);

        static const char *payload = "GET / HTTP/1.1\r\n\r\n"; // add struct there

        xTaskNotify(VOLTAGE_TASK_HANDLE, 32000, eSetValueWithOverwrite);
        xTaskNotify(CURRENT_TASK_HANDLE, 32000, eSetValueWithOverwrite);
        xTaskNotify(DISTURBNCE_TASK_HANDLE, 32000, eSetValueWithOverwrite);

        xTaskNotify(TEMPERATURE1_TASK_HANDLE, 32000, eSetValueWithOverwrite);
        xTaskNotify(TEMPERATURE2_TASK_HANDLE, 32000, eSetValueWithOverwrite);
        xTaskNotify(TEMPERATURE3_TASK_HANDLE, 32000, eSetValueWithOverwrite);

        xTaskNotify(WEIGHT_TASK_HANDLE, 32000, eSetValueWithOverwrite);
        xTaskNotify(RPM_TASK_HANDLE, 32000, eSetValueWithOverwrite);

        static_cast<byte *>(); // for crc

        struct addrinfo hints = {.ai_socktype = SOCK_STREAM};
        struct addrinfo *address_info;
        int sock = INVALID_SOCK;

        int res = getaddrinfo(CONFIG_EXAMPLE_TCP_CLIENT_CONNECT_ADDRESS, CONFIG_EXAMPLE_TCP_CLIENT_CONNECT_PORT, &hints, &address_info);
        if (res != 0 || address_info == NULL)
        {
            ESP_LOGE(TAG_TCP_SOCKET, "couldn't get hostname for `%s` "
                                     "getaddrinfo() returns %d, addrinfo=%p",
                     CONFIG_EXAMPLE_TCP_CLIENT_CONNECT_ADDRESS, res, address_info);
            goto error;
        }

        // Creating client's socket
        sock = socket(address_info->ai_family, address_info->ai_socktype, address_info->ai_protocol);
        if (sock < 0)
        {
            log_socket_error(TAG_TCP_SOCKET, sock, errno, "Unable to create socket");
            goto error;
        }
        ESP_LOGI(TAG_TCP_SOCKET, "Socket created, connecting to %s:%s", CONFIG_EXAMPLE_TCP_CLIENT_CONNECT_ADDRESS, CONFIG_EXAMPLE_TCP_CLIENT_CONNECT_PORT);

        // Marking the socket as non-blocking
        flags = fcntl(sock, F_GETFL);
        if (fcntl(sock, F_SETFL, flags | O_NONBLOCK) == -1)
        {
            log_socket_error(TAG_TCP_SOCKET, sock, errno, "Unable to set socket non blocking");
        }

        if (connect(sock, address_info->ai_addr, address_info->ai_addrlen) != 0)
        {
            if (errno == EINPROGRESS)
            {
                ESP_LOGD(TAG_TCP_SOCKET, "connection in progress");
                fd_set fdset;
                FD_ZERO(&fdset);
                FD_SET(sock, &fdset);

                // Connection in progress -> have to wait until the connecting socket is marked as writable, i.e. connection completes
                res = select(sock + 1, NULL, &fdset, NULL, NULL);
                if (res < 0)
                {
                    log_socket_error(TAG_TCP_SOCKET, sock, errno, "Error during connection: select for socket to be writable");
                    goto error;
                }
                else if (res == 0)
                {
                    log_socket_error(TAG_TCP_SOCKET, sock, errno, "Connection timeout: select for socket to be writable");
                    goto error;
                }
                else
                {
                    int sockerr;
                    socklen_t len = (socklen_t)sizeof(int);

                    if (getsockopt(sock, SOL_SOCKET, SO_ERROR, (void *)(&sockerr), &len) < 0)
                    {
                        log_socket_error(TAG_TCP_SOCKET, sock, errno, "Error when getting socket error using getsockopt()");
                        goto error;
                    }
                    if (sockerr)
                    {
                        log_socket_error(TAG_TCP_SOCKET, sock, sockerr, "Connection error");
                        goto error;
                    }
                }
            }
            else
            {
                log_socket_error(TAG_TCP_SOCKET, sock, errno, "Socket is unable to connect");
                goto error;
            }
        }
        for (;;)
        {
            ESP_LOGI(TAG_TCP_SOCKET, "Client sends data to the server...");
            len_msg = socket_send(TAG_TCP_SOCKET, sock, payload, strlen(payload));
            if (len_msg < 0)
            {
                ESP_LOGE(TAG_TCP_SOCKET, "Error occurred during socket_send");
                goto error;
            }
            ESP_LOGI(TAG_TCP_SOCKET, "struct was sent");

            // Keep receiving until we have a reply
            do
            {
                len_msg = try_receive(TAG_TCP_SOCKET, sock, rx_buffer, sizeof(rx_buffer));
                if (len_msg < 0)
                {
                    ESP_LOGE(TAG_TCP_SOCKET, "Error occurred during try_receive");
                    goto error;
                }
                vTaskDelay(pdMS_TO_TICKS(YIELD_TO_ALL_MS));
            } while (len_msg == 0);
            ESP_LOGI(TAG_TCP_SOCKET, "Received: %.*s", len_msg, rx_buffer);

            vTaskDelay(100);
        }
    error:
        if (sock != INVALID_SOCK)
        {
            close(sock);
        }
        free(address_info);
        vTaskDelete(NULL);
    }
};