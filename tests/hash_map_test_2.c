
#include "timing.h"
#include "../src/hash_map.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#define KEY_LEN 25

#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KCYN  "\x1B[36m"

typedef uint32_t item;


void success_or_failure(int res) {
    if (res == 1) {
        printf(KGRN "success!\n" KNRM);
    } else {
        printf(KRED "failure!\n" KNRM);
    }
}

void check_string(SimpleSet *set, item key) {
    if (set_contains(set, &key) == SET_TRUE) {
        printf("Set contains [%d]!\n", key);
    } else {
        printf("Set does not contains [%d]!\n", key);
    }
}

static uint64_t item_hash(void *_key, void *_global) {
    use(_global);
    item *key = _key;
    uint32_t n_bytes = sizeof(item);
    uint8_t *bytes = (uint8_t *) key;
    // FNV-1a hash (http://www.isthe.com/chongo/tech/comp/fnv/)
    uint64_t h = 14695981039346656073ULL; // FNV_OFFSET 64 bit
    for (uint32_t i = 0; i < n_bytes; i++) {
        h = h ^ bytes[i];
        h = h * 1099511628211ULL; // FNV_PRIME 64 bit
    }
    return h;
}

static void *item_copy(void *_key, void *_global) {
    use(_global);
    item *key = _key;
    item *copy = malloc(sizeof(item));
    *copy = *key;
    return copy;
}

static void item_free(void *key, void *_global) {
    use(_global);
    free(key);
}

static int item_equals(void *_key_1, void *_key_2, void *_global) {
    use(_global);
    item *key_1 = _key_1;
    item *key_2 = _key_2;
    if (*key_1 != *key_2) {
        return 0;
    }
    return 1;
}

item make_key(int i) {
    return (item) i;
}

void free_key(item key) {
    use(key);
}

void initialize_set(SimpleSet *set, int start, int elements, int itter, int TEST) {
    int i;
    for (i = start; i < elements; i+=itter) {
        item key = make_key(i);
        int res = set_add(set, &key);
        free_key(key);
        if (res != TEST) {
            printf("Error: %d\tres: %d\n", key, res);
        }
    }
}

