#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "libcrow.h"
#include "crow_array.h"
#include "crow_group.h"

crow_object_t* crow_object_create (){
    crow_object_t* g = (crow_object_t*)malloc(sizeof(crow_object_t));
    memset (g, 0, sizeof(crow_object_t));
    g->clipping = cairo_region_create ();

    return g;
}

void crow_object_destroy (crow_object_t* go){
	if (go->background)
		cairo_pattern_destroy (go->background);
	if (go->bmp)
		free (go->bmp);
	if (go->children_count > 0)
		free (go->children);
	cairo_region_destroy (go->clipping);
    free (go);
}

void crow_object_set_type (crow_object_t* go, crow_type_t ct) {
	go->obj_type = ct;
	switch (ct) {
    case CROW_TYPE_SIMPLE:
		go->MeasureRawSize = crow_object_measure;
		go->UpdateLayout = crow_object_do_layout;
	    go->OnLayoutChanged = crow_object_layout_changed;
	    go->OnChildLayoutChanged = NULL;
	    go->ChildrenLayoutingConstraints = NULL;
	    go->OnDraw = crow_object_draw;
	    go->UpdateCache = crow_object_update_cache;
    	break;
    case CROW_TYPE_CONTAINER:
		go->MeasureRawSize = crow_object_measure;
		go->UpdateLayout = crow_object_do_layout;
	    go->OnLayoutChanged = crow_object_layout_changed;
	    go->OnChildLayoutChanged = NULL;
	    go->ChildrenLayoutingConstraints = NULL;
	    go->OnDraw = crow_object_draw;
    	break;
    case CROW_TYPE_GROUP:
		go->MeasureRawSize = crow_group_measure;
		go->UpdateLayout = crow_object_do_layout;
	    go->OnLayoutChanged = crow_group_layout_changed;
	    go->OnChildLayoutChanged = crow_group_child_layout_changed;
	    go->ChildrenLayoutingConstraints = NULL;
	    go->OnDraw = crow_group_draw;
	    go->UpdateCache = crow_group_update_cache;
    	break;
    case CROW_TYPE_STACK:
		go->MeasureRawSize = crow_object_measure;
		go->UpdateLayout = crow_object_do_layout;
	    go->OnLayoutChanged = crow_object_layout_changed;
	    go->OnChildLayoutChanged = NULL;
	    go->ChildrenLayoutingConstraints = NULL;
	    go->OnDraw = crow_group_draw;
    	break;
    }
}

void crow_object_enqueue (crow_object_t* go, crow_byte_t layoutType) {
    crow_lqi_t* lqi = (crow_lqi_t*)malloc(sizeof(crow_lqi_t));
    memset (lqi, 0, sizeof(crow_lqi_t));
    lqi->graphicObj = go;
    lqi->graphicObj->registered_layoutings |= layoutType;
    lqi->LayoutType = layoutType;

    crow_lqi_enqueue (go->context->MainQ, lqi);
}

void crow_object_child_add (crow_object_t* parent, crow_object_t* child) {	
	if (parent->children_count == 0)
		parent->children = (crow_object_t**)malloc(sizeof(crow_object_t*));
	else
		parent->children = (crow_object_t**)realloc(parent->children, sizeof(crow_object_t*) * (parent->children_count+1));
	parent->children[parent->children_count] = child;
	parent->children_count ++;
	child->parent = parent;
}
void crow_object_child_remove (crow_object_t* parent, crow_object_t* child) {

}

void crow_object_register_repaint (crow_object_t* go) {	
	if (go->in_clipping_pool)
		return;
	LOG(LOG_INFO, LOG_CLIP, "%d: register_repaint, put in clipping pool\n", go);
	crow_array_add (go->context->clipping_pool, go);
	go->in_clipping_pool = true;
}



