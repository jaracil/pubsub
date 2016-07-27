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


int ctx1=0;
int ctx2=0;
int ctx3=0;

void msg_cb(handle_t *h, const msg_t *msg){
	(*(int*)(h->ctx))++;
	if (h->ctx == &ctx1 && ctx1 == 1){
		assert(strcmp(msg->buf,"sota") == 0);
		return;
	}
	if (h->ctx == &ctx1 && ctx1 == 2){
		assert(strcmp(msg->buf,"caballo") == 0);
		return;
	}
	if (h->ctx == &ctx1 && ctx1 == 3){
		assert(strcmp(msg->buf,"rey") == 0);
		return;
	}
	if (h->ctx == &ctx2 && ctx2 == 1){
		assert(strcmp(msg->buf,"sota") == 0);
		return;
	}
	if (h->ctx == &ctx2 && ctx2 == 2){
		assert(strcmp(msg->buf,"caballo") == 0);
		return;
	}
	if (h->ctx == &ctx3 && ctx3 == 1){
		assert(strcmp(msg->buf,"sota") == 0);
		return;
	}
	printf("Invalid stat ctx1:%d ctx2:%d ctx3:%d\r\n",ctx1, ctx2, ctx3);
	assert(false);
}

void pubsub_test(){
	handle_t h1 = {.cb = &msg_cb, .ctx = &ctx1};
	handle_t h2 = {.cb = &msg_cb, .ctx = &ctx2};
	handle_t h3 = {.cb = &msg_cb, .ctx = &ctx3};

	assert(pubsub_subscribe("a", &h1) == 0);
	assert(pubsub_subscribe("a.b", &h2) == 0);
	assert(pubsub_subscribe("a.b.c", &h3) == 0);

	assert(pubsub_publish_str("a.b.c", "sota") == 3);
	assert(pubsub_publish_str("a.b", "caballo") == 2);
	assert(pubsub_publish_str("a", "rey") == 1);

	assert(ctx1 == 3);
	assert(ctx2 == 2);
	assert(ctx3 == 1);

	assert(pubsub_unsubscribe("a", &h1) == 0);
	assert(pubsub_unsubscribe("a.b", &h2) == 0);
	assert(pubsub_unsubscribe("a.b.c", &h3) == 0);

}

int main(int argc, char* argv[]){
	pubsub_test();
	puts("Test passed.");
	return 0;
}
