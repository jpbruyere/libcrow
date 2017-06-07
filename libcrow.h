#ifndef LIBCROW_H_INCLUDED
#define LIBCROW_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <cairo.h>

#define true 1
#define false 0

#define CROW_CACHE_SIZE_MAX 2048

typedef uint8_t		crow_bool_t;
typedef uint8_t		crow_byte_t;
typedef int32_t		crow_int_t;

#define LOG_ERR			0x00
#define LOG_INFO		0x01
#define LOG_FULL		0xff

#define LOG_LAYOUT		0x01
#define LOG_DRAW		0x02
#define LOG_CLIP		0x04
#define LOG_ALL			0xff


#ifdef DEBUG
static crow_byte_t log_level	= LOG_FULL;
static crow_byte_t log_type		= LOG_ALL;
#define LOG(level,type,...) (log_level & level)&&(log_type & type) ? fprintf (stdout, __VA_ARGS__):true; 
#else
#define LOG
#endif

typedef struct _crow_array	crow_array_t;

typedef enum {
    CROW_LAYOUT_NONE = 0x00,
    CROW_LAYOUT_X = 0x01,
    CROW_LAYOUT_Y = 0x02,
    CROW_LAYOUT_POSITION = 0x03,
    CROW_LAYOUT_WIDTH = 0x04,
    CROW_LAYOUT_HEIGHT = 0x08,
    CROW_LAYOUT_SIZE = 0x0C,
    CROW_LAYOUT_CHILDS = 0x10,
    CROW_LAYOUT_ALL = 0xFF
} crow_layout_t;

typedef enum {
    CROW_H_ALIGN_CENTER,
    CROW_H_ALIGN_LEFT,
    CROW_H_ALIGN_RIGHT,
} crow_h_align_t;

typedef enum {
    CROW_V_ALIGN_CENTER,
    CROW_V_ALIGN_TOP,
    CROW_V_ALIGN_BOTTOM,
} crow_v_align_t;

typedef enum {
    CROW_UNIT_PIXEL,
    CROW_UNIT_PERCENT,
    CROW_UNIT_INHERIT
} crow_unit_t;

#define CROW_MEASURE_STRETCHED (crow_measure_t){100,CROW_UNIT_PERCENT}
#define CROW_MEASURE_FIT (crow_measure_t){-1,CROW_UNIT_PIXEL}

#define IS_STRETCHED(msr) (msr.value == 100 && msr.units == CROW_UNIT_PERCENT)
#define IS_FIXED(msr) (msr.value > 0 && msr.units == CROW_UNIT_PIXEL)
#define IS_FIT(msr) (msr.value == -1 && msr.units == CROW_UNIT_PIXEL)


typedef enum {
	CROW_TYPE_SIMPLE,
	CROW_TYPE_CONTAINER,
	CROW_TYPE_GROUP,
	CROW_TYPE_STACK
} crow_type_t;

typedef struct {
    crow_int_t width;
    crow_int_t height;
} crow_size_t;

typedef struct {
    crow_int_t x;
    crow_int_t y;
    crow_int_t width;
    crow_int_t height;
} crow_rectangle_t;

typedef struct {
    crow_int_t value;
    crow_byte_t units;
} crow_measure_t;

typedef struct {
	double r;
	double g;
	double b;
	double a;
} crow_color_t;

struct _crow_object;

typedef struct _crow_lqi {
    struct _crow_object* graphicObj;
    crow_layout_t LayoutType;
    crow_int_t LayoutingTries;
    crow_int_t DiscardCount;
    struct _crow_lqi* nextLQI;
} crow_lqi_t;

typedef struct _crow_layout_queue
{
    crow_lqi_t* FirstLQI;
    crow_lqi_t* LastLQI;
} crow_layout_queue_t;



typedef struct _crow_context
{
    crow_layout_queue_t*		MainQ;
    crow_layout_queue_t*		DiscarQ;
    crow_array_t*			clipping_pool;
    struct _crow_object*		root;
} crow_context_t;