void crow_object_register_layouting (crow_object_t* go, crow_byte_t layoutType) {
	LOG(LOG_INFO, LOG_DRAW, "%d: register_layouting -> %d\n", go, layoutType);
    if (!go->parent)
        return;
    //prevent queueing same LayoutingType for this
    layoutType &= (~go->registered_layoutings);

    if (!layoutType)
        return;
    //dont set position for stretched item
    if (IS_STRETCHED(go->width))
        layoutType &= (~CROW_LAYOUT_X);
    if (IS_STRETCHED(go->height))
        layoutType &= (~CROW_LAYOUT_Y);

    //apply constraints depending on parent type
    if (go->parent->ChildrenLayoutingConstraints)
    	go->parent->ChildrenLayoutingConstraints (go, &layoutType);

    if (!layoutType)
        return;

    //enqueue LQI LayoutingTypes separately
    if (layoutType & CROW_LAYOUT_WIDTH)
        crow_object_enqueue (go, CROW_LAYOUT_WIDTH);
    if (layoutType & CROW_LAYOUT_HEIGHT)
        crow_object_enqueue (go, CROW_LAYOUT_HEIGHT);
    if (layoutType & CROW_LAYOUT_X)
        crow_object_enqueue (go, CROW_LAYOUT_X);
    if (layoutType & CROW_LAYOUT_Y)
        crow_object_enqueue (go, CROW_LAYOUT_Y);
    if (layoutType & CROW_LAYOUT_CHILDS)
        crow_object_enqueue (go, CROW_LAYOUT_CHILDS);
}

crow_int_t crow_object_measure (crow_object_t* go, crow_byte_t layoutType) {
	switch (layoutType) {
    case CROW_LAYOUT_WIDTH:
    	return go->content_size.width + 2 * go->margin;
    case CROW_LAYOUT_HEIGHT:
    	return go->content_size.height + 2 * go->margin;
    }
    return 0;
}

crow_bool_t crow_object_do_layout (crow_object_t* go, crow_byte_t layoutType) {
	LOG(LOG_INFO, LOG_LAYOUT, "%d: layouting -> %d\n", go, layoutType);
    //unset bit, it would be reset if LQI is re-queued
    go->registered_layoutings &= (~layoutType);

    switch (layoutType) {
    case CROW_LAYOUT_X:
        if (go->left == 0) {
            if ((go->parent->registered_layoutings & CROW_LAYOUT_WIDTH) || (go->registered_layoutings & CROW_LAYOUT_WIDTH))
                return 0;
            switch (go->v_align) {
            case CROW_H_ALIGN_LEFT:
                go->slot.x = 0;
                break;
            case CROW_H_ALIGN_RIGHT:
                go->slot.x = go->parent->slot.width - go->parent->margin*2 - go->slot.width;
                break;
            case CROW_H_ALIGN_CENTER:
                go->slot.x = (go->parent->slot.width - go->parent->margin*2) / 2 - go->slot.width / 2;
                break;
            }
        } else
            go->slot.x = go->left;

        if (go->last_slot.x == go->slot.x)
            break;

        go->is_dirty = 1;

        go->OnLayoutChanged (go, layoutType);

        go->last_slot.x = go->slot.x;
        break;
    case CROW_LAYOUT_Y:
        if (go->top == 0) {
            if ((go->parent->registered_layoutings & CROW_LAYOUT_HEIGHT) || (go->registered_layoutings & CROW_LAYOUT_HEIGHT))
                return 0;

            switch (go->v_align) {
            case CROW_V_ALIGN_TOP://this could be processed even if parent height is not known
                go->slot.y = 0;
                break;
            case CROW_V_ALIGN_BOTTOM:
                go->slot.y = go->parent->slot.height - go->parent->margin*2 - go->slot.height;
                break;
            case CROW_V_ALIGN_CENTER:
                go->slot.y = (go->parent->slot.height - go->parent->margin*2) / 2 - go->slot.height / 2;
                break;
            }
        } else
            go->slot.y = go->top;

        if (go->last_slot.y == go->slot.y)
            break;

        go->is_dirty = 1;

        go->OnLayoutChanged (go, layoutType);

        go->last_slot.y = go->slot.y;
        break;
    case CROW_LAYOUT_WIDTH:
        if (go->visible) {
            if (go->width.value > 0 && go->width.units == CROW_UNIT_PIXEL)
                go->slot.width = go->width.value;
            else if (go->width.value < 0 && go->width.units == CROW_UNIT_PIXEL)
                go->slot.width = go->MeasureRawSize (go, CROW_LAYOUT_WIDTH);
            else if (go->parent->registered_layoutings & CROW_LAYOUT_WIDTH)
                return 0;
            else if (IS_STRETCHED(go->width)){
                go->slot.width = go->parent->slot.width - go->parent->margin*2;
            }else
                go->slot.width = (int)round((double)((go->parent->slot.width - go->parent->margin * 2) * go->width.value) / 100.0);

            if (go->slot.width < 0)
                return 0;

            //size constrain
            if (go->slot.width < go->min_size.width) {
                go->slot.width = go->min_size.width;
                //NotifyValueChanged ("WidthPolicy", Measure.Stretched);
            } else if (go->slot.width > go->max_size.width && go->max_size.width > 0) {
                go->slot.width = go->max_size.width;
                //NotifyValueChanged ("WidthPolicy", Measure.Stretched);
            }
        } else
            go->slot.width = 0;

        if (go->last_slot.width == go->slot.width)
            break;

        go->is_dirty = 1;

        go->OnLayoutChanged (go, layoutType);

        go->last_slot.width = go->slot.width;
        break;
    case CROW_LAYOUT_HEIGHT:
        if (go->visible) {
            if (go->height.value > 0 && go->height.units == CROW_UNIT_PIXEL)
                go->slot.height = go->height.value;
            else if (go->height.value < 0 && go->height.units == CROW_UNIT_PIXEL)
                go->slot.height = go->MeasureRawSize (go, CROW_LAYOUT_HEIGHT);
            else if (go->parent->registered_layoutings & CROW_LAYOUT_HEIGHT)
                return false;
            else if (IS_STRETCHED(go->height))
                go->slot.height = go->parent->slot.height - go->parent->margin*2;
            else
            	go->slot.height = (int)round((double)((go->parent->slot.height - go->parent->margin * 2) * go->height.value) / 100.0);                

            if (go->slot.height < 0)
                return 0;

            //size constrain
            if (go->slot.height < go->min_size.height) {
                go->slot.height = go->min_size.height;
                //NotifyValueChanged ("HeightPolicy", Measure.Stretched);
            } else if (go->slot.height > go->max_size.height && go->max_size.height > 0) {
                go->slot.height = go->max_size.height;
                //NotifyValueChanged ("HeightPolicy", Measure.Stretched);
            }
        } else
            go->slot.height = 0;

        if (go->last_slot.height == go->slot.height)
            break;

        go->is_dirty = 1;

        go->OnLayoutChanged (go, layoutType);

        go->last_slot.height = go->slot.height;
        break;
    }

    //if no layouting remains in queue for item, registre for redraw
    if (!go->registered_layoutings && go->is_dirty)
        crow_object_register_repaint (go);

    return 1;
}

