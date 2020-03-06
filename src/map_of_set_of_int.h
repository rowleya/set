#ifndef __MAP_OF_SET_OF_INT_H
#define __MAP_OF_SET_OF_INT_H

#include "hash_map.h"

typedef struct map_key {
    uint32_t n_dims;
    uint16_t *index;
} map_key;

SimpleSet *init_map(void);

int add_item(SimpleSet *map, map_key key, uint32_t label);

int get_labels(SimpleSet *map, map_key key, uint32_t ***labels, uint64_t *n_labels);

#endif // __MAP_OF_SET_OF_INT_H
