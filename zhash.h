#ifndef _ZHASH_H
#define _ZHASH_H

#include <stdint.h>

#define DEFINE_HASH_ITEM_TYPE(name, data)  \
struct name {   \
    struct zhash_item *hnext;   \
    struct zhash_item *hprev;   \
    struct zhash_item *qnext;   \
    data    \
};	\

struct zhash;
struct zhash_item;

typedef int (*zhash_equal)(struct zhash_item*, void *);

struct zhash *zhash_init(int *errno, uint32_t max_size, float load_factor, uint16_t item_size, zhash_equal equal_func);
struct zhash_item *zhash_pool_get(struct zhash *zhash);
void zhash_pool_put(struct zhash *zhash, struct zhash_item *item);
struct zhash_item *zhash_find(struct zhash *zhash, uint64_t hashcode, void *data);
void zhash_insert(struct zhash *zhash, uint64_t hashcode, struct zhash_item *item);
struct zhash_item *zhash_delete(struct zhash *zhash, uint64_t hashcode, void *data);
void zhash_clear(struct zhash *zhash);
int zhash_free(struct zhash *zhash);
char *zhash_err_msg(int errno);
#endif
