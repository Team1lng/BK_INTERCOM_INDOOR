#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "ak_drv_wdt.h"
#include "ak_log.h"
#include "ak_common.h"

#define WATCHDOG_FILE_PATH    "/dev/watchdog"
#define WATCHDOG_IOCTL_BASE 'W'
#define WDIOC_KEEPALIVE     _IOR(WATCHDOG_IOCTL_BASE, 5, int)
#define    WDIOC_SETTIMEOUT    _IOWR(WATCHDOG_IOCTL_BASE, 6, int)
#define WDIOC_GETTIMEOUT    _IOR(WATCHDOG_IOCTL_BASE, 7, int)

static int wdt_fd = -1; /* device fd define */
static int wdt_feed_timeout = 0; /* watch dog feed time define */


/**
 * ak_drv_wdt_open - open watch dog and watch dog start work.
 * @feed_timeout: [in] second, [1, 357]
 * return: 0 - success; otherwise error code;
 * note: 
 */
int ak_drv_wdt_open(unsigned int feed_timeout) {
    /* arguments check */
    if (wdt_fd >= 0) {
        ak_print_notice(MODULE_ID_DRV, "watch dog have been opened.\n");
        return ERROR_TYPE_EBUSY;
    }
    /* feed time should > 0 */
    if (feed_timeout < 1) {
        ak_print_error_ex(MODULE_ID_DRV, "invalid arg.\n");
        return ERROR_TYPE_INVALID_ARG;
    }
    /* open watch dog device */
    wdt_fd = ak_open(WATCHDOG_FILE_PATH, O_RDONLY);
    if (wdt_fd < 0) {
        ak_print_error(MODULE_ID_DRV, "open watch dog  fail.\n");
        return ERROR_TYPE_DEV_OPEN_FAILED;
    }
    /*
     * set timeout
     * set close-on-exec flag
     */
    wdt_feed_timeout = feed_timeout;

    if (ioctl(wdt_fd, WDIOC_SETTIMEOUT, &wdt_feed_timeout)) {
        ioctl(wdt_fd, WDIOC_GETTIMEOUT, &wdt_feed_timeout);
        ak_print_notice_ex(MODULE_ID_DRV, "set timeout fail, use value %d.\n", wdt_feed_timeout);
    }

    return AK_SUCCESS;
}

/**
 * ak_drv_wdt_feed - feed watch dog.
 * return:0 - success; otherwise error code;
 */
int ak_drv_wdt_feed(void) {
    /* not open watch dog */
    if (wdt_fd < 0) {
        ak_print_error_ex(MODULE_ID_DRV, "watch dog not opened.\n");
        return ERROR_TYPE_NOT_INIT;
    }

    if (ioctl(wdt_fd, WDIOC_KEEPALIVE, &wdt_feed_timeout)) {
        ak_print_error_ex(MODULE_ID_DRV, "feed fail.\n");
        return ERROR_TYPE_DEV_CTRL_FAILED;
    }

    return AK_SUCCESS;
}

/**
 * ak_drv_wdt_close - close watch dog.
 * return:0 - success; otherwise error code;
 */
int ak_drv_wdt_close(void) {
    if (wdt_fd < 0) {
        ak_print_error_ex(MODULE_ID_DRV, "watch dog not opened.\n");
        return ERROR_TYPE_NOT_INIT;
    }
    /*
     * set timeout -1 is to close watch dog,
     * its difference with close device.
     * if close device directly, system will reboot
     */
    wdt_feed_timeout = -1;
    /*
     * before we close watch dog,
     * set watch dog alive .
     */
    if (ioctl(wdt_fd, WDIOC_SETTIMEOUT, &wdt_feed_timeout) < 0) {
        ak_print_error_ex(MODULE_ID_DRV, "ioctl fail.\n");
        return ERROR_TYPE_DEV_CTRL_FAILED;
    }
    /* close device */
    if (close(wdt_fd) < 0) {
        ak_print_error_ex(MODULE_ID_DRV, "close watch_dog fail.\n");
        return ERROR_TYPE_DEV_CLOSE_FAILED;
    }
    wdt_fd = -1;

    return AK_SUCCESS;
}

