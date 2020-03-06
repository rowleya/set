#include "../src/map_of_set_of_int.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

typedef map_key collection;

collection make_2d(uint16_t d1, uint16_t d2) {
    collection c;
    c.n_dims = 2;
    c.index = malloc(2 * sizeof(uint16_t));
    c.index[0] = d1;
    c.index[1] = d2;
    return c;
}

collection make_3d(uint16_t d1, uint16_t d2, uint16_t d3) {
    collection c;
    c.n_dims = 3;
    c.index = malloc(3 * sizeof(uint16_t));
    c.index[0] = d1;
    c.index[1] = d2;
    c.index[2] = d3;
    return c;
}

void free_collection(collection c) {
    free(c.index);
}

int main() {
    SimpleSet *map2d = init_map();
    for (int i = 0; i < 100; i++) {
        uint16_t x = rand() & 0xF;
        uint16_t y = rand() & 0xF;
        collection key = make_2d(x, y);
        uint32_t label = i % 10;
        if (!add_item(map2d, key, label)) {
            printf("Not adding %d, %d = %d twice!\n", x, y, label);
        }
        free_collection(key);
    }
    uint32_t **labels;
    uint64_t n_labels;
    for (int x = 0; x < 0xF; x++) {
        for (int y = 0; y < 0xF; y++) {
            collection key = make_2d(x, y);
            if (get_labels(map2d, key, &labels, &n_labels)) {
                printf("Labels for (%d, %d): [", x, y);
                for (uint64_t z = 0; z < n_labels; z++) {
                    printf("%d", *labels[z]);
                    if (z + 1 < n_labels) {
                        printf(", ");
                    }
                }
                printf("]\n");
            }
        }
    }
}
