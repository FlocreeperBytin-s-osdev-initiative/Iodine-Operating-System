#include "bsdutils.h"
#include "../../drivers/rtc.h"
#include "../../lib/stdio.h"
#include "../../lib/stdlib.h"

static const char *month_names[] = {
    "", "January", "February", "March", "April", "May", "June",
    "July", "August", "September", "October", "November", "December"
};

static const int days_in_month[] = {
    0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

static bool is_leap_year(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

static int day_of_week(int d, int m, int y) {
    if (m < 3) {
        m += 12;
        y--;
    }
    int k = y % 100;
    int j = y / 100;
    int h = (d + (13 * (m + 1)) / 5 + k + k / 4 + j / 4 + 5 * j) % 7;
    // 0 = Saturday, 1 = Sunday, 2 = Monday, ...
    int dow = (h + 6) % 7; // Convert to 0 = Sunday, 1 = Monday, ...
    return dow;
}

void app_cal(int argc, char **argv) {
    datetime_t dt;
    rtc_get_datetime(&dt);

    int month = dt.month;
    int year  = dt.year;

    if (argc > 1) {
        int m = atoi(argv[1]);
        if (m >= 1 && m <= 12) month = m;
    }
    if (argc > 2) {
        int y = atoi(argv[2]);
        if (y > 0) year = y;
    }

    int total_days = days_in_month[month];
    if (month == 2 && is_leap_year(year)) {
        total_days = 29;
    }

    int start_dow = day_of_week(1, month, year);

    printf("    %s %d\n", month_names[month], year);
    printf("Su Mo Tu We Th Fr Sa\n");

    for (int i = 0; i < start_dow; i++) {
        printf("   ");
    }

    int cur_dow = start_dow;
    for (int day = 1; day <= total_days; day++) {
        printf("%2d ", day);
        cur_dow++;
        if (cur_dow == 7) {
            printf("\n");
            cur_dow = 0;
        }
    }
    if (cur_dow != 0) {
        printf("\n");
    }
}
