#include "calc.h"
#include "../lib/stdio.h"
#include "../lib/ctype.h"

static const char *expr_ptr = NULL;

static int parse_expression(void);
static int parse_term(void);
static int parse_factor(void);

static void skip_spaces(void) {
    while (*expr_ptr && isspace(*expr_ptr)) {
        expr_ptr++;
    }
}

static int parse_factor(void) {
    skip_spaces();
    int sign = 1;
    if (*expr_ptr == '+') {
        expr_ptr++;
        skip_spaces();
    } else if (*expr_ptr == '-') {
        sign = -1;
        expr_ptr++;
        skip_spaces();
    }

    if (*expr_ptr == '(') {
        expr_ptr++; // skip '('
        int val = parse_expression();
        skip_spaces();
        if (*expr_ptr == ')') expr_ptr++;
        return val * sign;
    }

    int res = 0;
    while (isdigit(*expr_ptr)) {
        res = res * 10 + (*expr_ptr - '0');
        expr_ptr++;
    }

    return res * sign;
}

static int parse_term(void) {
    int res = parse_factor();
    skip_spaces();

    while (*expr_ptr == '*' || *expr_ptr == '/' || *expr_ptr == '%') {
        char op = *expr_ptr++;
        int next_factor = parse_factor();
        if (op == '*') {
            res *= next_factor;
        } else if (op == '/') {
            if (next_factor != 0) {
                res /= next_factor;
            } else {
                printf("Error: Division by zero\n");
                return 0;
            }
        } else if (op == '%') {
            if (next_factor != 0) {
                res %= next_factor;
            } else {
                printf("Error: Modulo by zero\n");
                return 0;
            }
        }
        skip_spaces();
    }
    return res;
}

static int parse_expression(void) {
    int res = parse_term();
    skip_spaces();

    while (*expr_ptr == '+' || *expr_ptr == '-') {
        char op = *expr_ptr++;
        int next_term = parse_term();
        if (op == '+') {
            res += next_term;
        } else {
            res -= next_term;
        }
        skip_spaces();
    }
    return res;
}

void app_calc(const char *expr) {
    if (!expr || *expr == '\0') {
        printf("Usage: calc <math expression>\nExample: calc (10 + 5) * 4\n");
        return;
    }

    expr_ptr = expr;
    int result = parse_expression();
    printf("%s = %d\n", expr, result);
}
