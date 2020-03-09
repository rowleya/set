/*******************************************************************************
***
***     Author: Tyler Barrus
***     email:  barrust@gmail.com
***
***     Version: 0.1.9
***
***     License: MIT 2016
***
*******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "hash_map.h"

#define MAX_FULLNESS_PERCENT 0.75       /* arbitrary */

/* PRIVATE FUNCTIONS */
static int __get_index(SimpleSet *set, void *key, uint64_t hash, uint64_t *index);
static int __assign_node(SimpleSet *set, void *key, uint64_t index, void *data);
static void __free_index(SimpleSet *set, uint64_t index);
static int __set_contains(SimpleSet *set, void *key, uint64_t hash);
static int __set_add(SimpleSet *set, void *key, uint64_t hash, void *data);
static void __relayout_nodes(SimpleSet *set, uint64_t start, short end_on_null);
static void __set_clear(SimpleSet *set);

/*******************************************************************************
***        FUNCTIONS DEFINITIONS
*******************************************************************************/

int set_init(SimpleSet *set, void *global, uint64_t init_size,
        key_hash_function hash, key_equals_function equals,
        key_copy_function copy, key_free_function free) {
    uint64_t init_elements = init_size / MAX_FULLNESS_PERCENT;
    set->nodes = (simple_set_node**) malloc(init_elements * sizeof(simple_set_node*));
    if (set->nodes == NULL) {
        return SET_MALLOC_ERROR;
    }
    set->number_nodes = init_elements;
    uint64_t i;
    for (i = 0; i < set->number_nodes; i++) {
        set->nodes[i] = NULL;
    }
    set->used_nodes = 0;
    set->global = global;
    set->hash_function = hash;
    set->equals_function = equals;
    set->copy_function = copy;
    set->free_function = free;
    return SET_TRUE;
}

int set_clear(SimpleSet *set) {
    __set_clear(set);
    return SET_TRUE;
}

int set_destroy(SimpleSet *set) {
    __set_clear(set);
    free(set->nodes);
    set->number_nodes = 0;
    set->used_nodes = 0;
    set->hash_function = NULL;
    return SET_TRUE;
}

int set_add(SimpleSet *set, void *key) {
    uint64_t hash = set->hash_function(key, set->global);
    return __set_add(set, key, hash, NULL);
}

int set_add_with_data(SimpleSet *set, void *key, void *data) {
    uint64_t hash = set->hash_function(key, set->global);
    return __set_add(set, key, hash, data);
}

int set_contains(SimpleSet *set, void *key) {
    uint64_t index, hash = set->hash_function(key, set->global);
    return __get_index(set, key, hash, &index);
}

int set_remove(SimpleSet *set, void *key) {
    uint64_t index, hash = set->hash_function(key, set->global);
    int pos = __get_index(set, key, hash, &index);
    if (pos != SET_TRUE) {
        return pos;
    }
    // remove this node
    __free_index(set, index);
    // re-layout nodes
    __relayout_nodes(set, index, 0);
    set->used_nodes--;
    return SET_TRUE;
}

int set_get_data(SimpleSet *set, void *key, void **data) {
    uint64_t index, hash = set->hash_function(key, set->global);
    int result = __get_index(set, key, hash, &index);
    if (result == SET_TRUE) {
        *data = set->nodes[index]->_data;
    }
    return result;
}

uint64_t set_length(SimpleSet *set) {
    return set->used_nodes;
}

void *set_to_array(SimpleSet *set, uint64_t *size) {
    *size = set->used_nodes;
    void** results = malloc(set->used_nodes * sizeof(void *));
    uint64_t i, j = 0;
    for (i = 0; i < set->number_nodes; i++) {
        if (set->nodes[i] != NULL) {
            results[j] = set->copy_function(set->nodes[i]->_key, set->global);
            j++;
        }
    }
    return results;
}

