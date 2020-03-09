/*******************************************************************************
***
***     Author: Tyler Barrus
***     email:  barrust@gmail.com
***
***     Version: 0.1.9
***     Purpose: Simple, yet effective, set implementation
***
***     License: MIT 2016
***
***     URL: https://github.com/barrust/set
***
*******************************************************************************/

#ifndef HASH_MAP_H__
#define HASH_MAP_H__

#ifndef use
#define use(x)      do {} while ((x)!=(x))
#endif

#include <inttypes.h>       /* uint64_t */

typedef uint64_t (*key_hash_function) (void *key, void *global);
typedef int (*key_equals_function) (void *key_1, void *key_2, void *global);
typedef void* (*key_copy_function) (void *key, void *global);
typedef void (*key_free_function) (void *key, void *global);

typedef struct  {
    void *_key;
    void *_data;
} SimpleSetNode, simple_set_node;

typedef struct  {
    simple_set_node **nodes;
    void *global;
    uint64_t number_nodes;
    uint64_t used_nodes;
    uint64_t n_collisions;
    key_hash_function hash_function;
    key_equals_function equals_function;
    key_copy_function copy_function;
    key_free_function free_function;
} SimpleSet, simple_set;

/* Initialize the set */
int set_init(SimpleSet *set, void *global, uint64_t init_size,
        key_hash_function hash, key_equals_function equals,
        key_copy_function copy, key_free_function free);

/* Utility function to clear out the set */
int set_clear(SimpleSet *set);

/* Free memory */
int set_destroy(SimpleSet *set);

/*  Add element to set, returns SET_TRUE if added, SET_FALSE if already
    present, SET_ALREADY_PRESENT, or SET_CIRCULAR_ERROR if set is
    completely full */
int set_add(SimpleSet *set, void *key);

/* Add element to set with some data,
    returns SET_TRUE if added, SET_FALSE if already
    present, SET_ALREADY_PRESENT, or SET_CIRCULAR_ERROR if set is
    completely full */
int set_add_with_data(SimpleSet *set, void *key, void *data);

/*  Remove element from the set; Returns SET_TRUE if removed, SET_FALSE if
    not present */
int set_remove(SimpleSet *set, void *key);

/*  Check if key in hash; Returns SET_TRUE if present, SET_FALSE if not
    found, or SET_CIRCULAR_ERROR if set is full and not found */
int set_contains(SimpleSet *set, void *key);

/*  Get data associated with key.  Pass in a pointer to a data object to be
    filled in.  Returns SET_TRUE if element was found, or SET_FALSE if not;
    if not found, data will remain invalid. */
int set_get_data(SimpleSet *set, void *key, void **data);

/* Return the number of elements in the set */
uint64_t set_length(SimpleSet *set);

/*  Set res to the union of s1 and s2
    res = s1 ∪ s2

    The union of a set A with a B is the set of elements that are in either
    set A or B. The union is denoted as A ∪ B */
int set_union(SimpleSet *res, SimpleSet *s1, SimpleSet *s2);

/*  Set res to the intersection of s1 and s2
    res = s1 ∩ s2

    The intersection of a set A with a B is the set of elements that are in
    both set A and B. The intersection is denoted as A ∩ B */
int set_intersection(SimpleSet *res, SimpleSet *s1, SimpleSet *s2);

/*  Set res to the difference between s1 and s2
    res = s1∖ s2

    The set difference between two sets A and B is written A ∖ B, and means
    the set that consists of the elements of A which are not elements
    of B: x ∈ A ∖ B ⟺ x ∈ A ∧ x ∉ B. Another frequently seen notation
    for S ∖ T is S − T. */
int set_difference(SimpleSet *res, SimpleSet *s1, SimpleSet *s2);

/*  Set res to the symmetric difference between s1 and s2
    res = s1 △ s2

    The symmetric difference of two sets A and B is the set of elements either
    in A or in B but not in both. Symmetric difference is denoted
    A △ B or A * B */
int set_symmetric_difference(SimpleSet *res, SimpleSet *s1, SimpleSet *s2);

/*  Return SET_TRUE if test is fully contained in s2; returns SET_FALSE
    otherwise
    test ⊆ against

    A set A is a subset of another set B if all elements of the set A are
    elements of the set B. In other words, the set A is contained inside
    the set B. The subset relationship is denoted as A ⊆ B */
int set_is_subset(SimpleSet *test, SimpleSet *against);

/*  Inverse of subset; return SET_TRUE if set test fully contains
    (including equal to) set against; return SET_FALSE otherwise
    test ⊇ against

    Superset Definition: A set A is a superset of another set B if all
    elements of the set B are elements of the set A. The superset
    relationship is denoted as A ⊇ B */
int set_is_superset(SimpleSet *test, SimpleSet *against);

/*  Strict subset ensures that the test is a subset of against, but that
    the two are also not equal.
    test ⊂ against

    Set A is a strict subset of another set B if all elements of the set A
    are elements of the set B. In other words, the set A is contained inside
    the set B. A ≠ B is required. The strict subset relationship is denoted
    as A ⊂ B */
int set_is_subset_strict(SimpleSet *test, SimpleSet *against);

/*  Strict superset ensures that the test is a superset of against, but that
    the two are also not equal.
    test ⊃ against

    Strict Superset Definition: A set A is a superset of another set B if
    all elements of the set B are elements of the set A. A ≠ B is required.
    The superset relationship is denoted as A ⊃ B */
int set_is_superset_strict(SimpleSet *test, SimpleSet *against);

/*  Return an array of the elements in the set
    NOTE: Up to the caller to free the memory
    NOTE: Return type will be an array of data elements, which depends on
          the type of the data originally provided */
void *set_to_array(SimpleSet *set, uint64_t *size);

/*  Returns based on number elements:
    -1 if left is less than right
    1 if right is less than left
    0 if left is the same size as right and keys match
    2 if size is the same but elements are different */
int set_cmp(SimpleSet *left, SimpleSet *right);

#define SET_TRUE 0
#define SET_FALSE -1
#define SET_MALLOC_ERROR -2
#define SET_CIRCULAR_ERROR -3
#define SET_OCCUPIED_ERROR -4
#define SET_ALREADY_PRESENT 1

#define SET_RIGHT_GREATER -1
#define SET_LEFT_GREATER 1
#define SET_EQUAL 0
#define SET_UNEQUAL 2


#endif /* END _HASH_MAP_H */
