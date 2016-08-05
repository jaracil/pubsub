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

void msg_cb_ctx2(void *ctx, const msg_t *msg){
	(*(int*)(ctx))++;
}

void msg_cb1(void *ctx, const msg_t *msg){
	(*(int*)(ctx))++;
	assert(strcmp(msg->buf,"sota") == 0);
}

void msg_cb2(void *ctx, const msg_t *msg){
	(*(int*)(ctx))++;
	assert(strcmp(msg->buf,"caballo") == 0);
	}

void msg_cb3(void *ctx, const msg_t *msg){
	(*(int*)(ctx))++;
	assert(strcmp(msg->buf,"rey") == 0);
}

void pubsub_test(){
	assert(pubsub_subscribe("a.1", &ctx, &msg_cb1) == 1 );
	assert(pubsub_subscribe("a.2", &ctx, &msg_cb2) == 1);
	assert(pubsub_subscribe("a.3", &ctx, &msg_cb3) == 1);
	assert(pubsub_subscribe("a.3", &ctx, &msg_cb3) ==	0); // Already registered, must return 0 subscriptions

	assert(pubsub_subscribe("a.*", &ctx2, msg_cb_ctx2) == 3); // 3 subscriptions

	assert(pubsub_publish_str("a.1", "sota") == 2);
	assert(pubsub_publish_str("a.2", "caballo") == 2);
	assert(pubsub_publish_str("a.3", "rey") == 2);
	assert(pubsub_publish_str("a.4", "comodin") == 0); // Nobody registered at a.4, 0 subscribers reached.

	assert(ctx == 3);
	assert(ctx2 == 3);

	assert(pubsub_unsubscribe("*", &ctx) == 3); // 3 unsubscribed
	assert(pubsub_unsubscribe("*", &ctx2) == 3); // 3 unsubscribed
	assert(pubsub_unsubscribe("*", &ctx2) == 0); // Nobody topics unsubscribed
}

int main(int argc, char* argv[]){
	pubsub_test();
	puts("Test passed.");
	return 0;
}
