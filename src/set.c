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
#include "set.h"

#define INITIAL_NUM_ELEMENTS 1024
#define MAX_FULLNESS_PERCENT 0.25       /* arbitrary */

/* PRIVATE FUNCTIONS */
static void __free(item *key);
static void __copy(item *key, item *copy);
static int __equals(item key1, item key2);
static uint64_t __default_hash(item key);
static int __get_index(SimpleSet *set, item key, uint64_t hash, uint64_t *index);
static int __assign_node(SimpleSet *set, item key, uint64_t hash, uint64_t index);
static void __free_index(SimpleSet *set, uint64_t index);
static int __set_contains(SimpleSet *set, item key, uint64_t hash);
static int __set_add(SimpleSet *set, item key, uint64_t hash);
static void __relayout_nodes(SimpleSet *set, uint64_t start, short end_on_null);
static void __set_clear(SimpleSet *set);

/*******************************************************************************
***        FUNCTIONS DEFINITIONS
*******************************************************************************/

int set_init(SimpleSet *set) {
    return set_init_alt(set, NULL);
}

int set_init_alt(SimpleSet *set, set_hash_function hash) {
    set->nodes = (simple_set_node**) malloc(INITIAL_NUM_ELEMENTS * sizeof(simple_set_node*));
    if (set->nodes == NULL) {
        return SET_MALLOC_ERROR;
    }
    set->number_nodes = INITIAL_NUM_ELEMENTS;
    uint64_t i;
    for (i = 0; i < set->number_nodes; i++) {
        set->nodes[i] = NULL;
    }
    set->used_nodes = 0;
    set->hash_function = (hash == NULL) ? &__default_hash : hash;
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

int set_add(SimpleSet *set, item key) {
    uint64_t hash = set->hash_function(key);
    return __set_add(set, key, hash);
}

int set_contains(SimpleSet *set, item key) {
    uint64_t index, hash = set->hash_function(key);
    return __get_index(set, key, hash, &index);
}

int set_remove(SimpleSet *set, item key) {
    uint64_t index, hash = set->hash_function(key);
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

uint64_t set_length(SimpleSet *set) {
    return set->used_nodes;
}

item* set_to_array(SimpleSet *set, uint64_t *size) {
    *size = set->used_nodes;
    item* results = malloc(set->used_nodes * sizeof(item));
    uint64_t i, j = 0;
    size_t len;
    for (i = 0; i < set->number_nodes; i++) {
        if (set->nodes[i] != NULL) {
            item *key = &(set->nodes[i]->_key);
            __copy(key, &results[j]);
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
            __set_add(res, s1->nodes[i]->_key, s1->nodes[i]->_hash);
        }
    }
    for (i = 0; i < s2->number_nodes; i++) {
        if (s2->nodes[i] != NULL) {
            __set_add(res, s2->nodes[i]->_key, s2->nodes[i]->_hash);
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
            if (__set_contains(s2, s1->nodes[i]->_key, s1->nodes[i]->_hash) == SET_TRUE) {
                __set_add(res, s1->nodes[i]->_key, s1->nodes[i]->_hash);
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
            if (__set_contains(s2, s1->nodes[i]->_key, s1->nodes[i]->_hash) != SET_TRUE) {
                __set_add(res, s1->nodes[i]->_key, s1->nodes[i]->_hash);
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
            if (__set_contains(s2, s1->nodes[i]->_key, s1->nodes[i]->_hash) != SET_TRUE) {
                __set_add(res, s1->nodes[i]->_key, s1->nodes[i]->_hash);
            }
        }
    }
    // loop over set 2 and add elements that are unique to set 2
    for (i = 0; i < s2->number_nodes; i++) {
        if (s2->nodes[i] != NULL) {
            if (__set_contains(s1, s2->nodes[i]->_key, s2->nodes[i]->_hash) != SET_TRUE) {
                __set_add(res, s2->nodes[i]->_key, s2->nodes[i]->_hash);
            }
        }
    }
    return SET_TRUE;
}

int set_is_subset(SimpleSet *test, SimpleSet *against) {
    uint64_t i;
    for (i = 0; i < test->number_nodes; i++) {
        if (test->nodes[i] != NULL) {
            if (__set_contains(against, test->nodes[i]->_key, test->nodes[i]->_hash) == SET_FALSE) {
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
static void __copy(item *key, item *copy) {
    int n_bytes = key->n_dims * sizeof(uint16_t);
    copy->index = malloc(n_bytes);
    copy->n_dims = key->n_dims;
    copy->label = key->label;
    memcpy(copy->index, key->index, n_bytes);
}

static void __free(item *key) {
    free(key->index);
}

static int __equals(item key_1, item key_2) {
    if (key_1.n_dims != key_2.n_dims) {
        return 0;
    }
    for (int i = 0; i < key_1.n_dims; i++) {
        if (key_1.index[i] != key_2.index[i]) {
            return 0;
        }
    }
    return 1;
}

static uint64_t __default_hash(item key) {
    // FNV-1a hash (http://www.isthe.com/chongo/tech/comp/fnv/)
    uint64_t h = 14695981039346656073ULL; // FNV_OFFSET 64 bit
    for (int i = 0; i < key.n_dims; i++){
        h = h ^ key.index[i];
        h = h * 1099511628211ULL; // FNV_PRIME 64 bit
    }
    return h;
}

static int __set_contains(SimpleSet *set, item key, uint64_t hash) {
    uint64_t index;
    return __get_index(set, key, hash, &index);
}

static int __set_add(SimpleSet *set, item key, uint64_t hash) {
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
        __assign_node(set, key, hash, index);
        set->used_nodes++;
        return SET_TRUE;
    } else {
        return res;
    }
}

static int __get_index(SimpleSet *set, item key, uint64_t hash, uint64_t *index) {
    uint64_t i, idx;
    idx = hash % set->number_nodes;
    i = idx;
    while (1) {
        if (set->nodes[i] == NULL) {
            *index = i;
            return SET_FALSE; // not here OR first open slot
        } else if (hash == set->nodes[i]->_hash && __equals(set->nodes[i]->_key, key)) {
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

static int __assign_node(SimpleSet *set, item key, uint64_t hash, uint64_t index) {
    set->nodes[index] = malloc(sizeof(simple_set_node));
    __copy(&key, &(set->nodes[index]->_key));
    set->nodes[index]->_hash = hash;
    return SET_TRUE;
}

static void __free_index(SimpleSet *set, uint64_t index) {
    __free(&(set->nodes[index]->_key));
    free(set->nodes[index]);
    set->nodes[index] = NULL;
}

static void __relayout_nodes(SimpleSet *set, uint64_t start, short end_on_null) {
    uint64_t index = 0, i;
    for (i = start; i < set->number_nodes; i++) {
        if(set->nodes[i] != NULL) {
            __get_index(set, set->nodes[i]->_key, set->nodes[i]->_hash, &index);
            if (i != index) { // we are moving this node
                __assign_node(set, set->nodes[i]->_key, set->nodes[i]->_hash, index);
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
