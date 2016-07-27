/*
 * pubsub.h
 *
 *  Created on: 18 de jul. de 2016
 *      Author: pepe
 */

#ifndef PUBSUB_H_
#define PUBSUB_H_
#include <stddef.h>
#include <sys/types.h>
#include "uthash.h"

enum msg_types {
	MSG_INT_TYPE,
	MSG_DBL_TYPE,
	MSG_PTR_TYPE,
	MSG_STR_TYPE,
	MSG_BUF_TYPE,
};

typedef struct {
	char *topic;			// Message topic
	enum msg_types type;	// Message type
	union {
		double dbl_val;
		int64_t int_val;
		void *ptr_val;
		size_t buf_sz;
	};
	void  *buf;
} msg_t;

typedef struct handle_s handle_t;
typedef void (*msg_callback_t)(handle_t *h, const msg_t *m);

typedef struct handle_s {
	msg_callback_t cb;
	void *ctx;
} handle_t;

typedef struct handle_list_s {
	handle_t *handle;
	struct handle_list_s *prev;
	struct handle_list_s *next;
} handle_list_t;

typedef struct topic_map_s {
	char *topic;
	handle_list_t *handles;
	UT_hash_handle hh;
} topic_map_t;

int pubsub_subscribe(const char *topic, handle_t *handle);
int pubsub_subscribe_many(char *topics[], handle_t *handle);
int pubsub_unsubscribe(const char *topic, handle_t *handle);
int pubsub_unsubscribe_many(char *topics[], handle_t *handle);
size_t pubsub_count(const char *topic);
size_t pubsub_publish(const msg_t *msg);

//Helper functions
size_t pubsub_publish_int(char *topic, int64_t val);
size_t pubsub_publish_dbl(char *topic, double val);
size_t pubsub_publish_ptr(char *topic, void *val);
size_t pubsub_publish_str(char *topic, char *val);
size_t pubsub_publish_buf(char *topic, void *val, size_t sz);

#endif /* PUBSUB_H_ */
