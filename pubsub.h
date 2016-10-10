/*
 * pubsub.h
 *
 *  Created on: 18 de jul. de 2016
 *      Author: pepe
 */

#ifndef PUBSUB_H_
#define PUBSUB_H_
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>
#include "uthash.h"

#ifndef PUBSUB_TOPIC_SIZE
#define PUBSUB_TOPIC_SIZE 32
#endif

enum msg_types {
	MSG_INT_TYPE,
	MSG_DBL_TYPE,
	MSG_PTR_TYPE,
	MSG_STR_TYPE,
	MSG_BUF_TYPE,
};

enum msg_flags {
	MSG_FL_INSTANT = 1 << 0,
	MSG_FL_NONRECURSIVE = 1 << 1,
};

typedef struct {
	char *topic;			// Message topic
	enum msg_types type;	// Message type
	uint32_t flags;
	union {
		double dbl_val;
		int64_t int_val;
		void *ptr_val;
		size_t buf_sz;
	};
	union {
		const void  *buf;
		const char  *str;
	};
} msg_t;

typedef void (*msg_callback_t)(void *c, const msg_t *m);

int pubsub_subscribe(const char *topic, void *ctx, msg_callback_t cb);
int pubsub_unsubscribe(const char *topic, void *ctx);
int pubsub_unsubscribe_all(void *ctx);
size_t pubsub_publish(msg_t *msg);
size_t pubsub_count(const char *topic);
void pubsub_deferred();

msg_t *pubsub_msg_clone(const msg_t* msg);
void pubsub_msg_free(msg_t *msg);

//Helper functions
size_t pubsub_publish_int(const char *topic, int64_t val, uint32_t flags);
size_t pubsub_publish_dbl(const char *topic, double val, uint32_t flags);
size_t pubsub_publish_ptr(const char *topic, void *val, uint32_t flags);
size_t pubsub_publish_str(const char *topic, const char *val, uint32_t flags);
size_t pubsub_publish_buf(const char *topic, const void *val, size_t sz, uint32_t flags);

#endif /* PUBSUB_H_ */
