#ifndef __MAP_OF_SET_OF_INT_H
#define __MAP_OF_SET_OF_INT_H

#include "hash_map.h"

// A key consisting of a number of "coordinates"
typedef struct map_key {
    // The values of the coordinates
    uint16_t *index;
} map_key;

typedef uint16_t map_key_n_dims;

// Rename to collection for convenience
typedef map_key collection;

// Make a 2D key
collection make_2d(uint16_t d1, uint16_t d2);

// Make a 3D key
collection make_3d(uint16_t d1, uint16_t d2, uint16_t d3);

// Free a previously made collection
void free_collection(collection c);

// Create a new Map instance
SimpleSet *init_map(map_key_n_dims *n_dims, uint64_t init_size);

// Add an item to the map and associate it with a label
// Returns 1 if the item was new, or 0 if it already existed
int add_item(SimpleSet *map, map_key key, uint32_t label);

// Get the labels currently assigned to the given coordinates
// labels is a pointer to a list of pointers to labels(!)
// n_labels is a pointer to how many labels there are
// Returns 1 if there are any labels at all, or 0 if there are none (in which
// case *labels will be invalid)
int get_labels(SimpleSet *map, map_key key, uint32_t ***labels, uint64_t *n_labels);

// Get the non-empty keys in the map
map_key **get_keys(SimpleSet *map, uint64_t *n_keys);

#endif // __MAP_OF_SET_OF_INT_H
