#include "map_of_set_of_int.h"
#include <stdlib.h>
#include <string.h>

collection make_2d(uint16_t d1, uint16_t d2) {
    collection c;
    c.index = malloc(2 * sizeof(uint16_t));
    c.index[0] = d1;
    c.index[1] = d2;
    return c;
}

collection make_3d(uint16_t d1, uint16_t d2, uint16_t d3) {
    collection c;
    c.index = malloc(3 * sizeof(uint16_t));
    c.index[0] = d1;
    c.index[1] = d2;
    c.index[2] = d3;
    return c;
}

void free_collection(collection c) {
    free(c.index);
}

static uint64_t map_key_hash(void *_key, void *_global) {
    map_key *key = _key;
    map_key_n_dims *n_dims = _global;
    uint32_t n_bytes = *n_dims * sizeof(key->index[0]);
    uint8_t *bytes = (uint8_t *) key->index;
    // FNV-1a hash (http://www.isthe.com/chongo/tech/comp/fnv/)
    uint64_t h = 14695981039346656073ULL; // FNV_OFFSET 64 bit
    for (uint32_t i = 0; i < n_bytes; i++){
        h = h ^ bytes[i];
        h = h * 1099511628211ULL; // FNV_PRIME 64 bit
    }
    return h;
}

static int map_key_equals(void *_key_1, void *_key_2, void *_global) {
    map_key *key_1 = _key_1;
    map_key *key_2 = _key_2;
    map_key_n_dims *n_dims = _global;
    for (uint32_t i = 0; i < *n_dims; i++) {
        if (key_1->index[i] != key_2->index[i]) {
            return 0;
        }
    }
    return 1;
}

static void *map_key_copy(void *_key, void *_global) {
    map_key *key = _key;
    map_key_n_dims *n_dims = _global;
    map_key *copy = malloc(sizeof(map_key));
    uint32_t n_bytes = *n_dims * sizeof(uint16_t);
    copy->index = malloc(n_bytes);
    memcpy(copy->index, key->index, n_bytes);
    return copy;
}

static void map_key_free(void *_key, void *_global) {
    use(_global);
    map_key *key = _key;
    free(key->index);
    free(key);
}

typedef uint32_t set_key;

static uint64_t set_key_hash(void *_key, void *_global) {
    use(_global);
    set_key *key = _key;
    uint32_t n_bytes = sizeof(set_key);
    uint8_t *bytes = (uint8_t *) key;
    // FNV-1a hash (http://www.isthe.com/chongo/tech/comp/fnv/)
    uint64_t h = 14695981039346656073ULL; // FNV_OFFSET 64 bit
    for (uint32_t i = 0; i < n_bytes; i++) {
        h = h ^ bytes[i];
        h = h * 1099511628211ULL; // FNV_PRIME 64 bit
    }
    return h;
}

static int set_key_equals(void *_key_1, void *_key_2, void *_global) {
    use(_global);
    set_key *key_1 = _key_1;
    set_key *key_2 = _key_2;
    if (*key_1 == *key_2) {
        return 1;
    }
    return 0;
}

static void *set_key_copy(void *_key, void *_global) {
    use(_global);
    set_key *key = _key;
    set_key *copy = malloc(sizeof(set_key));
    *copy = *key;
    return copy;
}

static void set_key_free(void *key, void *_global) {
    use(_global);
    free(key);
}

SimpleSet *init_map(map_key_n_dims *n_dims, uint64_t init_size) {
    SimpleSet *map = malloc(sizeof(SimpleSet));
    set_init(map, n_dims, init_size, map_key_hash, map_key_equals, map_key_copy, map_key_free);
    return map;
}

int add_item(SimpleSet *map, map_key key, uint32_t label) {
    int result = set_contains(map, &key);
    if (result == SET_FALSE) {
        SimpleSet *label_set = malloc(sizeof(SimpleSet));
        set_init(label_set, NULL, 4, set_key_hash, set_key_equals, set_key_copy, set_key_free);
        set_add(label_set, &label);
        set_add_with_data(map, &key, label_set);
        return 1;
    }
    SimpleSet *label_set;
    set_get_data(map, &key, (void **) &label_set);
    if (set_add(label_set, &label) == SET_TRUE) {
        return 1;
    }
    return 0;
}

int get_labels(SimpleSet *map, map_key key, uint32_t ***labels, uint64_t *n_labels) {
    SimpleSet *label_set;
    if (set_get_data(map, &key, (void **) &label_set) == SET_TRUE) {
        *labels = set_to_array(label_set, n_labels);
        return 1;
    }
    return 0;
}

map_key **get_keys(SimpleSet *map, uint64_t *n_keys) {
    return (map_key **) set_to_array(map, n_keys);
}
