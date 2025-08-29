#include <zephyr/logging/log.h>

#include <tt_sdk/plugins/logging.h>

LOG_MODULE_REGISTER(ttsdk);

void zephyrLog(void*, TtLogLevel level, const char* msg);

const TtLogging ttPluginLogging = {
    .context = NULL,
    .name = "zephyr",
    .init = NULL,
    .log = &zephyrLog,
};

void zephyrLog(void*, TtLogLevel level, const char* msg)
{
    switch (level) {
    case TT_LL_ERROR:
        LOG_ERR("%s", msg);
        break;
    case TT_LL_WARN:
        LOG_WRN("%s", msg);
        break;
    case TT_LL_INFO:
        LOG_INF("%s", msg);
        break;
    case TT_LL_DEBUG:
        LOG_DBG("%s", msg);
        break;
    }
}
