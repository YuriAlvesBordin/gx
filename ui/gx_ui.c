#include "gx_ui.h"
#include <string.h>

/* --------------------------------------------------------------------------
 * Memory pools
 * -------------------------------------------------------------------------- */
#if GX_USE_POOL

static GX_ShapeWidget s_shape_pool[GX_POOL_SHAPES];
static GX_LabelWidget s_label_pool[GX_POOL_LABELS];
static GX_ImageWidget s_image_pool[GX_POOL_IMAGES];
static uint32_t       s_shape_mask;
static uint32_t       s_label_mask;
static uint32_t       s_image_mask;

static GX_ShapeWidget *alloc_shape(void)
{
    for (uint8_t i = 0u; i < GX_POOL_SHAPES; i++)
        if (!(s_shape_mask & (1u << i))) {
            s_shape_mask |= (1u << i);
            memset(&s_shape_pool[i], 0, sizeof(GX_ShapeWidget));
            return &s_shape_pool[i];
        }
    return NULL;
}

static GX_LabelWidget *alloc_label(void)
{
    for (uint8_t i = 0u; i < GX_POOL_LABELS; i++)
        if (!(s_label_mask & (1u << i))) {
            s_label_mask |= (1u << i);
            memset(&s_label_pool[i], 0, sizeof(GX_LabelWidget));
            return &s_label_pool[i];
        }
    return NULL;
}

static GX_ImageWidget *alloc_image(void)
{
    for (uint8_t i = 0u; i < GX_POOL_IMAGES; i++)
        if (!(s_image_mask & (1u << i))) {
            s_image_mask |= (1u << i);
            memset(&s_image_pool[i], 0, sizeof(GX_ImageWidget));
            return &s_image_pool[i];
        }
    return NULL;
}

static void free_node(GX_Node *n)
{
    if (!n) return;
    if (n->type == GX_WT_SHAPE) {
        for (uint8_t i = 0u; i < GX_POOL_SHAPES; i++)
            if ((GX_Node *)&s_shape_pool[i] == n) { s_shape_mask &= ~(1u << i); return; }
    } else if (n->type == GX_WT_LABEL) {
        for (uint8_t i = 0u; i < GX_POOL_LABELS; i++)
            if ((GX_Node *)&s_label_pool[i] == n) { s_label_mask &= ~(1u << i); return; }
    } else if (n->type == GX_WT_IMAGE) {
        for (uint8_t i = 0u; i < GX_POOL_IMAGES; i++)
            if ((GX_Node *)&s_image_pool[i] == n) { s_image_mask &= ~(1u << i); return; }
    }
}

#else  /* GX_USE_POOL == 0 */

#include <stdlib.h>
static GX_ShapeWidget *alloc_shape(void) { return (GX_ShapeWidget *)calloc(1, sizeof(GX_ShapeWidget)); }
static GX_LabelWidget *alloc_label(void) { return (GX_LabelWidget *)calloc(1, sizeof(GX_LabelWidget)); }
static GX_ImageWidget *alloc_image(void) { return (GX_ImageWidget *)calloc(1, sizeof(GX_ImageWidget)); }
static void free_node(GX_Node *n) { free(n); }

#endif /* GX_USE_POOL */

/* --------------------------------------------------------------------------
 * Init
 * -------------------------------------------------------------------------- */
void gx_ui_init(void)
{
#if GX_USE_POOL
    s_shape_mask = 0u;
    s_label_mask = 0u;
    s_image_mask = 0u;
    memset(s_shape_pool, 0, sizeof(s_shape_pool));
    memset(s_label_pool, 0, sizeof(s_label_pool));
    memset(s_image_pool, 0, sizeof(s_image_pool));
#endif
}

/* --------------------------------------------------------------------------
 * Tree helpers
 * -------------------------------------------------------------------------- */
static void attach(GX_Node *parent, GX_Node *child)
{
    child->parent = parent;
    if (!parent) return;
    if (!parent->first_child) { parent->first_child = child; return; }
    GX_Node *sib = parent->first_child;
    while (sib->next_sibling) sib = sib->next_sibling;
    sib->next_sibling = child;
}

static void detach(GX_Node *n)
{
    if (!n->parent) return;
    if (n->parent->first_child == n) { n->parent->first_child = n->next_sibling; return; }
    GX_Node *sib = n->parent->first_child;
    while (sib && sib->next_sibling != n) sib = sib->next_sibling;
    if (sib) sib->next_sibling = n->next_sibling;
}

void gx_node_destroy(GX_Node *node)
{
    if (!node) return;
    GX_Node *child = node->first_child;
    while (child) {
        GX_Node *next   = child->next_sibling;
        child->parent   = NULL;
        gx_node_destroy(child);
        child = next;
    }
    detach(node);
    free_node(node);
}

/* --------------------------------------------------------------------------
 * Creators
 * -------------------------------------------------------------------------- */
GX_ShapeWidget *gx_shape_create(GX_Node *parent, int16_t x, int16_t y,
                                  const int16_t *pts, uint8_t numpts)
{
    GX_ShapeWidget *w = alloc_shape();
    if (!w) return NULL;
    w->node.type    = GX_WT_SHAPE;
    w->node.visible = 1u;
    w->node.color   = GX_COLOR_BLACK;
    w->node.x = x; w->node.y = y;
    w->pts    = pts;
    w->numpts = numpts;
    w->fill   = true;
    attach(parent, &w->node);
    return w;
}