void crow_object_layout_changed (crow_object_t* go, crow_byte_t layoutType) {
	LOG(LOG_INFO, LOG_LAYOUT, "%d:\tlayout changed -> %d", go, layoutType);
	switch (layoutType) {
	case CROW_LAYOUT_WIDTH:
		LOG(LOG_INFO, LOG_LAYOUT, ": W: %d -> %d\n", go->last_slot.width, go->slot.width);
		crow_object_register_layouting (go, CROW_LAYOUT_X);
		break;
	case CROW_LAYOUT_HEIGHT:
		LOG(LOG_INFO, LOG_LAYOUT, ": H: %d -> %d\n", go->last_slot.height, go->slot.height);
		crow_object_register_layouting (go, CROW_LAYOUT_Y);
		break;
	default:
		LOG(LOG_INFO, LOG_LAYOUT, "\n", go->last_slot.height, go->slot.height);
		break;
	}
}

crow_rectangle_t crow_object_get_client_rect (crow_object_t* go) {
	return (crow_rectangle_t) {
				go->slot.x + go->margin,
				go->slot.y + go->margin,
				go->slot.width - 2 * go->margin,
				go->slot.height - 2 * go->margin };
}

crow_int_t crow_object_get_client_x (crow_object_t* go) {
	return !go ? 0 : go->slot.x + go->margin;
}
crow_int_t crow_object_get_client_y (crow_object_t* go) {
	return !go ? 0 : go->slot.y + go->margin;
}
crow_int_t crow_object_get_client_width (crow_object_t* go) {
	return !go ? 0 : go->slot.width - go->margin * 2;
}
crow_int_t crow_object_get_client_height (crow_object_t* go) {
	return !go ? 0 : go->slot.height - go->margin * 2;
}
void crow_object_register_clip (crow_object_t* go, crow_rectangle_t c) {
	crow_rectangle_t r = {
		c.x + crow_object_get_client_x (go),
		c.y + crow_object_get_client_y (go),
		c.width, c.height};
	LOG(LOG_INFO, LOG_CLIP, "%d: register_clip (%d, %d, %d, %d)\n", go, r.x, r.y, r.width, r.height);
	cairo_region_union_rectangle (go->clipping, (cairo_rectangle_int_t*)&r);
}

