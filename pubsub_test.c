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

void msg_cb(void *ctx, const msg_t *msg){
	(*(int*)(ctx))++;
	if (ctx == &ctx1 && ctx1 == 1){
		assert(strcmp(msg->buf,"sota") == 0);
		return;
	}
	if (ctx == &ctx2 && ctx2 == 1){
		assert(strcmp(msg->buf,"caballo") == 0);
		return;
	}
	if (ctx == &ctx3 && ctx3 == 1){
		assert(strcmp(msg->buf,"rey") == 0);
		return;
	}
	if (ctx == &ctx3 && ctx3 == 2){
		assert(strcmp(msg->buf,"as") == 0);
		return;
	}

	printf("Invalid stat ctx1:%d ctx2:%d ctx3:%d\r\n",ctx1, ctx2, ctx3);
	assert(false);
}

void sub_unsub(int sub){
	pubsub_sub_unsub(sub, "a", &msg_cb, &ctx1);
	pubsub_sub_unsub(sub, "a.b", &msg_cb, &ctx2);
	pubsub_sub_unsub(sub, "a.b.c", &msg_cb, &ctx3);
}

void pubsub_test(){

	sub_unsub(1);

	assert(pubsub_publish_str("a", "sota") == 1);
	assert(pubsub_publish_str("a.b", "caballo") == 1);
	assert(pubsub_publish_str("a.b.c", "rey") == 1);
	assert(pubsub_publish_str("a.b.c", "as") == 1);
	assert(pubsub_publish_str("a.b.c.d", "comodin") == 0);

	assert(ctx1 == 1);
	assert(ctx2 == 1);
	assert(ctx3 == 2);

	sub_unsub(0);
}

int main(int argc, char* argv[]){
	pubsub_test();
	puts("Test passed.");
	return 0;
}
