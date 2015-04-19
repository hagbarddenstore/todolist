#include "is_int.h"

#include <stdlib.h>
#include <ctype.h>

bool is_int(const char *input) {
    if (*input == '-') {
        ++input;
    }

    if (!*input) {
        return false;
    }

    while (*input) {
        if (!isdigit(*input)) {
            return false;
        } else {
            ++input;
        }
    }

    return true;
}
