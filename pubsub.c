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
	char topic[PUBSUB_TOPIC_SIZE];
	handle_list_t *handles;
	UT_hash_handle hh;
} topic_map_t;

static topic_map_t *Topics = NULL;

static char *_strdup(const char *src){
	int len = strlen(src) + 1;
	return memcpy(malloc(len), src, len);
}

msg_t *pubsub_msg_clone(const msg_t* msg){
	msg_t *m;

	m = malloc(sizeof(*m));
	m->topic = _strdup(msg->topic);
	m->type = msg->type;
	switch (m->type){
	case MSG_INT_TYPE:
		m->int_val = msg->int_val;
		break;
	case MSG_DBL_TYPE:
		m->dbl_val = msg->dbl_val;
		break;
	case MSG_PTR_TYPE:
		m->ptr_val = msg->ptr_val;
		break;
	case MSG_STR_TYPE:
		m->str = _strdup(msg->str);
		break;
	case MSG_BUF_TYPE:
		m->buf_sz = msg->buf_sz;
		m->buf = memcpy(malloc(m->buf_sz), msg->buf, m->buf_sz);
		break;
	}
	return m;
}

void pubsub_msg_free(msg_t *msg){
	free((void *)msg->topic);
	if (msg->type == MSG_BUF_TYPE || msg->type == MSG_STR_TYPE) {
		free((void *)msg->buf);
	}
	free(msg);
}

int pubsub_subscribe(const char *topic, void *ctx, msg_callback_t cb){
	topic_map_t *tm;
	handle_list_t *hl;

	HASH_FIND_STR(Topics, topic, tm);
	if (tm==NULL){
		tm = calloc(1, sizeof(*tm));
		strncpy(tm->topic, topic, PUBSUB_TOPIC_SIZE - 1);
		HASH_ADD_STR( Topics, topic, tm );
	}
	DL_SEARCH_SCALAR(tm->handles, hl, ctx, ctx);
	if (hl != NULL) {
		return -1;
	}
	hl = calloc(1, sizeof(*hl));
	hl->cb = cb;
	hl->ctx = ctx;
	DL_APPEND(tm->handles, hl);
	return 0;
}

int pubsub_unsubscribe(const char *topic, void *ctx){
	topic_map_t *tm;
	handle_list_t *hl;

	HASH_FIND_STR(Topics, topic, tm);
	if (tm==NULL){
		return -1;
	}
	DL_SEARCH_SCALAR(tm->handles, hl, ctx, ctx);
	if (hl == NULL) {
		return -1;
	}
	DL_DELETE(tm->handles, hl);
	free(hl);
	if (tm->handles == NULL) { //Empty list
		HASH_DEL(Topics, tm);
		free(tm);
	}
	return 0;
}

int pubsub_unsubscribe_all(void *ctx){
	topic_map_t *tm, *tmp_tm;
	int ret = 0;

	HASH_ITER(hh, Topics, tm, tmp_tm){
		if (pubsub_unsubscribe(tm->topic, ctx) == 0) {
			ret ++;
		}
	}
	return ret;
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
	handle_list_t *hl, *tmp_hl;
	size_t ret = 0;

	HASH_FIND_STR(Topics, msg->topic, tm);
	if (tm==NULL){
		return 0;
	}
	DL_FOREACH_SAFE(tm->handles, hl, tmp_hl) {
		if (hl->cb != NULL){
			hl->cb(hl->ctx, msg);
			ret++;
		}
	}
	return ret;
}

size_t pubsub_publish_int(const char *topic, int64_t val){
	return pubsub_publish(&((msg_t){.topic=topic, .type=MSG_INT_TYPE, .int_val=val}));
}

size_t pubsub_publish_dbl(const char *topic, double val){
	return pubsub_publish(&((msg_t){.topic=topic, .type=MSG_DBL_TYPE, .dbl_val=val}));
}

size_t pubsub_publish_ptr(const char *topic, void *val){
	return pubsub_publish(&((msg_t){.topic=topic, .type=MSG_PTR_TYPE, .ptr_val=val}));
}

size_t pubsub_publish_str(const char *topic, const char *val){
	return pubsub_publish(&((msg_t){.topic=topic, .type=MSG_STR_TYPE, .buf=val }));
}

size_t pubsub_publish_buf(const char *topic, const void *val, size_t sz){
	return pubsub_publish(&((msg_t){.topic=topic, .type=MSG_BUF_TYPE, .buf_sz=sz, .buf=val }));
}
