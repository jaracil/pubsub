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


static topic_map_t *Topics = NULL;


int pubsub_subscribe(const char *topic, handle_t *handle){
	topic_map_t *tm;
	handle_list_t *hl;

	HASH_FIND_STR(Topics, topic, tm);
	if (tm==NULL){
		tm = calloc(1, sizeof(*tm));
		tm->topic = strdup(topic);
		HASH_ADD_STR( Topics, topic, tm );
	}
	hl = calloc(1, sizeof(*hl));
	hl->handle = handle;
	DL_APPEND(tm->handles, hl);
	return 0;
}

int pubsub_subscribe_many(char *topics[], handle_t *handle){
	int i;

	if (topics == NULL){
		return 0;
	}
	for(i = 0; topics[i] != NULL; i++) {
		pubsub_subscribe(topics[i], handle);
	}
	return 0;
}

int pubsub_unsubscribe(const char *topic, handle_t *handle){
	topic_map_t *tm;
	handle_list_t *hl;

	HASH_FIND_STR(Topics, topic, tm);
	if (tm==NULL){
		return -1;
	}
	DL_SEARCH_SCALAR(tm->handles, hl, handle, handle);
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

int pubsub_unsubscribe_many(char *topics[], handle_t *handle){
	int i;

	if (topics == NULL){
		return 0;
	}
	for(i = 0; topics[i] != NULL; i++) {
		pubsub_unsubscribe(topics[i], handle);
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
	DL_COUNT(tm->handles, hl, ret);
	return ret;
}

static size_t _pubsub_topic_publish(const char *topic, const msg_t *msg){
	topic_map_t *tm;
	handle_list_t *hl;
	size_t ret = 0;

	HASH_FIND_STR(Topics, topic, tm);
	if (tm==NULL){
		return 0;
	}
	DL_FOREACH(tm->handles, hl) {
		hl->handle->cb(hl->handle, msg);
		ret++;
	}
	return ret;
}

size_t pubsub_publish(const msg_t *msg){
	size_t ret = 0, l;
	char *tp = strdup(msg->topic);

	while(strlen(tp) > 0) {
		ret += _pubsub_topic_publish(tp, msg);
		for(l = strlen(tp); l > 0; l --){
			if (tp[l - 1] == '.'){
				tp[l - 1] = 0;
				break;
			}
			tp[l - 1] = 0;
		}
	}
	ret += _pubsub_topic_publish(".", msg); // Send message to root
	free(tp);
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
