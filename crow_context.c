#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libcrow.h"
#include "crow_array.h"

#define CROW_MAX_LAYOUT_TRY      3
#define CROW_MAX_LAYOUT_DISCARD  5

crow_context_t* crow_context_create () {
    crow_context_t* ctx = (crow_context_t*)malloc(sizeof(crow_context_t));
    ctx->MainQ = (crow_layout_queue_t*)malloc(sizeof(crow_layout_queue_t));
    *(ctx->MainQ) = (crow_layout_queue_t){0, 0};
    ctx->DiscarQ = 0;

    ctx->clipping_pool = crow_array_create ();

    return ctx;
}
void crow_context_destroy (crow_context_t* ctx) {
    free (ctx->MainQ);
    free (ctx);
}

void crow_lqi_enqueue (crow_layout_queue_t* lq, crow_lqi_t* lqi) {
	if (!lq->FirstLQI){
        lq->FirstLQI = lqi;
        lq->FirstLQI->nextLQI = 0;
    }else
        lq->LastLQI->nextLQI = lqi;
    lq->LastLQI = lqi;
    //printf ("LQI queued: %d firstLQI=%d secondLQI=%d lastLQI=%d\n", lqi, lq->FirstLQI, lq->FirstLQI->nextLQI, lq->LastLQI);
}

crow_lqi_t* crow_lqi_dequeue (crow_layout_queue_t* lq) {
    if (!lq->FirstLQI){
        //printf("queue Emptry: first=%d last=%d", lq->FirstLQI, lq->LastLQI);
        return 0;
    }

    //printf ("LQI dequeue: firstLQI=%d ", lq->FirstLQI);
    //printf ("secondLQI=%d lastLQI=%d\n", lq->FirstLQI->nextLQI, lq->LastLQI);
    crow_lqi_t* tmp = lq->FirstLQI;
    lq->FirstLQI = tmp->nextLQI;
    tmp->nextLQI = 0;
    //printf ("after dequeue: firstLQI=%d lastLQI=%d\n", lq->FirstLQI, lq->LastLQI);
    return tmp;
}

void crow_context_process_clipping (crow_context_t* ctx) {
	int i = 0;
	for (i = 0; i < ctx->clipping_pool->count; i++) {
		crow_object_t* go = (crow_object_t*)ctx->clipping_pool->elements[i];
		go->in_clipping_pool = false;
		//TODO:try remove this
		if (!go->parent)
			continue;
		crow_object_register_clip (go->parent, go->last_painted_slot);
		crow_object_register_clip (go->parent, go->slot);
	}
	crow_array_reset (ctx->clipping_pool);
}

void crow_context_process_layouting (crow_context_t* ctx) {
	printf ("process layouting\n");
    ctx->DiscarQ = (crow_layout_queue_t*)malloc(sizeof(crow_layout_queue_t));
    *(ctx->DiscarQ) = (crow_layout_queue_t){0, 0};

    while (ctx->MainQ->FirstLQI) {
        crow_lqi_t* lqi = crow_lqi_dequeue (ctx->MainQ);
        if (lqi->graphicObj->parent) {
            lqi->LayoutingTries++;
            if (!crow_object_do_layout (lqi->graphicObj, lqi->LayoutType)){
                if (lqi->LayoutingTries < CROW_MAX_LAYOUT_TRY){
                    lqi->graphicObj->registered_layoutings |= lqi->LayoutType;
                    crow_lqi_enqueue(ctx->MainQ, lqi);
                    continue;
                }else if (lqi->DiscardCount < CROW_MAX_LAYOUT_DISCARD) {
					lqi->LayoutingTries = 0;
					lqi->DiscardCount++;
					lqi->graphicObj->registered_layoutings |= lqi->LayoutType;
					crow_lqi_enqueue (ctx->DiscarQ, lqi);
					continue;
                }
            }
            free (lqi);
        }
    }
    free (ctx->MainQ);
    ctx->MainQ = ctx->DiscarQ;
    ctx->DiscarQ = 0;
}