int set_union(SimpleSet *res, SimpleSet *s1, SimpleSet *s2) {
    if (res->used_nodes != 0) {
        return SET_OCCUPIED_ERROR;
    }
    // loop over both s1 and s2 and get keys and insert them into res
    uint64_t i;
    for (i = 0; i < s1->number_nodes; i++) {
        if (s1->nodes[i] != NULL) {
            uint64_t hash = res->hash_function(s1->nodes[i]->_key, res->global);
            __set_add(res, s1->nodes[i]->_key, hash, s1->nodes[i]->_data);
        }
    }
    for (i = 0; i < s2->number_nodes; i++) {
        if (s2->nodes[i] != NULL) {
            uint64_t hash = res->hash_function(s2->nodes[i]->_key, res->global);
            __set_add(res, s2->nodes[i]->_key, hash, s2->nodes[i]->_data);
        }
    }
    return SET_TRUE;
}

int set_intersection(SimpleSet *res, SimpleSet *s1, SimpleSet *s2) {
    if (res->used_nodes != 0) {
        return SET_OCCUPIED_ERROR;
    }
    // loop over both one of s1 and s2: get keys, check the other, and insert them into res if it is
    uint64_t i;
    for (i = 0; i < s1->number_nodes; i++) {
        if (s1->nodes[i] != NULL) {
            uint64_t hash = s1->hash_function(s1->nodes[i]->_key, s1->global);
            if (__set_contains(s2, s1->nodes[i]->_key, hash) == SET_TRUE) {
                __set_add(res, s1->nodes[i]->_key, hash, s1->nodes[i]->_data);
            }
        }
    }
    return SET_TRUE;
}

/* difference is s1 - s2 */
int set_difference(SimpleSet *res, SimpleSet *s1, SimpleSet *s2) {
    if (res->used_nodes != 0) {
        return SET_OCCUPIED_ERROR;
    }
    // loop over s1 and keep only things not in s2
    uint64_t i;
    for (i = 0; i < s1->number_nodes; i++) {
        if (s1->nodes[i] != NULL) {
            uint64_t hash = s1->hash_function(s1->nodes[i]->_key, s1->global);
            if (__set_contains(s2, s1->nodes[i]->_key, hash) != SET_TRUE) {
                __set_add(res, s1->nodes[i]->_key, hash, s1->nodes[i]->_data);
            }
        }
    }
    return SET_TRUE;
}

int set_symmetric_difference(SimpleSet *res, SimpleSet *s1, SimpleSet *s2) {
    if (res->used_nodes != 0) {
        return SET_OCCUPIED_ERROR;
    }
    uint64_t i;
    // loop over set 1 and add elements that are unique to set 1
    for (i = 0; i < s1->number_nodes; i++) {
        if (s1->nodes[i] != NULL) {
            uint64_t hash = s1->hash_function(s1->nodes[i]->_key, s1->global);
            if (__set_contains(s2, s1->nodes[i]->_key, hash) != SET_TRUE) {
                __set_add(res, s1->nodes[i]->_key, hash, s1->nodes[i]->_data);
            }
        }
    }
    // loop over set 2 and add elements that are unique to set 2
    for (i = 0; i < s2->number_nodes; i++) {
        if (s2->nodes[i] != NULL) {
            uint64_t hash = s2->hash_function(s2->nodes[i]->_key, s2->global);
            if (__set_contains(s1, s2->nodes[i]->_key, hash) != SET_TRUE) {
                __set_add(res, s2->nodes[i]->_key, hash, s2->nodes[i]->_data);
            }
        }
    }
    return SET_TRUE;
}

int set_is_subset(SimpleSet *test, SimpleSet *against) {
    uint64_t i;
    for (i = 0; i < test->number_nodes; i++) {
        if (test->nodes[i] != NULL) {
            uint64_t hash = test->hash_function(test->nodes[i]->_key, test->global);
            if (__set_contains(against, test->nodes[i]->_key, hash) == SET_FALSE) {
                return SET_FALSE;
            }
        }
    }
    return SET_TRUE;
}

int set_is_subset_strict(SimpleSet *test, SimpleSet *against) {
    if (test->used_nodes >= against->used_nodes) {
        return SET_FALSE;
    }
    return set_is_subset(test, against);
}

int set_is_superset(SimpleSet *test, SimpleSet *against) {
    return set_is_subset(against, test);
}

