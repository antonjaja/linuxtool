#include <sys/stat.h>
#include <linux/ioctl.h>
#include <time.h>

#include "mp.h"

#define EFM32_RTC_SETTIME	(0xE0)
#define EFM32_RTC_SETALARM	(0xE1)
#define EFM32_RTC_GETTIME	(0xE2)
#define EFM32_RTC_GETALARM	(0xE3)
#define EFM32_RESET			(0xE4)
#define EFM32_CRYPT_SET		(0xE5)
#define EFM32_CRYPT_GET		(0xE6)
#define EFM32_BWVER_GET		(0xE7)
/* main mode : return 'w' + mcu version[3]
 * boot mode : return 'b' + update FW size[3] => waitting for MCU coding
 */
#define EFM32_WRITE_DATA	(0xE8)
#define EFM32_STATUS_SET	(0xE9)
/* 0: power off at main mode
 * 1: power reset at main mode
 * 2: remember power status at main mode
 * 3: ignore power status at main mode
 * 4: update FW transfer complete, request the MCU to Decryption and Write to main flash at boot mode => waitting for MCU coding
 * 5: OS startup finished at main mode => waitting for MCU coding
 */
#define EFM32_STATUS_GET	(0xEA)
/* 0:
 * 1:
 * 2: get power status at main mode
      00: ignore
      01: remember
 * 3:
 * 4: Return Status at boot mode => waitting for MCU coding
      00 First write
      01 Flash writing
      02 16bytes write is complete(write to temp flash)
      03 Total data write is complete(write to main flash)
 * 5:
 */


/*
 * The struct used to pass data via the following ioctl. Similar to the
 * struct tm in <time.h>, but it needs to be here so that the kernel
 * source is self contained, allowing cross-compiles, etc. etc.
 */

struct rtc_time {
	int tm_sec;
	int tm_min;
	int tm_hour;
	int tm_mday;
	int tm_mon;
	int tm_year;
	int tm_wday;
	int tm_yday;
	int tm_isdst;
};

/*
 * This data structure is inspired by the EFI (v0.92) wakeup
 * alarm API.
 */
struct rtc_wkalrm {
	unsigned char enabled;	/* 0 = alarm disabled, 1 = alarm enabled */
	unsigned char pending;  /* 0 = alarm not pending, 1 = alarm pending */
	struct rtc_time time;	/* time the alarm is set to */
};



#define RTC_AIE_ON      _IO('p', 0x01)  /* Alarm int. enable on         */
#define RTC_AIE_OFF     _IO('p', 0x02)  /* ... off                      */
#define RTC_UIE_ON      _IO('p', 0x03)  /* Update int. enable on        */
#define RTC_UIE_OFF     _IO('p', 0x04)  /* ... off                      */
#define RTC_PIE_ON      _IO('p', 0x05)  /* Periodic int. enable on      */
#define RTC_PIE_OFF     _IO('p', 0x06)  /* ... off                      */
#define RTC_WIE_ON      _IO('p', 0x0f)  /* Watchdog int. enable on      */
#define RTC_WIE_OFF     _IO('p', 0x10)  /* ... off                      */

#define RTC_ALM_SET     _IOW('p', 0x07, struct rtc_time) /* Set alarm time  */
#define RTC_ALM_READ    _IOR('p', 0x08, struct rtc_time) /* Read alarm time */
#define RTC_RD_TIME     _IOR('p', 0x09, struct rtc_time) /* Read RTC time   */
#define RTC_SET_TIME    _IOW('p', 0x0a, struct rtc_time) /* Set RTC time    */
#define RTC_IRQP_READ   _IOR('p', 0x0b, unsigned long)   /* Read IRQ rate   */
#define RTC_IRQP_SET    _IOW('p', 0x0c, unsigned long)   /* Set IRQ rate    */
#define RTC_EPOCH_READ  _IOR('p', 0x0d, unsigned long)   /* Read epoch      */
#define RTC_EPOCH_SET   _IOW('p', 0x0e, unsigned long)   /* Set epoch       */

#define RTC_WKALM_SET	_IOW('p', 0x0f, struct rtc_wkalrm)/* Set wakeup alarm*/
#define RTC_WKALM_RD	_IOR('p', 0x10, struct rtc_wkalrm)/* Get wakeup alarm*/

#define EFM_SEND			_IO('p', 0x11)  /* EFM32 send data			*/
#define EFM32_VER_GET		_IO('p', 0x12)  /* EFM32 get boot/main and version*/
#define EFM_UPDATE_START	_IO('p', 0x13)  /* EFM32 update start		*/
#define EFM_UPDATE_FINISHED	_IO('p', 0x14)  /* EFM32 update finished	*/
#define EFM_SEND_ALARM		_IO('p', 0x15)  /* EFM32 send Alarm time	*/
#define EFM_GET_ALARM		_IO('p', 0x16)  /* EFM32 get Alarm time		*/
#define EFM_UPDATE_SAVETIME	_IO('p', 0x17)  /* EFM32 get save time		*/
#define EFM_UPDATE_STATUS	_IO('p', 0x18)  /* EFM32 set power status	*/

#define O_RDWR		     	02

static struct option EFM_LONG_OPTSTR[] = {
	{"setalarm", 1, 0, 1},
	{"getalarm", 1, 0, 2},
	{"status", 1, 0, 3},
	{"update", 1, 0, 4},
	{"setclock", 1, 0, 5},
	{"getclock", 1, 0, 6},
	{0 ,0 , 0, 0}
};

//support for -a -b -c -d
#define  EFM_SHORT_OPTSTR "ab:c:dv"