typedef crow_int_t	(*func_measure)(struct _crow_object* co, crow_byte_t lt);
typedef crow_bool_t	(*func_update_layout)(struct _crow_object* co, crow_byte_t lt);
typedef void		(*func_layout_changed)(struct _crow_object* co, crow_byte_t lt);
typedef void		(*func_children_layout_constraints)(struct _crow_object* co, crow_byte_t* lt);
typedef void		(*func_paint)(struct _crow_object* co, cairo_t* ctx);

typedef struct _crow_object {
	crow_byte_t			obj_type;
    crow_int_t			managedIdx;
	crow_context_t*		context;
    crow_int_t			children_count;
    struct _crow_object* parent;
    struct _crow_object** children;
    struct _crow_object* largest_child;
    struct _crow_object* tallest_child;
    crow_int_t			left;
    crow_int_t			top;
    crow_measure_t		width;
    crow_measure_t		height;
    crow_int_t			margin;
    crow_size_t			min_size;
    crow_size_t			max_size;
    cairo_pattern_t*		background;
    crow_bool_t			visible;
    crow_bool_t			is_dirty;
    crow_bool_t			in_clipping_pool;
    crow_bool_t			cache_enabled;
    cairo_region_t*		clipping;
    //unsigned char ArrangeChildren;
    crow_byte_t			registered_layoutings;
    crow_size_t			content_size;
    crow_rectangle_t	slot;
    crow_rectangle_t	last_slot;
    crow_rectangle_t	last_painted_slot;
    crow_byte_t			v_align;
    crow_byte_t			h_align;
    crow_byte_t*			bmp;

    func_measure						MeasureRawSize;
	func_update_layout					UpdateLayout;
    func_layout_changed					OnLayoutChanged;
    func_layout_changed					OnChildLayoutChanged;
    func_children_layout_constraints	ChildrenLayoutingConstraints;
    func_paint							UpdateCache;
    func_paint							OnDraw;
} crow_object_t;

void		crow_lqi_enqueue				(crow_layout_queue_t* lq, crow_lqi_t* lqi);

crow_context_t*	crow_context_create				();
void			crow_context_destroy			(crow_context_t* ctx);
void			crow_context_set_root			(crow_context_t* ctx, crow_object_t* root);
void			crow_context_process_clipping	(crow_context_t* ctx);
void			crow_context_process_layouting	(crow_context_t* ctx);
void			crow_context_process_drawing	(crow_context_t* ctx, cairo_t* cairo_ctx);
void			crow_context_resize				(crow_context_t* ctx, int width, int height);


crow_object_t* 	crow_object_create				();
void			crow_object_destroy				(crow_object_t* go);
void			crow_object_set_type			(crow_object_t* go, crow_type_t ct);
void			crow_object_enqueue				(crow_object_t* go, crow_byte_t layoutType);
void			crow_object_register_layouting	(crow_object_t* go, crow_byte_t layoutType);
crow_int_t		crow_object_measure				(crow_object_t* go, crow_byte_t layoutType);
crow_bool_t		crow_object_do_layout			(crow_object_t* go, crow_byte_t layoutType);
void			crow_object_layout_changed		(crow_object_t* go, crow_byte_t layoutType);
void			crow_object_register_clip		(crow_object_t* go, crow_rectangle_t r);
crow_rectangle_t crow_object_get_client_rect(crow_object_t* go);
crow_int_t		crow_object_get_client_x		(crow_object_t* go);
crow_int_t		crow_object_get_client_y		(crow_object_t* go);
crow_int_t		crow_object_get_client_width	(crow_object_t* go);
crow_int_t		crow_object_get_client_height	(crow_object_t* go);

void			crow_object_child_add			(crow_object_t* parent, crow_object_t* child);
void			crow_object_child_remove		(crow_object_t* parent, crow_object_t* child);

void			crow_object_init_cache			(crow_object_t* go);
void			crow_object_update_cache		(crow_object_t* go, cairo_t* ctx);
void			crow_object_draw				(crow_object_t* go, cairo_t* ctx);
void			crow_object_paint				(crow_object_t* go, cairo_t* ctx);
#ifdef  __cplusplus
}
#endif
#endif // LIBCROW_H_INCLUDED
