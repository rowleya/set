#include "map_of_bitset.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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

static uint64_t map_key_hash(void *_key) {
    map_key *key = _key;
    // FNV-1a hash (http://www.isthe.com/chongo/tech/comp/fnv/)
    uint64_t h = 14695981039346656073ULL; // FNV_OFFSET 64 bit
    for (uint32_t i = 0; i < key->n_dims; i++){
        h = h ^ key->index[i];
        h = h * 1099511628211ULL; // FNV_PRIME 64 bit
    }
    return h;
}

static int map_key_equals(void *_key_1, void *_key_2) {
    map_key *key_1 = _key_1;
    map_key *key_2 = _key_2;
    if (key_1->n_dims != key_2->n_dims) {
        return 0;
    }
    for (uint32_t i = 0; i < key_1->n_dims; i++) {
        if (key_1->index[i] != key_2->index[i]) {
            return 0;
        }
    }
    return 1;
}

static void *map_key_copy(void *_key) {
    map_key *key = _key;
    map_key *copy = malloc(sizeof(map_key));
    copy->n_dims = key->n_dims;
    uint32_t n_bytes = key->n_dims * sizeof(uint32_t);
    copy->index = malloc(n_bytes);
    memcpy(copy->index, key->index, n_bytes);
    return copy;
}

static void map_key_free(void *_key) {
    map_key *key = _key;
    free(key->index);
    free(key);
}

SimpleSet *init_map(void) {
    SimpleSet *map = malloc(sizeof(SimpleSet));
    set_init(map, map_key_hash, map_key_equals, map_key_copy, map_key_free);
    return map;
}

int add_item(SimpleSet *map, map_key key, uint32_t label) {
    if (label >= 32) {
        printf("Labels limited to values between 0 and 32");
    }
    int result = set_contains(map, &key);
    if (result == SET_FALSE) {
        uint32_t *label_set = malloc(sizeof(uint32_t));
        *label_set = 1 << label;
        set_add_with_data(map, &key, label_set);
        return 1;
    }
    uint32_t *label_set;
    set_get_data(map, &key, (void **) &label_set);
    if (*label_set & (1 << label)) {
        return 0;
    }
    *label_set |= (1 << label);
    return 1;
}

static uint32_t count_set_bits(int n) {
    uint32_t count = 0;
    while (n) {
        n &= (n - 1);
        count++;
    }
    return count;
}

int get_labels(SimpleSet *map, map_key key, uint32_t **labels, uint32_t *n_labels) {
    uint32_t *label_set;
    if (set_get_data(map, &key, (void **) &label_set) == SET_TRUE) {
        *n_labels = count_set_bits(*label_set);
        *labels = malloc(*n_labels * sizeof(uint32_t));
        uint32_t j = 0;
        for (int i = 0; i < 32; i++) {
            if (*label_set & (1 << i)) {
                (*labels)[j++] = i;
            }
        }
        return 1;
    }
    return 0;
}