/// <summary> This is the common overridable drawing routine to create new widget </summary>
void crow_object_draw (crow_object_t* go, cairo_t* ctx) {
	LOG(LOG_INFO, LOG_DRAW, "%d: crow_object draw\n", go);
	if (go->background) {		
		cairo_rectangle (ctx, 0, 0, go->slot.width, go->slot.height);
		cairo_set_source (ctx, go->background);
		cairo_fill (ctx);
	}
}

/// <summary>
/// Internal drawing context creation on a cached surface limited to slot size
/// this trigger the effective drawing routine </summary>
void crow_object_init_cache (crow_object_t* go) {
	LOG(LOG_INFO, LOG_DRAW, "%d: init_cache\n", go);
	int stride = 4 * go->slot.width;
	int bmpSize = abs (stride) * go->slot.height;

	if (go->bmp)
		free (go->bmp);
	go->bmp = (crow_byte_t*)malloc(sizeof(crow_byte_t)*bmpSize);
	go->is_dirty = false;

	cairo_surface_t* draw =
		cairo_image_surface_create_for_data (go->bmp, CAIRO_FORMAT_ARGB32, go->slot.width, go->slot.height, stride);
	cairo_t* gr = cairo_create (draw);

	go->OnDraw (go, gr);

	cairo_destroy (gr);
	cairo_surface_flush (draw);
	cairo_surface_destroy (draw);
}

void crow_object_update_cache (crow_object_t* go, cairo_t* ctx) {
	LOG(LOG_INFO, LOG_DRAW, "%d: update_cache\n", go);
	cairo_surface_t* cache = cairo_image_surface_create_for_data (go->bmp, CAIRO_FORMAT_ARGB32,
			go->slot.width, go->slot.height, 4 * go->slot.width);

	cairo_set_source_surface (ctx, cache,
			go->slot.x + crow_object_get_client_x (go->parent), go->slot.y + crow_object_get_client_y (go->parent));
	cairo_paint (ctx);

	cairo_surface_destroy (cache);

	cairo_region_destroy (go->clipping);
	go->clipping = cairo_region_create ();
}

/// <summary> Chained painting routine on the parent context of the actual cached version
/// of the widget </summary>
void crow_object_paint (crow_object_t* go, cairo_t* ctx) {
	//TODO:this test should not be necessary
	if (go->slot.width < 0 || go->slot.height < 0)
		return;
	LOG(LOG_INFO, LOG_DRAW, "%d: paint (is dirty=%d)\n", go, go->is_dirty);
	if (go->cache_enabled) {
		if (go->slot.width > CROW_CACHE_SIZE_MAX || go->slot.height > CROW_CACHE_SIZE_MAX)
			go->cache_enabled = false;
	}

	if (go->cache_enabled) {
		if (go->is_dirty)
			crow_object_init_cache (go);

		crow_object_update_cache (go, ctx);
		//if (!isEnabled)
		//	paintDisabled (ctx, nativeHnd->Slot + Parent.ClientRectangle.Position);
	} else {
		//TODO:this test should not be necessary
		if (!go->parent){
			return;
		}
		
		cairo_save (ctx);
		cairo_translate (ctx,
			go->slot.x + crow_object_get_client_x (go->parent), go->slot.y + crow_object_get_client_y (go->parent));		

		go->OnDraw (go, ctx);

		cairo_restore (ctx);
	}
	go->last_painted_slot = go->slot;
}
/*
void paintDisabled(Context gr, Rectangle rb){
	gr.Operator = Operator.Xor;
	gr.SetSourceRGBA (0.6, 0.6, 0.6, 0.3);
	gr.Rectangle (rb);
	gr.Fill ();
	gr.Operator = Operator.Over;
}*/