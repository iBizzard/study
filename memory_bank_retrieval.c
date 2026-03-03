/*
 * memory_bank_retrieval.c
 * Memory Bank的检索方式 (Memory Bank Retrieval Methods)
 *
 * This program demonstrates four common approaches to retrieving data
 * from a memory bank:
 *   1. Sequential Search       (顺序检索)
 *   2. Hash-Based Retrieval    (基于哈希的检索)
 *   3. Content-Addressable     (内容寻址检索)
 *   4. Similarity-Based Search (基于相似度的检索)
 *
 * Compile: gcc -o memory_bank_retrieval memory_bank_retrieval.c -lm
 * Run:     ./memory_bank_retrieval
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* ================================================================
 * Common definitions
 * ================================================================ */
#define BANK_SIZE    64   /* number of slots in the memory bank    */
#define KEY_LEN      16   /* max key length (including '\0')       */
#define VAL_LEN      64   /* max value length (including '\0')     */

typedef struct {
    int    key;
    char   value[VAL_LEN];
} Entry;

/* ================================================================
 * 1. Sequential Search (顺序检索)
 *
 * Traverse the memory bank linearly from the first slot to the last.
 * Time complexity: O(n)
 * ================================================================ */
typedef struct {
    Entry  entries[BANK_SIZE];
    int    count;
} SeqBank;

void seq_init(SeqBank *bank)
{
    bank->count = 0;
}

int seq_insert(SeqBank *bank, int key, const char *value)
{
    if (bank->count >= BANK_SIZE)
        return -1;
    bank->entries[bank->count].key = key;
    strncpy(bank->entries[bank->count].value, value, VAL_LEN - 1);
    bank->entries[bank->count].value[VAL_LEN - 1] = '\0';
    bank->count++;
    return 0;
}

/* Return pointer to found entry, or NULL if not found */
const char *seq_search(const SeqBank *bank, int key)
{
    int i;
    for (i = 0; i < bank->count; i++) {
        if (bank->entries[i].key == key)
            return bank->entries[i].value;
    }
    return NULL;
}

void demo_sequential(void)
{
    SeqBank bank;
    const char *result;

    printf("=== 1. Sequential Search (顺序检索) ===\n");
    seq_init(&bank);
    seq_insert(&bank, 10, "alpha");
    seq_insert(&bank, 25, "beta");
    seq_insert(&bank, 37, "gamma");
    seq_insert(&bank, 42, "delta");

    result = seq_search(&bank, 37);
    printf("  Search key=37 -> %s\n", result ? result : "NOT FOUND");

    result = seq_search(&bank, 99);
    printf("  Search key=99 -> %s\n", result ? result : "NOT FOUND");
    printf("\n");
}

/* ================================================================
 * 2. Hash-Based Retrieval (基于哈希的检索)
 *
 * Use a hash function to map keys to bucket indices for O(1) average
 * lookup.  Collisions are handled with linear probing.
 * ================================================================ */
typedef struct {
    Entry  buckets[BANK_SIZE];
    int    occupied[BANK_SIZE]; /* 0 = empty, 1 = used */
} HashBank;

void hash_init(HashBank *bank)
{
    memset(bank->occupied, 0, sizeof(bank->occupied));
}

static unsigned int hash_func(int key)
{
    unsigned int h = (unsigned int)key;
    h = ((h >> 16) ^ h) * 0x45d9f3b;
    h = ((h >> 16) ^ h) * 0x45d9f3b;
    h = (h >> 16) ^ h;
    return h % BANK_SIZE;
}

int hash_insert(HashBank *bank, int key, const char *value)
{
    unsigned int idx = hash_func(key);
    unsigned int start = idx;
    do {
        if (!bank->occupied[idx]) {
            bank->buckets[idx].key = key;
            strncpy(bank->buckets[idx].value, value, VAL_LEN - 1);
            bank->buckets[idx].value[VAL_LEN - 1] = '\0';
            bank->occupied[idx] = 1;
            return 0;
        }
        idx = (idx + 1) % BANK_SIZE;
    } while (idx != start);
    return -1; /* table full */
}

const char *hash_search(const HashBank *bank, int key)
{
    unsigned int idx = hash_func(key);
    unsigned int start = idx;
    do {
        if (!bank->occupied[idx])
            return NULL;
        if (bank->buckets[idx].key == key)
            return bank->buckets[idx].value;
        idx = (idx + 1) % BANK_SIZE;
    } while (idx != start);
    return NULL;
}

void demo_hash(void)
{
    HashBank bank;
    const char *result;

    printf("=== 2. Hash-Based Retrieval (基于哈希的检索) ===\n");
    hash_init(&bank);
    hash_insert(&bank, 10, "alpha");
    hash_insert(&bank, 25, "beta");
    hash_insert(&bank, 37, "gamma");
    hash_insert(&bank, 42, "delta");

    result = hash_search(&bank, 42);
    printf("  Search key=42 -> %s\n", result ? result : "NOT FOUND");

    result = hash_search(&bank, 99);
    printf("  Search key=99 -> %s\n", result ? result : "NOT FOUND");
    printf("\n");
}

/* ================================================================
 * 3. Content-Addressable Memory (内容寻址检索 / CAM)
 *
 * Instead of searching by an explicit key, search by matching the
 * content (value) itself.  All entries whose value matches the query
 * are returned.  This models hardware CAM / ternary CAM behavior.
 * Time complexity: O(n) per query (hardware CAM achieves O(1)).
 * ================================================================ */
