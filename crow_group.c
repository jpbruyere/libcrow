//
// crow_group.h
//
// Author:
//       Jean-Philippe Bruyère <jp.bruyere@hotmail.com>
//
// Copyright (c) 2013-2017 Jean-Philippe Bruyère
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
#include <stdlib.h>
#include <stdio.h>

#include "crow_group.h"

void crow_group_reset_content (crow_object_t* go){
	go->largest_child = NULL;
	go->tallest_child = NULL;
	go->content_size = (crow_size_t){0,0};
}

void crow_group_search_largest_child (crow_object_t* go) {
	go->largest_child = NULL;
	go->content_size.width = 0;
	int i;
	for (i = 0; i < go->children_count; i++) {
		if (!go->children [i]->visible)
			continue;
		if (go->children [i]->registered_layoutings & CROW_LAYOUT_WIDTH)
			continue;
		if (go->children [i]->slot.width > go->content_size.width) {
			go->content_size.width = go->children [i]->slot.width;
			go->largest_child = go->children [i];
		}
	}
}

void crow_group_search_tallest_child (crow_object_t* go) {
	go->tallest_child = NULL;
	go->content_size.height = 0;
	int i;
	for (i = 0; i < go->children_count; i++) {
		if (!go->children [i]->visible)
			continue;
		if (go->children [i]->registered_layoutings & CROW_LAYOUT_HEIGHT)
			continue;
		if (go->children [i]->slot.height > go->content_size.height) {
			go->content_size.height = go->children [i]->slot.height;
			go->tallest_child = go->children [i];
		}
	}
}

crow_int_t crow_group_measure (crow_object_t* go, crow_byte_t layoutType) {
	if (go->children_count>0){
		if (layoutType == CROW_LAYOUT_WIDTH) {
			if (!go->largest_child)
				crow_group_search_largest_child (go);
			if (!go->largest_child){
				go->children[0]->width = CROW_MEASURE_FIT;
				return -1;
			}
		}else{
			if (!go->tallest_child)
				crow_group_search_tallest_child (go);
			if (!go->tallest_child){
				go->children[0]->height = CROW_MEASURE_FIT;
				return -1;
			}
		}
	}
    return crow_object_measure (go, layoutType);
}

void crow_group_child_layout_changed (crow_object_t* child, crow_byte_t layoutType) {
	switch (layoutType) {
    case CROW_LAYOUT_WIDTH:
    	if (!IS_FIT(child->parent->width))
    		return;
		if (child->slot.width > child->parent->content_size.width) {
			child->parent->largest_child = child;
			child->parent->content_size.width = child->slot.width;
		} else if (child == child->parent->largest_child)
			 crow_group_search_largest_child (child->parent);
		crow_object_register_layouting (child->parent, CROW_LAYOUT_WIDTH);
		break;    	
    case CROW_LAYOUT_HEIGHT:
    	if (!IS_FIT(child->parent->height))
    		return;
		if (child->slot.height > child->parent->content_size.height) {
			child->parent->tallest_child = child;
			child->parent->content_size.height = child->slot.height;
		} else if (child == child->parent->tallest_child)
			 crow_group_search_tallest_child (child->parent);
		crow_object_register_layouting (child->parent, CROW_LAYOUT_HEIGHT);
		break;
    }
}

void crow_group_layout_changed (crow_object_t* go, crow_byte_t layoutType) {
	crow_object_layout_changed (go, layoutType);

	int i;
	//position smaller objects in group when group size is fit
	switch (layoutType) {
    case CROW_LAYOUT_WIDTH:
		for (i = 0; i < go->children_count; i++) {
			if (go->children[i]->width.units == CROW_UNIT_PERCENT)
				crow_object_register_layouting (go->children[i], CROW_LAYOUT_WIDTH);
			else
				crow_object_register_layouting (go->children[i], CROW_LAYOUT_X);
		}
		break;
    case CROW_LAYOUT_HEIGHT:
		for (i = 0; i < go->children_count; i++) {
			if (go->children[i]->height.units == CROW_UNIT_PERCENT)
				crow_object_register_layouting (go->children[i], CROW_LAYOUT_HEIGHT);
			else
				crow_object_register_layouting (go->children[i], CROW_LAYOUT_Y);
		}
		break;
	}
}

void crow_group_draw (crow_object_t* go, cairo_t* ctx) {
	crow_object_draw (go, ctx);
	LOG(LOG_INFO, LOG_DRAW, "%d: crow_group draw\n", go);
	cairo_save (ctx);

	//clip to client zone
	cairo_rectangle (ctx, 
		crow_object_get_client_x (go),
		crow_object_get_client_y (go),
		crow_object_get_client_width (go),
		crow_object_get_client_height (go));
	cairo_clip (ctx);

	int i;
	for (i = 0; i < go->children_count; i++)
		crow_object_paint (go->children[i], ctx);

	cairo_restore (ctx);
}

void crow_group_update_cache (crow_object_t* go, cairo_t* ctx) {
	cairo_surface_t* cache = cairo_image_surface_create_for_data (go->bmp, CAIRO_FORMAT_ARGB32,
			go->slot.width, go->slot.height, 4 * go->slot.width);
	cairo_t* gr = cairo_create (cache);

	if (!cairo_region_is_empty (go->clipping)){
		int i;
		for (i = 0; i < cairo_region_num_rectangles (go->clipping); i++){
			crow_rectangle_t r;
			cairo_region_get_rectangle (go->clipping, i, &r);
			cairo_rectangle (gr, r.x, r.y, r.width, r.height);
			cairo_clip_preserve (gr);
		}
		cairo_set_operator (gr, CAIRO_OPERATOR_CLEAR);
		cairo_fill (gr);
		cairo_set_operator (gr, CAIRO_OPERATOR_OVER);

		crow_object_draw (go, gr);

		for (i = 0; i < go->children_count; i++) {
			if (!go->children[i]->visible)
				continue;
			crow_rectangle_t r = go->children[i]->slot;
			r.x += crow_object_get_client_x (go->parent);
			r.y += crow_object_get_client_y (go->parent);
			if (cairo_region_contains_rectangle (go->clipping, &r) == CAIRO_REGION_OVERLAP_OUT)
				continue;
			crow_object_paint (go->children[i], gr);
		}
	}

	cairo_destroy (gr);
	cairo_surface_flush (cache);

	cairo_set_source_surface (ctx, cache,
			go->slot.x + crow_object_get_client_x (go->parent), go->slot.y + crow_object_get_client_y (go->parent));
	cairo_paint (ctx);

	cairo_surface_destroy (cache);

	cairo_region_destroy (go->clipping);
	go->clipping = cairo_region_create ();
}