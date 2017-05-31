#ifndef LIBCROW_H_INCLUDED
#define LIBCROW_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <cairo.h>

#define true 1
#define false 0

typedef uint8_t		crow_bool_t;
typedef uint8_t		crow_byte_t;
typedef int32_t		crow_int_t;

#define IS_STRETCHED(msr) (msr.Value == 100 && msr.Units == CROW_UNIT_PERCENT)

#ifdef DEBUG
static const crow_byte_t LOG_ERR		0x00;
static const crow_byte_t LOG_INFO		0x01;
static const crow_byte_t LOG_FULL		0xff;
static const crow_byte_t LOG_LAYOUT		0x01;
static const crow_byte_t LOG_DRAW		0x02;
#define LOG(level,logtype,...) fprintf (stdout, __VA_ARGS__)
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
    crow_int_t Value;
    crow_byte_t Units;
} crow_measure_t;

struct _crow_object_t;

typedef struct _crow_lqi {
    struct _crow_object_t* graphicObj;
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
} crow_context_t;


typedef crow_int_t	(*func_measure)(struct _crow_object_t* co, crow_byte_t lt);
typedef crow_bool_t	(*func_update_layout)(struct _crow_object_t* co, crow_byte_t lt);
typedef void		(*func_layout_changed)(struct _crow_object_t* co, crow_byte_t lt);
typedef void		(*func_child_layout_changed)(struct _crow_object_t* co, crow_byte_t lt);
typedef void		(*func_children_layout_constraints)(struct _crow_object_t* co, crow_byte_t* lt);
typedef void		(*func_children_draw)(struct _crow_object_t* co, cairo_t* ctx);

typedef struct _crow_object_t {
	crow_byte_t			obj_type;
	crow_context_t*		context;
    crow_int_t			children_count;
    struct _crow_object_t* parent;
    struct _crow_object_t** children;
    crow_int_t			left;
    crow_int_t			top;
    crow_measure_t		width;
    crow_measure_t		height;
    crow_int_t			margin;
    crow_size_t			MinimumSize;
    crow_size_t			MaximumSize;
    crow_bool_t			visible;
    crow_bool_t			is_dirty;
    crow_bool_t			in_clipping_pool;
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
    func_child_layout_changed			OnChildLayoutChanged;
    func_children_layout_constraints	ChildrenLayoutingConstraints;
    func_children_draw					OnDraw;
} crow_object_t;

void		crow_lqi_enqueue				(crow_layout_queue_t* lq, crow_lqi_t* lqi);

crow_object_t* crow_object_create			();
void		crow_object_destroy				(crow_object_t* go);
void		crow_object_set_type			(crow_object_t* go, crow_type_t ct);
void		crow_object_enqueue				(crow_object_t* go, crow_byte_t layoutType);
void		crow_object_register_layouting	(crow_object_t* go, crow_byte_t layoutType);
crow_int_t	crow_object_measure				(crow_object_t* go, crow_byte_t layoutType);
crow_bool_t	crow_object_do_layout			(crow_object_t* go, crow_byte_t layoutType);
void		crow_object_layout_changed		(crow_object_t* go, crow_byte_t layoutType);
void		crow_object_register_clip		(crow_object_t* go, crow_rectangle_t r);
crow_rectangle_t crow_object_get_client_rect(crow_object_t* go);
crow_int_t	crow_object_get_client_x		(crow_object_t* go);
crow_int_t	crow_object_get_client_y		(crow_object_t* go);
crow_int_t	crow_object_get_client_width	(crow_object_t* go);
crow_int_t	crow_object_get_client_height	(crow_object_t* go);

void		crow_object_draw				(crow_object_t* go, cairo_t* ctx);
#ifdef  __cplusplus
}
#endif
#endif // LIBCROW_H_INCLUDED