int main() {
    Timing t;
    timing_start(&t);

    unsigned int added_elements;
    int res, inaccuraces = 0;
    uint64_t i, elements = 50000;
    uint64_t ui;

    SimpleSet A, B, C;
    /* Initialize Set A to 1/2 the elements in A */
    printf("==== Set A Initialization with %" PRIu64 " Elements ====\n", elements);
    set_init(&A, NULL, 1024, item_hash, item_equals, item_copy, item_free);
    initialize_set(&A, 0, elements, 1, SET_TRUE);
    assert(A.used_nodes == elements);

    item** keys = set_to_array(&A, &ui);
    assert(A.used_nodes == ui);  // in multi-thread, don't want to assume it hasn't changed
    printf("==== Set A has %" PRIu64 " Keys ====\n", ui);
    for (i = 0; i < 15; i++) {
        printf("%" PRIu64 "\t%d\n", i, *keys[i]);
    }
    // free the keys memory
    for (i = 0; i < ui; i++) {
        free(keys[i]);
    }
    free(keys);

    /* Initialize Set B to 1/2 the elements in A */
    printf("==== Set B Initialization with %" PRIu64 " Elements ====\n", elements / 2);
    set_init(&B, NULL, 1024, item_hash, item_equals, item_copy, item_free);
    initialize_set(&B, 0, elements / 2, 1, SET_TRUE);
    assert(B.used_nodes == elements / 2);

    /* Test the length of the set */
    printf("==== Set B Length Test ====\n");
    printf("Set length test: ");
    success_or_failure(set_length(&B) == elements / 2);


    /*  Test that inserting the same element into the set does not increase the
        number of nodes. */
    printf("==== Test Double Insertion ===\n");
    added_elements = A.used_nodes;
    initialize_set(&A, 0, elements / 2, 1, SET_ALREADY_PRESENT);
    printf("Double Insetion check: ");
    success_or_failure(added_elements == A.used_nodes);


    /*  Test the contains functions for keys that exist and keys that are not in
        the set */
    printf("\n\n==== Test Set Contains ====\n");
    printf("The symbol âˆˆ indicates set membership and means â€œis an element ofâ€� so that the statement x âˆˆ A means that x is an element of the set A. In other words, x is one of the objects in the collection of (possibly many) objects in the set A.\n");

    printf("Missing keyes check: ");
    for (i = 0; i < elements; i++) {
        item key = make_key(i);
        if (set_contains(&A, &key) != SET_TRUE) {
            printf("Missing Key: [%d]\n", key);
            inaccuraces++;
        }
    }
    success_or_failure(inaccuraces == 0);


    printf("Non-present keys check:\n");
    inaccuraces = 0;
    for (i = elements; i < elements * 2; i++) {
        item key = make_key(i);
        if (set_contains(&A, &key) == SET_TRUE) {
            printf("Non-present key: [%d]\n", key);
            inaccuraces++;
        }
        free_key(key);
    }
    success_or_failure(inaccuraces == 0);

    /* Test removing elements from the set */
    printf("\n\n==== Test Set Remove ====\n");

    printf("Remove keys didn't throw error: ");
    for (i = elements / 2; i < elements; i++) {
        item key = make_key(i);
        if (set_remove(&A, &key) != SET_TRUE) {
            inaccuraces++;
        }
        free_key(key);
    }
    success_or_failure(inaccuraces == 0 && A.used_nodes == elements / 2);

    printf("Remove keys check: ");
    inaccuraces = 0;
    for (i = 0; i < elements; i++) {
        item key = make_key(i);
        if (i >= elements / 2) {
            if (set_contains(&A, &key) == SET_TRUE) {
                printf("Additional Key: [%d]\n", key);
                inaccuraces++;
            }
        } else {
            if (set_contains(&A, &key) != SET_TRUE) {
                printf("Missing Key: [%d]\n", key);
                inaccuraces++;
            }
        }
        free_key(key);
    }
    success_or_failure(inaccuraces == 0);


    /* Test clearing a set and then reset to its original value */
    printf("\n\n==== Reset A ====\n");
    printf("Clear set A: ");
    set_clear(&A);
    inaccuraces = 0;
    for(ui=0; ui < A.number_nodes; ui++) {
        if (A.nodes[ui] != NULL) {
            inaccuraces++;
        }
    }
    success_or_failure(inaccuraces == 0 && A.used_nodes == 0);
    printf("Reset initialize A: ");
    initialize_set(&A, 0, elements, 1, SET_TRUE);
    success_or_failure(A.used_nodes == elements);


    /* Test subset functionality. A is a superset of B. B is a subset of A */
    printf("\n\n==== Test Set Subset ====\n");
    printf("A set A is a subset of another set B if all elements of the set A are elements of the set B. In other words, the set A is contained inside the set B. The subset relationship is denoted as A âŠ† B.\n");
    res = set_is_subset(&A, &B);
    printf("A âŠ† B: %s - ", res == 0 ? "yes" : "no");
    success_or_failure(res == SET_FALSE);
    res = set_is_subset(&B, &A);
    printf("B âŠ† A: %s - ", res == 0 ? "yes" : "no");
    success_or_failure(res == SET_TRUE);
    res = set_is_subset(&A, &A);
    printf("A âŠ† A: %s - ", res == 0 ? "yes" : "no");
    success_or_failure(res == SET_TRUE);
    res = set_is_subset(&B, &B);
    printf("B âŠ† B: %s - ", res == 0 ? "yes" : "no");
    success_or_failure(res == SET_TRUE);

    /*  Test strict subset functionality. A is a superset of B. B is a subset
        of A */
    printf("\n\n==== Test Set Strict Subset ====\n");
    printf("Set A is a strict subset of another set B if all elements of the set A are elements of the set B. In other words, the set A is contained inside the set B. A â‰  B is required. The strict subset relationship is denoted as A âŠ‚ B.\n");
    res = set_is_subset_strict(&A, &B);
    printf("A âŠ‚ B: %s - ", res == 0 ? "yes" : "no");
    success_or_failure(res == SET_FALSE);
    res = set_is_subset_strict(&B, &A);
    printf("B âŠ‚ A: %s - ", res == 0 ? "yes" : "no");
    success_or_failure(res == SET_TRUE);
    res = set_is_subset_strict(&A, &A);
    printf("A âŠ‚ A: %s - ", res == 0 ? "yes" : "no");
    success_or_failure(res == SET_FALSE);
    res = set_is_subset_strict(&B, &B);
    printf("B âŠ‚ B: %s - ", res == 0 ? "yes" : "no");
    success_or_failure(res == SET_FALSE);

    /* Test superset functionality. A is a superset of B. B is a subset of A */
    printf("\n\n==== Test Set Superset ====\n");
    printf("Superset Definition: A set A is a superset of another set B if all elements of the set B are elements of the set A. The superset relationship is denoted as A âŠ‡ B.\n");
    res = set_is_superset(&A, &B);
    printf("A âŠ‡ B: %s - ", res == 0 ? "yes" : "no");
    success_or_failure(res == SET_TRUE);
    res = set_is_superset(&B, &A);
    printf("B âŠ‡ A: %s - ", res == 0 ? "yes" : "no");
    success_or_failure(res == SET_FALSE);
    res = set_is_superset(&A, &A);
    printf("A âŠ‡ A: %s - ", res  == 0 ? "yes" : "no");
    success_or_failure(res == SET_TRUE);
    res = set_is_superset(&B, &B);
    printf("B âŠ‡ B: %s - ", res == 0 ? "yes" : "no");
    success_or_failure(res == SET_TRUE);

    /*  Test strict superset functionality. A is a superset of B. B is a
        subset of A */
    printf("\n\n==== Test Set Strict Superset ====\n");
    printf("Strict Superset Definition: A set A is a superset of another set B if all elements of the set B are elements of the set A. A â‰  B is required. The superset relationship is denoted as A âŠƒ B.\n");
    res = set_is_superset_strict(&A, &B);
    printf("A âŠƒ B: %s - ", res == 0 ? "yes" : "no");
    success_or_failure(res == SET_TRUE);
    res = set_is_superset_strict(&B, &A);
    printf("B âŠƒ A: %s - ", res == 0 ? "yes" : "no");
    success_or_failure(res == SET_FALSE);
    res = set_is_superset_strict(&A, &A);
    printf("A âŠƒ A: %s - ", res  == 0 ? "yes" : "no");
    success_or_failure(res == SET_FALSE);
    res = set_is_superset_strict(&B, &B);
    printf("B âŠƒ B: %s - ", res == 0 ? "yes" : "no");
    success_or_failure(res == SET_FALSE);

    /* Test Set Intersection functionality. */
    printf("\n\n==== Test Set Intersection ====\n");
    printf("The intersection of a set A with a B is the set of elements that are in both set A and B. The intersection is denoted as A âˆ© B.\n");
    set_destroy(&A);
    set_destroy(&B);
    set_init(&A, NULL, 1024, item_hash, item_equals, item_copy, item_free);
    set_init(&B, NULL, 1024, item_hash, item_equals, item_copy, item_free);
    set_init(&C, NULL, 1024, item_hash, item_equals, item_copy, item_free);
    assert(A.used_nodes == 0);
    assert(B.used_nodes == 0);
    assert(C.used_nodes == 0);

    initialize_set(&A, 0, elements, 1, SET_TRUE);
    initialize_set(&B, elements / 2, elements * 2, 1, SET_TRUE);
    set_intersection(&C, &A, &B);

    assert(C.used_nodes == (elements / 2));

    inaccuraces = 0;
    for (i = 0; i < elements / 2;  i++) {
        item key = make_key(i);
        if (set_contains(&C, &key) == SET_TRUE) {
            printf("Non-present key: [%d]\n", key);
            inaccuraces++;
        }
        free_key(key);
    }
    for (i = elements + 1; i < elements * 2;  i++) {
        item key = make_key(i);
        if (set_contains(&C, &key) == SET_TRUE) {
            printf("Non-present key: [%d]\n", key);
            inaccuraces++;
        }
        free_key(key);
    }
    printf("Non-present keys check: ");
    success_or_failure(inaccuraces == 0);

    inaccuraces = 0;
    for (i = elements / 2; i < elements; i++) {
        item key = make_key(i);
        if (set_contains(&C, &key) != SET_TRUE) {
            printf("Missing Key: [%d]\n", key);
            inaccuraces++;
        }
        free_key(key);
    }
    printf("Missing keys check: ");
    success_or_failure(inaccuraces == 0);


    /*  Test Set Difference: A âˆ– B where C contains only those elements from A
        that are not in B. */
    printf("\n\n==== Test Set Difference ====\n");
    printf("The set difference between two sets A and B is written A âˆ– B, and means the set that consists of the elements of A which are not elements of B: x âˆˆ A âˆ– B âŸº x âˆˆ A âˆ§ x âˆ‰ B. Another frequently seen notation for S âˆ– T is S âˆ’ T.\n");
    set_clear(&C);
    set_difference(&C, &A, &B);
    assert(C.used_nodes == (elements / 2));

    inaccuraces = 0;
    for (i = 0; i < elements / 2; i++) {
        item key = make_key(i);
        if (set_contains(&C, &key) != SET_TRUE) {
            printf("Missing Key: [%d]\n", key);
            inaccuraces++;
        }
        free_key(key);
    }
    printf("Missing keys check: ");
    success_or_failure(inaccuraces == 0);

    inaccuraces = 0;
    for (i = elements + 1; i < elements * 2;  i++) {
        item key = make_key(i);
        if (set_contains(&C, &key) == SET_TRUE) {
            printf("Non-present key: [%d]\n", key);
            inaccuraces++;
        }
        free_key(key);
    }
    printf("Non-present keys check: ");
    success_or_failure(inaccuraces == 0);


    /*  Test Symmetric Difference: C = A â–³ B is where C contains only elements
        in A not in B and elements from B that are not in A. */
    printf("\n\n==== Test Set Symmetric Difference ====\n");
    printf("The symmetric difference of two sets A and B is the set of elements either in A or in B but not in both. Symmetric difference is denoted A â–³ B or A * B.\n");
    set_clear(&C);
    set_symmetric_difference(&C, &A, &B);
    assert(C.used_nodes == ((elements * 2) - (elements / 2)));

    printf("Completed the set_symmetric_difference\n");
    inaccuraces = 0;
    for (i = 0; i < elements / 2; i++) {
        item key = make_key(i);
        if (set_contains(&C, &key) != SET_TRUE) {
            printf("Missing Key: [%d]\n", key);
            inaccuraces++;
        }
        free_key(key);
    }
    for (i = elements; i < elements * 2; i++) {
        item key = make_key(i);
        if (set_contains(&C, &key) != SET_TRUE) {
            printf("Missing Key: [%d]\n", key);
            inaccuraces++;
        }
        free_key(key);
    }
    printf("Missing keys check: ");
    success_or_failure(inaccuraces == 0);

    inaccuraces = 0;
    for (i = elements / 2 + 1; i < elements;  i++) {
        item key = make_key(i);
        if (set_contains(&C, &key) == SET_TRUE) {
            printf("Non-present key: [%d]\n", key);
            inaccuraces++;
        }
        free_key(key);
    }
    printf("Non-present keys check: ");
    success_or_failure(inaccuraces == 0);


    /*  Test Set Union: C = A âˆª B denotes that all elements from A and B are
        part of C */
    printf("\n\n==== Test Set Union ====\n");
    printf("The union of a set A with a B is the set of elements that are in either set A or B. The union is denoted as A âˆª B.\n");
    set_clear(&C);
    set_union(&C, &A, &B);
    assert(C.used_nodes == elements * 2);

    for (i = elements; i < elements * 2; i++) {
        item key = make_key(i);
        if (set_contains(&C, &key) != SET_TRUE) {
            printf("Missing Key: [%d]\n", key);
            inaccuraces++;
        }
        free_key(key);
    }
    printf("Missing keys check: ");
    success_or_failure(inaccuraces == 0);

    set_clear(&A);
    set_clear(&B);
    initialize_set(&A, 0, elements, 1, SET_TRUE);
    initialize_set(&B, 1, elements + 1, 1, SET_TRUE);

    printf("\n\n==== Test Set Compare ====\n");
    printf("Sets are different in size Left larger than Right: ");
    res = set_cmp(&C, &A);
    success_or_failure(res == SET_LEFT_GREATER);
    printf("Sets are different in size Right larger than Left: ");
    res = set_cmp(&A, &C);
    success_or_failure(res == SET_RIGHT_GREATER);
    printf("Sets are exactly the same (keys and size): ");
    res = set_cmp(&A, &A);
    success_or_failure(res == SET_EQUAL);
    printf("Sets are the same size but different keys: ");
    res = set_cmp(&A, &B);
    success_or_failure(res == SET_UNEQUAL);

    printf("\n\n==== Clean Up Memory ====\n");
    set_destroy(&A);
    set_destroy(&B);
    set_destroy(&C);
    printf("\n\n==== Completed tests! ====\n");

    timing_end(&t);
    printf("Completed the Set tests in %f seconds!\n", timing_get_difference(t));
    return 0;
}
