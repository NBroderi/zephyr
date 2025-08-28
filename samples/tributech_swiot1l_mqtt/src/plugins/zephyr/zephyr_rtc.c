#include "tt_zephyr/zephyr_rtc.h"

#include <tt_sdk/plugins/rtc.h>
#include <zephyr/device.h>
#include <zephyr/drivers/rtc.h>

static TtError zephyrGet(void*, TtRtcValue* value);

TtRtc ttPluginRtc = {
    .context = NULL,
    .name = "zephyr",
    .init = NULL,
    .get = &zephyrGet,
};

static const struct device* zephyrRtcDevice = NULL;

static TtError zephyrGet(void*, TtRtcValue* value)
{
    // check if plugin is initialized
    if (zephyrRtcDevice == NULL) {
        return TT_E_NOT_INITIALIZED;
    }

    // get the current time from the RTC device
    struct rtc_time rtcTime = {};
    if (rtc_get_time(zephyrRtcDevice, &rtcTime) != 0) {
        return TT_E_FAULT;
    }

    // return the value
    value->second = rtcTime.tm_sec;
    value->minute = rtcTime.tm_min;
    value->hour = rtcTime.tm_hour;
    value->day = rtcTime.tm_mday;
    value->month = rtcTime.tm_mon;
    value->year = rtcTime.tm_year + 1900;

    return TT_E_OK;
}

TT_NO_DISCARD TtError ttZephyrRtcInit(const struct device* dev)
{
    // check the required argument
    if (dev == NULL) {
        return TT_E_INVALID_ARG;
    }

    // store the device pointer
    zephyrRtcDevice = dev;

    return TT_E_OK;
}
