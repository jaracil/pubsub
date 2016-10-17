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

enum msg_flags {
	MSG_FL_INSTANT = 1 << 0,
	MSG_FL_NONRECURSIVE = 1 << 1,
	MSG_FL_RESPONSE = 1 << 2,
	MSG_INT_TYPE = 0x00000100u,
	MSG_DBL_TYPE = 0x00000200u,
	MSG_PTR_TYPE = 0x00000300u,
	MSG_STR_TYPE = 0x00000400u,
	MSG_BUF_TYPE = 0x00000500u,
	MSG_MSK_TYPE = 0x00000F00u,
};

typedef struct {
	char *topic;			// Message topic
	char *rtopic;			// Response topic
	uint32_t flags;
	union {
		double dbl_val;
		int int_val;
		void *ptr_val;
		size_t buf_sz;
	};
	union {
		void  *buf;
		char  *str;
	};
} msg_t;

typedef void (*msg_callback_t)(void *c, const msg_t *m);

int pubsub_subscribe(const char *topic, void *ctx, msg_callback_t cb);
int pubsub_unsubscribe(const char *topic, void *ctx);
int pubsub_unsubscribe_all(void *ctx);
size_t pubsub_pub_msg(msg_t *msg);
size_t pubsub_pub(const char *topic, uint32_t flags, ...);
size_t pubsub_count(const char *topic);
void pubsub_deferred();
msg_t *pubsub_msg_clone(const msg_t* msg);
void pubsub_msg_free(msg_t *msg);

//Helper macros
#define pubsub_publish_int(topic, val, flags) pubsub_pub(topic, MSG_INT_TYPE | (flags), (int)(val))
#define pubsub_publish_dbl(topic, val, flags) pubsub_pub(topic, MSG_DBL_TYPE | (flags), (double)(val))
#define pubsub_publish_ptr(topic, val, flags) pubsub_pub(topic, MSG_PTR_TYPE | (flags), (void *)(val))
#define pubsub_publish_str(topic, val, flags) pubsub_pub(topic, MSG_STR_TYPE | (flags), (char *)(val))
#define pubsub_publish_buf(topic, val, size, flags) pubsub_pub(topic, MSG_BUF_TYPE | (flags), (void *)(val), (size_t) size)

#define pubsub_publish_int_r(topic, rtopic, val, flags) pubsub_pub(topic, MSG_INT_TYPE | MSG_FL_RESPONSE | (flags), (int)(val), (char *)(rtopic))
#define pubsub_publish_dbl_r(topic, rtopic, val, flags) pubsub_pub(topic, MSG_DBL_TYPE | MSG_FL_RESPONSE | (flags), (double)(val), (char *)(rtopic))
#define pubsub_publish_ptr_r(topic, rtopic, val, flags) pubsub_pub(topic, MSG_PTR_TYPE | MSG_FL_RESPONSE | (flags), (void *)(val), (char *)(rtopic))
#define pubsub_publish_str_r(topic, rtopic, val, flags) pubsub_pub(topic, MSG_STR_TYPE | MSG_FL_RESPONSE | (flags), (char *)(val), (char *)(rtopic))
#define pubsub_publish_buf_r(topic, rtopic, val, size, flags) pubsub_pub(topic, MSG_BUF_TYPE | MSG_FL_RESPONSE | (flags), (void *)(val), (size_t) size, (char *)(rtopic))

#define pubsub_respond_int(msg, val, flags) pubsub_pub((msg)->rtopic, MSG_INT_TYPE | (flags), (int)(val))
#define pubsub_respond_dbl(msg, val, flags) pubsub_pub((msg)->rtopic, MSG_DBL_TYPE | (flags), (double)(val))
#define pubsub_respond_ptr(msg, val, flags) pubsub_pub((msg)->rtopic, MSG_PTR_TYPE | (flags), (void *)(val))
#define pubsub_respond_str(msg, val, flags) pubsub_pub((msg)->rtopic, MSG_STR_TYPE | (flags), (char *)(val))
#define pubsub_respond_buf(msg, val, size, flags) pubsub_pub((msg)->rtopic, MSG_BUF_TYPE | (flags), (void *)(val), (size_t) size)

#define pubsub_is_int(msg) (((msg)->flags & MSG_MSK_TYPE) == MSG_INT_TYPE)
#define pubsub_is_dbl(msg) (((msg)->flags & MSG_MSK_TYPE) == MSG_DBL_TYPE)
#define pubsub_is_ptr(msg) (((msg)->flags & MSG_MSK_TYPE) == MSG_PTR_TYPE)
#define pubsub_is_str(msg) (((msg)->flags & MSG_MSK_TYPE) == MSG_STR_TYPE)
#define pubsub_is_buf(msg) (((msg)->flags & MSG_MSK_TYPE) == MSG_BUF_TYPE)
#define pubsub_type(msg) ((msg)->flags & MSG_MSK_TYPE)

#endif /* PUBSUB_H_ */
