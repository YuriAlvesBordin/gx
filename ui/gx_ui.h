#ifndef GX_UI_H
#define GX_UI_H

#include "../core/gx_gfx.h"

/* Widget type IDs */
typedef enum {
    GX_WT_SHAPE = 0,
    GX_WT_LABEL = 1,
    GX_WT_IMAGE = 2,
} GX_WidgetType;

/**
 * Base node embedded at the start of every widget struct.
 * Treat it as an opaque handle — use the gx_node_* helpers to manipulate it.
 */
typedef struct GX_Node {
    uint8_t          type;
    uint8_t          visible;
    uint8_t          color;
    uint8_t          _pad;
    int16_t          x, y;
    uint16_t         angle;
    uint16_t         _pad2;
    struct GX_Node  *parent;
    struct GX_Node  *first_child;
    struct GX_Node  *next_sibling;
} GX_Node;

typedef struct {
    GX_Node         node;
    const int16_t  *pts;
    uint8_t         numpts;
    uint8_t         roundness;
    bool            fill;
    bool            outline;
} GX_ShapeWidget;

typedef struct {
    GX_Node         node;
    const char     *text;
    const GX_Font  *font;
    uint8_t         scale;
    uint8_t         thickness;
    int8_t          spacing;
    GX_Align        align;
    bool            invert;
} GX_LabelWidget;

typedef struct {
    GX_Node          node;
    const GX_Image  *img;
    uint8_t          scale;
    bool             invert;
} GX_ImageWidget;

/* ---- Lifecycle ---- */
void gx_ui_init(void);
void gx_ui_render_tree(GX_Node *root);
void gx_node_destroy(GX_Node *node);

/* ---- Typed creators ---- */
GX_ShapeWidget *gx_shape_create   (GX_Node *parent, int16_t x, int16_t y, const int16_t *pts, uint8_t numpts);
GX_ShapeWidget *gx_shape_create_ex(GX_Node *parent, int16_t x, int16_t y, const int16_t *pts, uint8_t numpts, uint16_t angle, uint8_t roundness, bool fill, bool outline);
GX_LabelWidget *gx_label_create   (GX_Node *parent, int16_t x, int16_t y, const char *text, const GX_Font *font);
GX_ImageWidget *gx_image_create   (GX_Node *parent, int16_t x, int16_t y, const GX_Image *img);

/* ---- Generic node setters ---- */
void gx_node_set_pos      (GX_Node *n, int16_t x, int16_t y);
void gx_node_set_rotation (GX_Node *n, uint16_t angle);
void gx_node_set_visible  (GX_Node *n, bool visible);
void gx_node_set_color    (GX_Node *n, GX_Color color);

/* ---- Widget-specific setters ---- */
void gx_shape_set_points (GX_ShapeWidget *w, const int16_t *pts, uint8_t numpts);
void gx_shape_set_style  (GX_ShapeWidget *w, bool fill, bool outline, uint8_t roundness);
void gx_label_set_text   (GX_LabelWidget *w, const char *text);
void gx_label_set_style  (GX_LabelWidget *w, uint8_t scale, uint8_t thickness, int8_t spacing, bool invert);
void gx_label_set_align  (GX_LabelWidget *w, GX_Align align);
void gx_image_set_source (GX_ImageWidget *w, const GX_Image *img);
void gx_image_set_style  (GX_ImageWidget *w, uint8_t scale, bool invert);

#endif /* GX_UI_H */
