/*
 * queue_test.c
 *
 *  Created on: 19 de jul. de 2016
 *      Author: pepe
 */
#include <stdlib.h>
#include "pubsub.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>


int ctx = 0;
int ctx2 = 0;
int ctx_resp = 0;


void msg_resp(void *ctx, const msg_t *msg){
	(*(int*)(ctx)) = msg->int_val;
	assert(pubsub_is_int(msg));
}

void msg_cb_ctx2(void *ctx, const msg_t *msg){
	(*(int*)(ctx))++;
}

void msg_cb1(void *ctx, const msg_t *msg){
	(*(int*)(ctx))++;
	assert(pubsub_is_str(msg));
	assert(strcmp(msg->str,"sota") == 0);
	if (msg->rtopic != NULL){
		pubsub_publish_int(msg->rtopic, 10, MSG_FL_INSTANT);
	}
}

void msg_cb2(void *ctx, const msg_t *msg){
	(*(int*)(ctx))++;
	assert(pubsub_is_str(msg));
	assert(strcmp(msg->str,"caballo") == 0);
	if (msg->rtopic != NULL){
		pubsub_publish_int(msg->rtopic, 11, MSG_FL_INSTANT);
	}
}

void msg_cb3(void *ctx, const msg_t *msg){
	(*(int*)(ctx))++;
	assert(pubsub_is_str(msg));
	assert(strcmp(msg->str,"rey") == 0);
	if (msg->rtopic != NULL){
		pubsub_publish_int(msg->rtopic, 12, MSG_FL_INSTANT);
	}
}

void pubsub_test(){
	assert(pubsub_subscribe("resp", &ctx_resp, msg_resp) == 0);
	assert(pubsub_subscribe("a.b.1", &ctx, msg_cb1) == 0);
	assert(pubsub_subscribe("a.b.2", &ctx, msg_cb2) == 0);
	assert(pubsub_subscribe("a.b.3", &ctx, msg_cb3) == 0);
	assert(pubsub_subscribe("a.b.3", &ctx, msg_cb3) ==	-1); // Already registered, must return -1
	assert(pubsub_subscribe("a.*", &ctx2, msg_cb_ctx2) == 0);

	assert(pubsub_publish_str_r("a.b.1", "resp", "sota", MSG_FL_INSTANT) == 2);
	assert(ctx_resp == 10);
	assert(pubsub_publish_str_r("a.b.2", "resp", "caballo", MSG_FL_INSTANT) == 2);
	assert(ctx_resp == 11);
	ctx_resp = -1;
	assert(pubsub_publish_str("a.b.3", "rey", MSG_FL_INSTANT) == 2);
	assert(ctx_resp == -1);
	assert(pubsub_publish_str("a.b.3", "rey", 0) == 0); // Deferred
	assert(pubsub_publish_str("a.b.4", "comodin", MSG_FL_INSTANT) == 1);
	assert(pubsub_publish_str("a.b.4", "comodin", MSG_FL_INSTANT | MSG_FL_NONRECURSIVE) == 0);

	assert(ctx == 3);
	assert(ctx2 == 4);
	pubsub_deferred();
	assert(ctx == 4);
	assert(ctx2 == 5);

	assert(pubsub_unsubscribe("a.b.1", &ctx) == 0);
	assert(pubsub_unsubscribe_all(&ctx) == 2);
	assert(pubsub_unsubscribe_all(&ctx2) == 1);
	assert(pubsub_unsubscribe_all(&ctx_resp) == 1);
}

int main(int argc, char* argv[]){
	pubsub_test();
	puts("Test passed.");
	return 0;
}
