#include "../src/map_of_set_of_int.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

int main() {
    map_key_n_dims n_dims_2d = 2;
    SimpleSet *map2d = init_map(&n_dims_2d, 100);
    for (uint32_t i = 0; i < 100; i++) {
        uint16_t x = rand() & 0xF;
        uint16_t y = rand() & 0xF;
        collection key = make_2d(x, y);
        uint32_t label = i % 10;
        if (!add_item(map2d, key, label)) {
            printf("Not adding %d, %d = %d twice!\n", x, y, label);
        }
        free_collection(key);
    }
    uint64_t n_keys;
    collection **keys = get_keys(map2d, &n_keys);
    uint32_t **labels;
    uint64_t n_labels;
    for (uint32_t i = 0; i < n_keys; i++) {
        if (get_labels(map2d, *(keys[i]), &labels, &n_labels)) {
            printf("Labels for (%d, %d): [", keys[i]->index[0], keys[i]->index[1]);
            for (uint64_t j = 0; j < n_labels; j++) {
                printf("%d", *labels[j]);
                if (j + 1 < n_labels) {
                    printf(", ");
                }
            }
            printf("]\n");
        }
    }

    map_key_n_dims n_dims_3d = 3;
    SimpleSet *map3d = init_map(&n_dims_3d, 1000);
    for (uint32_t i = 0; i < 1000; i++) {
        uint16_t x = rand() & 0xF;
        uint16_t y = rand() & 0xF;
        uint16_t z = rand() & 0xF;
        collection key = make_3d(x, y, z);
        uint32_t label = i % 10;
        if (!add_item(map3d, key, label)) {
            printf("Not adding %d, %d, %d = %d twice!\n", x, y, z, label);
        }
        free_collection(key);
    }
    for (int x = 0; x < 0xF; x++) {
        for (int y = 0; y < 0xF; y++) {
            for (int z = 0; z < 0xF; z++) {
                collection key = make_3d(x, y, z);
                if (get_labels(map3d, key, &labels, &n_labels)) {
                    printf("Labels for (%d, %d, %d): [", x, y, z);
                    for (uint64_t w = 0; w < n_labels; w++) {
                        printf("%d", *labels[w]);
                        if (w + 1 < n_labels) {
                            printf(", ");
                        }
                    }
                    printf("]\n");
                }
            }
        }
    }
}