GX_ShapeWidget *gx_shape_create_ex(GX_Node *parent, int16_t x, int16_t y,
                                     const int16_t *pts, uint8_t numpts,
                                     uint16_t angle, uint8_t roundness,
                                     bool fill, bool outline)
{
    GX_ShapeWidget *w = gx_shape_create(parent, x, y, pts, numpts);
    if (!w) return NULL;
    w->node.angle = angle % 360u;
    w->roundness  = roundness;
    w->fill       = fill;
    w->outline    = outline;
    return w;
}

GX_LabelWidget *gx_label_create(GX_Node *parent, int16_t x, int16_t y,
                                  const char *text, const GX_Font *font)
{
    GX_LabelWidget *w = alloc_label();
    if (!w) return NULL;
    w->node.type    = GX_WT_LABEL;
    w->node.visible = 1u;
    w->node.color   = GX_COLOR_BLACK;
    w->node.x = x; w->node.y = y;
    w->text   = text;
    w->font   = font;
    w->scale  = GX_SCALE_1X;
    w->spacing = 1;
    w->align  = GX_ALIGN_CENTER;
    attach(parent, &w->node);
    return w;
}

GX_ImageWidget *gx_image_create(GX_Node *parent, int16_t x, int16_t y,
                                  const GX_Image *img)
{
    GX_ImageWidget *w = alloc_image();
    if (!w) return NULL;
    w->node.type    = GX_WT_IMAGE;
    w->node.visible = 1u;
    w->node.color   = GX_COLOR_BLACK;
    w->node.x = x; w->node.y = y;
    w->img    = img;
    w->scale  = GX_SCALE_1X;
    attach(parent, &w->node);
    return w;
}

/* --------------------------------------------------------------------------
 * Generic node setters
 * -------------------------------------------------------------------------- */
void gx_node_set_pos     (GX_Node *n, int16_t x, int16_t y) { if (n) { n->x = x; n->y = y; } }
void gx_node_set_rotation(GX_Node *n, uint16_t angle)        { if (n) n->angle = angle % 360u; }
void gx_node_set_visible (GX_Node *n, bool visible)          { if (n) n->visible = visible ? 1u : 0u; }
void gx_node_set_color   (GX_Node *n, GX_Color color)        { if (n) n->color = (uint8_t)color; }

/* --------------------------------------------------------------------------
 * Widget-specific setters
 * -------------------------------------------------------------------------- */
void gx_shape_set_points(GX_ShapeWidget *w, const int16_t *pts, uint8_t numpts)
    { if (w) { w->pts = pts; w->numpts = numpts; } }
void gx_shape_set_style(GX_ShapeWidget *w, bool fill, bool outline, uint8_t roundness)
    { if (w) { w->fill = fill; w->outline = outline; w->roundness = roundness; } }
void gx_label_set_text(GX_LabelWidget *w, const char *text)
    { if (w) w->text = text; }
void gx_label_set_style(GX_LabelWidget *w, uint8_t scale, uint8_t thickness, int8_t spacing, bool invert)
    { if (w) { w->scale = (scale < GX_SCALE_MIN) ? GX_SCALE_1X : scale; w->thickness = thickness; w->spacing = spacing; w->invert = invert; } }
void gx_label_set_align(GX_LabelWidget *w, GX_Align align)
    { if (w) w->align = (align > GX_ALIGN_RIGHT) ? GX_ALIGN_CENTER : align; }
void gx_image_set_source(GX_ImageWidget *w, const GX_Image *img)
    { if (w) w->img = img; }
void gx_image_set_style(GX_ImageWidget *w, uint8_t scale, bool invert)
    { if (w) { w->scale = (scale < GX_SCALE_MIN) ? GX_SCALE_1X : scale; w->invert = invert; } }

/* --------------------------------------------------------------------------
 * Render
 * -------------------------------------------------------------------------- */
static void render_node(GX_Node *n, int16_t px, int16_t py, uint16_t pa)
{
    if (!n || !n->visible) return;
    int32_t sa = gx_sin(pa), ca = gx_cos(pa);
    int16_t gx = px + (int16_t)(((int32_t)n->x * ca - (int32_t)n->y * sa + 128) >> 8);
    int16_t gy = py + (int16_t)(((int32_t)n->x * sa + (int32_t)n->y * ca + 128) >> 8);
    uint16_t ga = (uint16_t)((pa + n->angle) % 360u);
    switch (n->type) {
    case GX_WT_SHAPE: {
        GX_ShapeWidget *w = (GX_ShapeWidget *)n;
        gx_gfx_draw_polygon(gx, gy, w->pts, w->numpts, ga, w->roundness, w->fill, w->outline, (GX_Color)n->color);
        break;
    }
    case GX_WT_LABEL: {
        GX_LabelWidget *w = (GX_LabelWidget *)n;
        gx_gfx_draw_label(gx, gy, w->text, w->font, ga, w->scale, w->thickness, w->spacing, w->align, w->invert, (GX_Color)n->color);
        break;
    }
    case GX_WT_IMAGE: {
        GX_ImageWidget *w = (GX_ImageWidget *)n;
        gx_gfx_draw_image(gx, gy, w->img, ga, w->scale, w->invert, (GX_Color)n->color);
        break;
    }
    default: break;
    }
    GX_Node *child = n->first_child;
    while (child) { render_node(child, gx, gy, ga); child = child->next_sibling; }
}

void gx_ui_render_tree(GX_Node *root)
{
    if (!root) return;
    gx_gfx_clear();
    render_node(root, 0, 0, 0);
    gx_gfx_flush();
}
