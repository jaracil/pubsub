/*
 * pubsub.c
 *
 *  Created on: 18 de jul. de 2016
 *      Author: Pepe Aracil (pepe@nayarsystems.com)
 */

#include <string.h>

#include "uthash.h"
#include "utlist.h"
#include "pubsub.h"


typedef struct handle_list_s {
	msg_callback_t cb;
	void *ctx;
	struct handle_list_s *prev;
	struct handle_list_s *next;
} handle_list_t;

typedef struct topic_map_s {
	char *topic;
	handle_list_t *handles;
	UT_hash_handle hh;
} topic_map_t;

static topic_map_t *Topics = NULL;

static char* _strdup(const char* src){
	size_t len = strlen(src) + 1;
	return memcpy(malloc(len), src, len);
}

static int handle_cmp(handle_list_t *h1, handle_list_t *h2){
	if (h1->ctx < h2->ctx) return -1;
	if (h1->ctx > h2->ctx) return 1;
	if (h1->cb < h2->cb) return -1;
	if (h1->cb > h2->cb) return 1;
	return 0;
}

int pubsub_subscribe(const char *topic, msg_callback_t cb, void *ctx){
	topic_map_t *tm;
	handle_list_t *hl;

	HASH_FIND_STR(Topics, topic, tm);
	if (tm==NULL){
		tm = calloc(1, sizeof(*tm));
		tm->topic = _strdup(topic);
		HASH_ADD_STR( Topics, topic, tm );
	}
	hl = calloc(1, sizeof(*hl));
	hl->cb = cb;
	hl->ctx = ctx;
	DL_APPEND(tm->handles, hl);
	return 0;
}

int pubsub_unsubscribe(const char *topic, msg_callback_t cb, void *ctx){
	topic_map_t *tm;
	handle_list_t *hl, like = {.cb = cb, .ctx = ctx};

	HASH_FIND_STR(Topics, topic, tm);
	if (tm==NULL){
		return -1;
	}
	DL_SEARCH(tm->handles, hl, &like, handle_cmp);
	if (hl == NULL) {
		return -1;
	}
	DL_DELETE(tm->handles, hl);
	free(hl);
	if (tm->handles == NULL) { //Empty list
		HASH_DEL(Topics, tm);
		free(tm->topic);
		free(tm);
	}
	return 0;
}

int pubsub_sub_unsub(int sub, const char *topic, msg_callback_t cb, void *ctx){
	if (sub){
		return pubsub_subscribe(topic, cb, ctx);
	}
	return pubsub_unsubscribe(topic, cb, ctx);
}

size_t pubsub_count(const char *topic){
	topic_map_t *tm;
	handle_list_t *hl;
	size_t ret = 0;

	HASH_FIND_STR(Topics, topic, tm);
	if (tm==NULL){
		return ret;
	}
	DL_COUNT(tm->handles, hl, ret);
	return ret;
}

size_t pubsub_publish(const msg_t *msg){
	topic_map_t *tm;
	handle_list_t *hl;
	size_t ret = 0;

	HASH_FIND_STR(Topics, msg->topic, tm);
	if (tm==NULL){
		return 0;
	}
	DL_FOREACH(tm->handles, hl) {
		if (hl->cb != NULL){
			hl->cb(hl->ctx, msg);
			ret++;
		}
	}
	return ret;
}

size_t pubsub_publish_int(char *topic, int64_t val){
	return pubsub_publish(&((msg_t){.topic=topic, .type=MSG_INT_TYPE, .int_val=val}));
}

size_t pubsub_publish_dbl(char *topic, double val){
	return pubsub_publish(&((msg_t){.topic=topic, .type=MSG_DBL_TYPE, .dbl_val=val}));
}

size_t pubsub_publish_ptr(char *topic, void *val){
	return pubsub_publish(&((msg_t){.topic=topic, .type=MSG_PTR_TYPE, .ptr_val=val}));
}

size_t pubsub_publish_str(char *topic, char *val){
	return pubsub_publish(&((msg_t){.topic=topic, .type=MSG_STR_TYPE, .buf=val }));
}

size_t pubsub_publish_buf(char *topic, void *val, size_t sz){
	return pubsub_publish(&((msg_t){.topic=topic, .type=MSG_BUF_TYPE, .buf_sz=sz, .buf=val }));
}
