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


int ctx=0;

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
	assert(pubsub_subscribe("a", &ctx, &msg_cb1) ==0 );
	assert(pubsub_subscribe("a.b", &ctx, &msg_cb2) == 0);
	assert(pubsub_subscribe("a.b.c", &ctx, &msg_cb3) == 0);
	assert(pubsub_subscribe("a.b.c", &ctx, &msg_cb3) == -1); // Already registered, must return -1.

	assert(pubsub_publish_str("a", "sota") == 1);
	assert(pubsub_publish_str("a.b", "caballo") == 1);
	assert(pubsub_publish_str("a.b.c", "rey") == 1);
	assert(pubsub_publish_str("a.b.c.d", "comodin") == 0); // Nobody registered at a.b.c.d, 0 subscribers reached.

	assert(ctx == 3);

	assert(pubsub_unsubscribe_all(&ctx) == 3); // 3 unsubscribed
	assert(pubsub_unsubscribe_all(&ctx) == 0); // Nobody unsubscribed
}

int main(int argc, char* argv[]){
	pubsub_test();
	puts("Test passed.");
	return 0;
}
