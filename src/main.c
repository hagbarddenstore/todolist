#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <is_int.h>
#include <item.h>

int main(int argc, char **argv) {
    if (argc == 2) {
        if (is_int(argv[1])) {
            remove_item(atoi(argv[1]));
        } else {
            add_item(argv[1]);
        }

        return 0;
    }

    list_items();

    return 0;
}
