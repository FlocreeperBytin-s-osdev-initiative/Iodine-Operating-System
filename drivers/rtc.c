#include "rtc.h"
#include "../arch/i386/io.h"
#include "../lib/stdio.h"

#define CMOS_ADDRESS 0x70
#define CMOS_DATA    0x71

static int get_update_in_progress_flag(void) {
    outb(CMOS_ADDRESS, 0x0A);
    return (inb(CMOS_DATA) & 0x80);
}

static uint8_t get_rtc_register(int reg) {
    outb(CMOS_ADDRESS, reg);
    return inb(CMOS_DATA);
}

void rtc_init(void) {
    // Basic RTC init
}

void rtc_get_datetime(datetime_t *dt) {
    if (!dt) return;

    while (get_update_in_progress_flag());

    uint8_t second = get_rtc_register(0x00);
    uint8_t minute = get_rtc_register(0x02);
    uint8_t hour   = get_rtc_register(0x04);
    uint8_t day    = get_rtc_register(0x07);
    uint8_t month  = get_rtc_register(0x08);
    uint8_t year   = get_rtc_register(0x09);
    uint8_t century = get_rtc_register(0x32);

    uint8_t register_b = get_rtc_register(0x0B);

    // Convert BCD to binary values if necessary
    if (!(register_b & 0x04)) {
        second  = ((second & 0xF0) >> 1) + ((second & 0xF0) >> 3) + (second & 0xf);
        minute  = ((minute & 0xF0) >> 1) + ((minute & 0xF0) >> 3) + (minute & 0xf);
        hour    = ((((hour & 0xF0) >> 1) + ((hour & 0xF0) >> 3) + (hour & 0xf)) & 0x7F);
        day     = ((day & 0xF0) >> 1) + ((day & 0xF0) >> 3) + (day & 0xf);
        month   = ((month & 0xF0) >> 1) + ((month & 0xF0) >> 3) + (month & 0xf);
        year    = ((year & 0xF0) >> 1) + ((year & 0xF0) >> 3) + (year & 0xf);
        century = ((century & 0xF0) >> 1) + ((century & 0xF0) >> 3) + (century & 0xf);
    }

    // Convert 12 hour clock to 24 hour clock if necessary
    if (!(register_b & 0x02) && (hour & 0x80)) {
        hour = ((hour & 0x7F) + 12) % 24;
    }

    // Calculate full year
    uint32_t full_year;
    if (century != 0) {
        full_year = century * 100 + year;
    } else {
        full_year = 2000 + year;
    }

    dt->second = second;
    dt->minute = minute;
    dt->hour = hour;
    dt->day = day;
    dt->month = month;
    dt->year = full_year;
}

void rtc_get_formatted(char *buffer, size_t max_len) {
    datetime_t dt;
    rtc_get_datetime(&dt);
    snprintf(buffer, max_len, "%04u-%02u-%02u %02u:%02u:%02u UTC",
             dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second);
}
