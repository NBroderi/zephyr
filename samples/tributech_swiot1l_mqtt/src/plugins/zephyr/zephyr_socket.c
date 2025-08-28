#include <stdint.h>

#include <tt_sdk/plugins/socket.h>
#include <zephyr/net/net_ip.h>
#include <zephyr/net/socket.h>

static TtError zephyrResolve(void*, const char* name, TtSocketAddr* addr);
static TtError zephyrCreate(void*, const TtSocketAddr* addr, uintptr_t* handle);
static TtError zephyrClose(void*, uintptr_t handle);
static TtError zephyrSend(void*, uintptr_t handle, const void* data, size_t* len);
static TtError zephyrRecv(void*, uintptr_t handle, void* data, size_t* len);
static TtError zephyrSetTimeout(void*, uintptr_t handle, uint16_t timeoutMs);

TtSocket ttPluginSocket = {
    .context = NULL,
    .name = "zephyr",
    .init = NULL,
    .resolve = &zephyrResolve,
    .create = &zephyrCreate,
    .close = &zephyrClose,
    .send = &zephyrSend,
    .recv = &zephyrRecv,
    .setTimeout = &zephyrSetTimeout,
};

static TtError zephyrResolve(void*, const char* name, TtSocketAddr* addr)
{
    // TODO
    TT_UNUSED(name);
    TT_UNUSED(addr);
    return TT_E_NOT_SUPPORTED;
}

static TtError zephyrCreate(void*, const TtSocketAddr* addr, uintptr_t* handle)
{
    // try to create the socket itself
    int sock = zsock_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0) {
        return TT_E_FAULT;
    }

    // try to establish a connection
    struct sockaddr_in sockAddr = {
        .sin_family = AF_INET,
        .sin_port = htons(addr->port),
        .sin_addr = {
            .s_addr = *(const uint32_t*)(addr->ip), // already in network byte order
        },
    };
    if (zsock_connect(sock, (struct sockaddr*)&sockAddr, sizeof(sockAddr)) != 0) {
        zsock_close(sock);
        return TT_E_FAULT;
    }

    // connection established
    *handle = (uintptr_t)((intptr_t)sock);
    return TT_E_OK;
}

static TtError zephyrClose(void*, uintptr_t handle)
{
    const int sock = (int)((intptr_t)handle);
    if (sock >= 0 && zsock_close(sock) != 0) {
        return TT_E_FAULT;
    }
    return TT_E_OK;
}

static TtError zephyrSend(void*, uintptr_t handle, const void* data, size_t* len)
{
    const int sock = (int)((intptr_t)handle);
    const int ret = zsock_send(sock, data, *len, 0);
    if (ret < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return TT_E_TIMEOUT;
        }
        return TT_E_FAULT;
    }
    *len = (size_t)ret;
    return TT_E_OK;
}

static TtError zephyrRecv(void*, uintptr_t handle, void* data, size_t* len)
{
    const int sock = (int)((intptr_t)handle);
    const int ret = zsock_recv(sock, data, *len, 0);
    if (ret < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return TT_E_TIMEOUT;
        }
        return TT_E_FAULT;
    }
    *len = (size_t)ret;
    return TT_E_OK;
}

static TtError zephyrSetTimeout(void*, uintptr_t handle, uint16_t timeoutMs)
{
    // Ratio between seconds and milliseconds
    static const uint16_t MS_S_RATIO = 1000;
    // Ratio between milliseconds and microseconds
    static const uint16_t MS_US_RATIO = 1000;

    const uint16_t seconds = timeoutMs / MS_S_RATIO;
    const uint16_t us = (timeoutMs % MS_S_RATIO) * MS_US_RATIO;
    struct zsock_timeval timeout = { .tv_sec = seconds, .tv_usec = us };

    const int sock = (int)((intptr_t)(handle));
    if (zsock_setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout) < 0) {
        return TT_E_FAULT;
    }

    return TT_E_OK;
}