int set_is_superset_strict(SimpleSet *test, SimpleSet *against) {
    return set_is_subset_strict(against, test);
}

int set_cmp(SimpleSet *left, SimpleSet *right) {
    if (left->used_nodes < right->used_nodes) {
        return -1;
    } else if (right->used_nodes < left->used_nodes) {
        return 1;
    }
    uint64_t i;
    for (i = 0; i < left->number_nodes; i++) {
        if (left->nodes[i] != NULL) {
            if (set_contains(right, left->nodes[i]->_key) != SET_TRUE) {
                return 2;
            }
        }
    }

    return 0;
}


/*******************************************************************************
***        PRIVATE FUNCTIONS
*******************************************************************************/
static int __set_contains(SimpleSet *set, void *key, uint64_t hash) {
    uint64_t index;
    return __get_index(set, key, hash, &index);
}

static int __set_add(SimpleSet *set, void *key, uint64_t hash, void *data) {
    uint64_t index;
    if (__set_contains(set, key, hash) == SET_TRUE) {
        return SET_ALREADY_PRESENT;
    }
    // Expand nodes if we are close to our desired fullness
    if ((float)set->used_nodes / set->number_nodes > MAX_FULLNESS_PERCENT) {
        uint64_t num_els = set->number_nodes * 2; // we want to double each time
        simple_set_node** tmp = realloc(set->nodes, num_els * sizeof(simple_set_node*));
        if (tmp == NULL || set->nodes == NULL) { // malloc failure
            return SET_MALLOC_ERROR;
        }
        set->nodes = tmp;
        uint64_t i, orig_num_els = set->number_nodes;
        for (i = orig_num_els; i < num_els; i++) {
            set->nodes[i] = NULL;
        }
        set->number_nodes = num_els;
        // re-layout all nodes
        __relayout_nodes(set, 0, 1);
    }
    // add element in
    int res = __get_index(set, key, hash, &index);
    if (res == SET_FALSE) { // this is the first open slot
        __assign_node(set, key, index, data);
        set->used_nodes++;
        return SET_TRUE;
    } else {
        return res;
    }
}

static int __get_index(SimpleSet *set, void *key, uint64_t hash, uint64_t *index) {
    uint64_t i, idx;
    idx = hash % set->number_nodes;
    i = idx;
    while (1) {
        if (set->nodes[i] == NULL) {
            *index = i;
            return SET_FALSE; // not here OR first open slot
        } else if (set->equals_function(set->nodes[i]->_key, key, set->global)) {
            *index = i;
            return SET_TRUE;
        } else {
            i++;
            if (i == set->number_nodes) {
                i = 0;
            }

            if (i == idx) { // this means we went all the way around and the set is full
                return SET_CIRCULAR_ERROR;
            }
        }
    }
}

static int __assign_node(SimpleSet *set, void *key, uint64_t index, void *data) {
    set->nodes[index] = malloc(sizeof(simple_set_node));
    set->nodes[index]->_key = set->copy_function(key, set->global);
    set->nodes[index]->_data = data;
    return SET_TRUE;
}

static void __free_index(SimpleSet *set, uint64_t index) {
    set->free_function(set->nodes[index]->_key, set->global);
    free(set->nodes[index]);
    set->nodes[index] = NULL;
}

static void __relayout_nodes(SimpleSet *set, uint64_t start, short end_on_null) {
    uint64_t index = 0, i;
    for (i = start; i < set->number_nodes; i++) {
        if(set->nodes[i] != NULL) {
            uint64_t hash = set->hash_function(set->nodes[i]->_key, set->global);
            __get_index(set, set->nodes[i]->_key, hash, &index);
            if (i != index) { // we are moving this node
                __assign_node(set, set->nodes[i]->_key, index, set->nodes[i]->_data);
                __free_index(set, i);
            }
        } else if (end_on_null == 0 && i != start) {
            break;
        }
    }
}

static void __set_clear(SimpleSet *set) {
    uint64_t i;
    for(i = 0; i < set->number_nodes; i++) {
        if (set->nodes[i] != NULL) {
            __free_index(set, i);
        }
    }
    set->used_nodes = 0;
}