typedef struct {
    char   values[BANK_SIZE][VAL_LEN];
    int    tags[BANK_SIZE];   /* associated tag / metadata */
    int    count;
} CAMBank;

void cam_init(CAMBank *bank)
{
    bank->count = 0;
}

int cam_insert(CAMBank *bank, const char *value, int tag)
{
    if (bank->count >= BANK_SIZE)
        return -1;
    strncpy(bank->values[bank->count], value, VAL_LEN - 1);
    bank->values[bank->count][VAL_LEN - 1] = '\0';
    bank->tags[bank->count] = tag;
    bank->count++;
    return 0;
}

/* Search by content; returns the tag of the first match, or -1 */
int cam_search(const CAMBank *bank, const char *query)
{
    int i;
    for (i = 0; i < bank->count; i++) {
        if (strcmp(bank->values[i], query) == 0)
            return bank->tags[i];
    }
    return -1;
}

void demo_cam(void)
{
    int tag;

    printf("=== 3. Content-Addressable Memory (内容寻址检索) ===\n");
    CAMBank bank;
    cam_init(&bank);
    cam_insert(&bank, "192.168.1.0", 1);
    cam_insert(&bank, "10.0.0.0",    2);
    cam_insert(&bank, "172.16.0.0",  3);

    tag = cam_search(&bank, "10.0.0.0");
    printf("  Search content=\"10.0.0.0\" -> tag=%d\n", tag);

    tag = cam_search(&bank, "8.8.8.8");
    printf("  Search content=\"8.8.8.8\"  -> tag=%d (not found)\n", tag);
    printf("\n");
}

/* ================================================================
 * 4. Similarity-Based Search (基于相似度的检索)
 *
 * Each memory entry stores a fixed-size feature vector.  A query
 * vector is compared against every stored vector using Euclidean
 * distance, and the closest match is returned.  This is the basic
 * approach behind nearest-neighbor retrieval in vector databases and
 * neural memory networks.
 * Time complexity: O(n * d) where d is the vector dimension.
 * ================================================================ */
#define VEC_DIM 4

typedef struct {
    double vector[VEC_DIM];
    char   label[VAL_LEN];
} VecEntry;

typedef struct {
    VecEntry entries[BANK_SIZE];
    int      count;
} VecBank;

void vec_init(VecBank *bank)
{
    bank->count = 0;
}

int vec_insert(VecBank *bank, const double *vec, const char *label)
{
    int i;
    if (bank->count >= BANK_SIZE)
        return -1;
    for (i = 0; i < VEC_DIM; i++)
        bank->entries[bank->count].vector[i] = vec[i];
    strncpy(bank->entries[bank->count].label, label, VAL_LEN - 1);
    bank->entries[bank->count].label[VAL_LEN - 1] = '\0';
    bank->count++;
    return 0;
}

static double euclidean_dist(const double *a, const double *b)
{
    double sum = 0.0;
    int i;
    for (i = 0; i < VEC_DIM; i++)
        sum += (a[i] - b[i]) * (a[i] - b[i]);
    return sqrt(sum);
}

/* Return the label of the nearest entry; NULL if bank is empty */
const char *vec_nearest(const VecBank *bank, const double *query,
                        double *out_dist)
{
    int i, best = -1;
    double best_dist = 1e30;

    for (i = 0; i < bank->count; i++) {
        double d = euclidean_dist(bank->entries[i].vector, query);
        if (d < best_dist) {
            best_dist = d;
            best = i;
        }
    }
    if (best < 0)
        return NULL;
    if (out_dist)
        *out_dist = best_dist;
    return bank->entries[best].label;
}

void demo_similarity(void)
{
    VecBank bank;
    const char *label;
    double dist;

    double v1[] = {1.0, 0.0, 0.0, 0.0};
    double v2[] = {0.0, 1.0, 0.0, 0.0};
    double v3[] = {0.0, 0.0, 1.0, 0.0};
    double q[]  = {0.9, 0.1, 0.0, 0.0};

    printf("=== 4. Similarity-Based Search (基于相似度的检索) ===\n");
    vec_init(&bank);
    vec_insert(&bank, v1, "cat");
    vec_insert(&bank, v2, "dog");
    vec_insert(&bank, v3, "bird");

    label = vec_nearest(&bank, q, &dist);
    printf("  Query [0.9, 0.1, 0.0, 0.0]\n");
    printf("  Nearest -> \"%s\" (distance=%.4f)\n", label ? label : "EMPTY", dist);
    printf("\n");
}

/* ================================================================
 * Main — run all four demos
 * ================================================================ */
int main(void)
{
    printf("Memory Bank Retrieval Methods (memory bank的检索方式)\n");
    printf("====================================================\n\n");

    demo_sequential();
    demo_hash();
    demo_cam();
    demo_similarity();

    printf("====================================================\n");
    printf("Summary:\n");
    printf("  1. Sequential    - simple, O(n), good for small banks\n");
    printf("  2. Hash-based    - O(1) average, needs hash function\n");
    printf("  3. CAM           - search by content, hardware O(1)\n");
    printf("  4. Similarity    - nearest-neighbor, for vector data\n");

    return 0;
}
