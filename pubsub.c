/*
 * pubsub.c
 *
 *  Created on: 18 de jul. de 2016
 *      Author: Pepe Aracil (pepe@nayarsystems.com)
 */

#include <string.h>
#include <stdbool.h>
#include <stdarg.h>

#include "uthash.h"
#include "utlist.h"
#include "pubsub.h"

#include <stdio.h>
typedef struct defer_list_s {
	msg_t *msg;
	struct defer_list_s *next;
	struct defer_list_s *prev;
} defer_list_t;

typedef struct handle_list_s {
	msg_callback_t cb;
	void *ctx;
	struct handle_list_s *next;
	struct handle_list_s *prev;
} handle_list_t;

typedef struct topic_map_s {
	char topic[PUBSUB_TOPIC_SIZE];
	handle_list_t *handles;
	UT_hash_handle hh;
} topic_map_t;

static topic_map_t *Topics = NULL;
static defer_list_t *Defers = NULL;

static char *_strdup(const char *src){
	int len = strlen(src) + 1;
	return memcpy(malloc(len), src, len);
}

msg_t *pubsub_msg_clone(const msg_t* msg){
	msg_t *m;

	m = calloc(1, sizeof(*m));
	m->topic = _strdup(msg->topic);
	m->flags = msg->flags;
	if(msg->rtopic != NULL){
		m->rtopic = _strdup(msg->rtopic);
	}
	switch (m->flags & MSG_MSK_TYPE){
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
	free(msg->topic);
	if (msg->rtopic != NULL){
		free(msg->rtopic);
	}
	if ((msg->flags & MSG_MSK_TYPE) == MSG_BUF_TYPE || (msg->flags & MSG_MSK_TYPE) == MSG_STR_TYPE) {
		free(msg->buf);
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

int pubsub_unsubscribe_all(void *ctx){
	topic_map_t *tm, *tm_tmp;
	int count = 0;

	HASH_ITER(hh, Topics, tm, tm_tmp){
		if (pubsub_unsubscribe(tm->topic, ctx) == 0) count ++;
	}
	return count;
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

size_t pubsub_count(const char *topic){
	topic_map_t *tm;
	handle_list_t *hl;
	size_t ret = 0;

	HASH_FIND_STR(Topics, topic, tm);
	if (tm==NULL){
		return ret;
	}
	DL_FOREACH(tm->handles, hl) {
		if (hl->cb != NULL) ret ++;
	}
	return ret;
}

static size_t publish(const msg_t *msg) {
	topic_map_t *tm;
	handle_list_t *hl, *tmp_hl;
	size_t ret = 0;
	HASH_FIND_STR(Topics, msg->topic, tm);
	if (tm != NULL){
		DL_FOREACH_SAFE(tm->handles, hl, tmp_hl) {
			if (hl->cb != NULL){
				hl->cb(hl->ctx, msg);
				ret++;
			}
		}
	}
	if(!(msg->flags & MSG_FL_NONRECURSIVE)) { // Recursive
		char topic[PUBSUB_TOPIC_SIZE];
		strncpy(topic, msg->topic, PUBSUB_TOPIC_SIZE);
		int last = (int) strlen(topic) - 1;
		while(last >= 0){
			while(last >= 0){
				if (topic[last] == '.'){
					topic[last + 1] = '*';
					topic[last + 2] = '\0';
					last --;
					break;
				}
				last --;
			}
			if(last < 0){
				topic[0] = '*';
				topic[1] = '\0';
			}
			HASH_FIND_STR(Topics, topic, tm);
			if (tm != NULL){
				DL_FOREACH_SAFE(tm->handles, hl, tmp_hl) {
					if (hl->cb != NULL){
						hl->cb(hl->ctx, msg);
						ret++;
					}
				}
			}
		}
	}
	return ret;
}

size_t pubsub_pub_msg(msg_t *msg){
	defer_list_t *dl;
	size_t ret = 0;
	if (msg->flags & MSG_FL_INSTANT) {
		ret = publish(msg);
	} else {
		dl = calloc(1, sizeof(*dl));
		dl->msg = pubsub_msg_clone(msg);
		DL_APPEND(Defers, dl);
	}
	return ret;
}

void pubsub_deferred(){
	defer_list_t *dl;
	while (Defers != NULL) {
		dl = Defers;
		publish(dl->msg);
		DL_DELETE(Defers, dl);
		pubsub_msg_free(dl->msg);
		free(dl);
	}
}

size_t pubsub_pub(const char *topic, uint32_t flags, ...){
	msg_t msg = {0};
	va_list vl;

	if (topic == NULL) return 0;
	msg.topic = (char *)topic;
	msg.str = "";
	msg.flags = flags;
	va_start(vl, flags);
	switch (msg.flags & MSG_MSK_TYPE){
	case MSG_INT_TYPE:
		msg.int_val = va_arg(vl, int);
		break;
	case MSG_DBL_TYPE:
		msg.dbl_val = va_arg(vl, double);
		break;
	case MSG_PTR_TYPE:
		msg.ptr_val = va_arg(vl, void *);
		break;
	case MSG_STR_TYPE:
		msg.str = va_arg(vl, char *);
		break;
	case MSG_BUF_TYPE:
		msg.buf = va_arg(vl, void *);
		msg.buf_sz = va_arg(vl, size_t);
		break;
	}
	if (flags & MSG_FL_RESPONSE) {
		msg.rtopic = va_arg(vl, char *);
	}
	va_end(vl);
	return pubsub_pub_msg(&msg);
}

