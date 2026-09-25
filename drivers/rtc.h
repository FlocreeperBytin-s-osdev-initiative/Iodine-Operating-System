#ifndef DRIVERS_RTC_H
#define DRIVERS_RTC_H

#include <types.h>

typedef struct {
    uint8_t second;
    uint8_t minute;
    uint8_t hour;
    uint8_t day;
    uint8_t month;
    uint32_t year;
} datetime_t;

void rtc_init(void);
void rtc_get_datetime(datetime_t *dt);
void rtc_get_formatted(char *buffer, size_t max_len);

#endif /* DRIVERS_RTC_H */
