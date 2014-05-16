#include <stdlib.h>
#include <memory.h>
#include <stdint.h>

#include <proto/zhash.h>

struct zhash
{
	uint32_t size;
	struct zhash_item **htable;
	uint32_t htsize;
	struct zhash_item *list;
	struct zhash_item *free_list;
	uint32_t lsize;
	uint16_t item_size;
	zhash_equal equal_func;
};

enum zhash_errno
{
	ZHASH_INIT_PARAM_LOAD_FACTOR_ERROR = 1, 
	ZHASH_INIT_NO_MEMORY,
};

char *zhash_err_msgs[10] =
{
        [1] = "param loadfactor x shoud be 0 < x <=1",
        [2] = "can't allocate memory"
};

struct zhash_item
{
	struct zhash_item *hnext;
	struct zhash_item *hprev;
	struct zhash_item *qnext;
};

static void  _pool_init(struct zhash *zhash)
{
	struct zhash_item *cur_item;
	struct zhash_item *next_item;
	int i;

	for(i = 0; i < zhash->lsize - 1; i++)
	{
		cur_item = (struct zhash_item *)((uint8_t *)zhash->list + zhash->item_size * i);
		memset((void *)cur_item, 0, zhash->item_size);
		next_item = (struct zhash_item *)((uint8_t *)zhash->list + zhash->item_size * (i + 1));
		cur_item->qnext = next_item;
	}

	cur_item = (struct zhash_item *)((uint8_t *)zhash->list + (zhash->lsize - 1) * zhash->item_size);
	memset((void *)cur_item, 0, zhash->item_size);
	cur_item->qnext = NULL;

	zhash->free_list = zhash->list;
	return;
}

struct zhash *zhash_init(int *errno, uint32_t max_size, float load_factor, uint16_t item_size, zhash_equal equal_func)
{
	struct zhash *zhash;
	struct zhash_item *cur_item;
	struct zhash_item *next_item;
	int i;
	
	if (load_factor <= 0 || load_factor > 1)
	{
		*errno = ZHASH_INIT_PARAM_LOAD_FACTOR_ERROR;
		return NULL;
	}

	zhash = calloc(1, sizeof(struct zhash));
	if (!zhash)
	{
		*errno = ZHASH_INIT_NO_MEMORY;
		return NULL;
	}

	zhash->htsize = (uint32_t)((double) max_size / load_factor);
	zhash->htable = calloc(zhash->htsize, sizeof(struct zhash_item *));
	
	if (!zhash->htable)
	{
		*errno = ZHASH_INIT_NO_MEMORY;
		return NULL;
	}

	zhash->list = malloc(max_size * item_size);
	if (!zhash->list)
	{
		*errno = ZHASH_INIT_NO_MEMORY;
		return NULL;
	}

	zhash->lsize = max_size;
	zhash->item_size = item_size;

	_pool_init(zhash);

	zhash->equal_func = equal_func;

	return zhash;
}

struct zhash_item *zhash_pool_get(struct zhash *zhash)
{
	struct zhash_item *item;

	item = zhash->free_list;
	if (item)
		zhash->free_list = item->qnext;
	return item;
}

void zhash_pool_put(struct zhash *zhash, struct zhash_item *item)
{
	item->qnext = zhash->free_list;
	zhash->free_list = item;
	return;
}

struct zhash_item *zhash_find(struct zhash *zhash, uint64_t hashcode, void *data)
{
	struct zhash_item *item;

	item = zhash->htable[hashcode % zhash->htsize];
	while(item)
	{
		if (zhash->equal_func(item, data))
			return item;
		else
			item = item->hnext;
	}
	return NULL;
}


void zhash_insert(struct zhash *zhash, uint64_t hashcode, struct zhash_item *item)
{
	struct zhash_item *head;
	
	head = zhash->htable[hashcode % zhash->htsize];
	zhash->htable[hashcode % zhash->htsize] = item;
	item->hprev = NULL;
	item->hnext = head;
	if (head)
		head->hprev = item;
	return;
}

struct zhash_item *zhash_delete(struct zhash *zhash, uint64_t hashcode, void *data)
{
	struct zhash_item *item;
	struct zhash_item *head;

	item = zhash_find(zhash, hashcode, data);

	if (!item)
		return NULL;

	head = zhash->htable[hashcode % zhash->htsize];

	if (item == head)
		zhash->htable[hashcode % zhash->htsize] = head->hnext;
	else
		item->hprev->hnext = item->hnext;

	if (item->hnext)
		item->hnext->hprev = item->hprev;

	return 0;
}


void zhash_clear(struct zhash *zhash)
{
	uint32_t i;
	
	_pool_init(zhash);
	zhash->size = 0;
	for (i = 0; i < zhash->htsize; i++)
		zhash->htable[i] = NULL;
	return;
}

int zhash_free(struct zhash *zhash)
{
	free(zhash->htable);
	free(zhash->list);
	free(zhash);
	return 0;
}

char *zhash_err_msg(int errno)
{
        return zhash_err_msgs[errno];
}
