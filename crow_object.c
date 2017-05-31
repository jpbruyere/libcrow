#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "libcrow.h"
#include "crow_array.h"

crow_object_t* crow_object_create (){
    crow_object_t* g = (crow_object_t*)malloc(sizeof(crow_object_t));
    memset (g, 0, sizeof(crow_object_t));
    return g;
}

void crow_object_destroy (crow_object_t* go){
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
    	break;
    case CROW_TYPE_CONTAINER:
		go->MeasureRawSize = crow_object_measure;
		go->UpdateLayout = crow_object_do_layout;
	    go->OnLayoutChanged = crow_object_layout_changed;
	    go->OnChildLayoutChanged = NULL;
	    go->ChildrenLayoutingConstraints = NULL;
    	break;
    case CROW_TYPE_GROUP:
		go->MeasureRawSize = crow_object_measure;
		go->UpdateLayout = crow_object_do_layout;
	    go->OnLayoutChanged = crow_object_layout_changed;
	    go->OnChildLayoutChanged = NULL;
	    go->ChildrenLayoutingConstraints = NULL;
    	break;
    case CROW_TYPE_STACK:
		go->MeasureRawSize = crow_object_measure;
		go->UpdateLayout = crow_object_do_layout;
	    go->OnLayoutChanged = crow_object_layout_changed;
	    go->OnChildLayoutChanged = NULL;
	    go->ChildrenLayoutingConstraints = NULL;
    	break;
    }
}

void crow_object_register_repaint (crow_object_t* go) {
	if (go->in_clipping_pool)
		return;
	crow_array_add (go->context->clipping_pool, go);
	go->in_clipping_pool = true;
}



void crow_object_register_layouting (crow_object_t* go, crow_byte_t layoutType) {
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

void crow_object_enqueue (crow_object_t* go, crow_byte_t layoutType) {
    crow_lqi_t* lqi = (crow_lqi_t*)malloc(sizeof(crow_lqi_t));
    memset (lqi, 0, sizeof(crow_lqi_t));
    lqi->graphicObj = go;
    lqi->graphicObj->registered_layoutings |= layoutType;
    lqi->LayoutType = layoutType;

    crow_lqi_enqueue (go->context->MainQ, lqi);
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
	printf ("doLayout %d\n", go);
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
            if (go->width.Value > 0 && go->width.Units == CROW_UNIT_PIXEL)
                go->slot.width = go->width.Value;
            else if (go->width.Value < 0 && go->width.Units == CROW_UNIT_PIXEL)
                go->slot.width = go->MeasureRawSize (go, CROW_LAYOUT_WIDTH);
            else if (go->parent->registered_layoutings & CROW_LAYOUT_WIDTH)
                return 0;
            else if (IS_STRETCHED(go->width))
                go->slot.width = go->parent->slot.width - go->parent->margin*2;
            else
                go->slot.width = (int)round((double)((go->parent->slot.width - go->parent->margin * 2) * go->width.Value) / 100.0);

            if (go->slot.width < 0)
                return 0;

            //size constrain
            if (go->slot.width < go->MinimumSize.width) {
                go->slot.width = go->MinimumSize.width;
                //NotifyValueChanged ("WidthPolicy", Measure.Stretched);
            } else if (go->slot.width > go->MaximumSize.width && go->MaximumSize.width > 0) {
                go->slot.width = go->MaximumSize.width;
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
            if (go->height.Value > 0 && go->height.Units == CROW_UNIT_PIXEL)
                go->slot.height = go->height.Value;
            else if (go->height.Value < 0 && go->height.Units == CROW_UNIT_PIXEL)
                go->slot.height = go->MeasureRawSize (go, CROW_LAYOUT_HEIGHT);
            else if (go->parent->registered_layoutings & CROW_LAYOUT_HEIGHT)
                return false;
            else if (IS_STRETCHED(go->height))
                go->slot.height = go->parent->slot.height - go->parent->margin*2;
            else
            	go->slot.height = (int)round((double)((go->parent->slot.height - go->parent->margin * 2) * go->height.Value) / 100.0);                

            if (go->slot.height < 0)
                return 0;

            //size constrain
            if (go->slot.height < go->MinimumSize.height) {
                go->slot.height = go->MinimumSize.height;
                //NotifyValueChanged ("HeightPolicy", Measure.Stretched);
            } else if (go->slot.height > go->MaximumSize.height && go->MaximumSize.height > 0) {
                go->slot.height = go->MaximumSize.height;
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
	printf ("LayoutChanged => ");
	switch (layoutType) {
	case CROW_LAYOUT_WIDTH:
		printf ("W => %d: %d -> %d\n", go, go->last_slot.width, go->slot.width);
		crow_object_register_layouting (go, CROW_LAYOUT_X);
		break;
	case CROW_LAYOUT_HEIGHT:
		printf ("H => %d: %d -> %d\n", go, go->last_slot.height, go->slot.height);
		crow_object_register_layouting (go, CROW_LAYOUT_Y);
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
	return go->slot.x + go->margin;
}
crow_int_t crow_object_get_client_y (crow_object_t* go) {
	return go->slot.y + go->margin;
}
crow_int_t crow_object_get_client_width (crow_object_t* go) {
	return go->slot.width - go->margin * 2;
}
crow_int_t crow_object_get_client_height (crow_object_t* go) {
	return go->slot.height - go->margin * 2;
}
void crow_object_register_clip (crow_object_t* go, crow_rectangle_t c) {
	crow_rectangle_t r = {
		c.x + crow_object_get_client_x (go),
		c.y + crow_object_get_client_y (go),
		c.width, c.height};
	cairo_region_union_rectangle (go->clipping, (cairo_rectangle_int_t*)&r);
}

void crow_object_draw (crow_object_t* go, cairo_t* ctx) {
	cairo_rectangle (ctx, 0, 0,
		crow_object_get_client_width (go),
		crow_object_get_client_height (go));
	cairo_set_source_rgba (ctx, 1, 0, 0, 1);
	cairo_fill (ctx);
	DONOTHING
	cairo_fill (ctx);
}