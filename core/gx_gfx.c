#include "gx_gfx.h"
#include "../disp/controllers/gx_st7565_priv.h"
#include <string.h>

static const GX_Mcu  *s_port = NULL;
static uint8_t        s_framebuf[GX_BUF_SIZE];
static uint8_t        s_dirty;   /* bitmask of dirty pages (8 pages max) */

/* --------------------------------------------------------------------------
 * Q8 sin/cos  (256 = 1.0)
 * -------------------------------------------------------------------------- */
static const uint8_t s_sinlut[91] = {
    0,4,9,13,18,22,27,31,36,40,44,49,53,58,62,66,71,75,79,83,
    88,92,96,100,104,108,112,116,120,124,128,132,136,139,143,147,
    150,154,158,161,165,168,171,175,178,181,184,187,190,193,196,
    199,202,204,207,210,212,215,217,219,222,224,226,228,230,232,
    234,236,237,239,241,242,243,245,246,247,248,249,250,251,252,
    253,254,254,255,255,255,255,255,255,255
};

int32_t gx_sin(uint16_t deg)
{
    deg %= 360u;
    if (deg == 90u)  return  256;
    if (deg == 270u) return -256;
    if (deg < 90u)   return  (int32_t)s_sinlut[deg];
    if (deg < 180u)  return  (int32_t)s_sinlut[180u - deg];
    if (deg < 270u)  return -(int32_t)s_sinlut[deg - 180u];
    return -(int32_t)s_sinlut[360u - deg];
}

int32_t gx_cos(uint16_t deg)
{
    return gx_sin(deg + 90u);
}

static inline int16_t q8r(int32_t x)
{
    return (int16_t)((x + 128) >> 8);
}

/* --------------------------------------------------------------------------
 * Framebuffer
 * -------------------------------------------------------------------------- */
void gx_gfx_clear(void)
{
    memset(s_framebuf, 0, sizeof(s_framebuf));
    s_dirty = 0xFFu;
}

void gx_gfx_fill(GX_Color color)
{
    memset(s_framebuf, (color == GX_COLOR_BLACK) ? 0xFF : 0, sizeof(s_framebuf));
    s_dirty = 0xFFu;
}

void gx_gfx_draw_pixel(int16_t x, int16_t y, GX_Color color)
{
    if (x < 0 || x >= GX_WIDTH || y < 0 || y >= GX_HEIGHT) return;
    uint16_t idx  = (uint16_t)((uint16_t)(y >> 3) * GX_WIDTH + (uint16_t)x);
    uint8_t  mask = (uint8_t)(1u << (uint8_t)(y & 7u));
    if      (color == GX_COLOR_BLACK)  s_framebuf[idx] |=  mask;
    else if (color == GX_COLOR_WHITE)  s_framebuf[idx] &= (uint8_t)~mask;
    else                               s_framebuf[idx] ^=  mask;
    s_dirty |= (uint8_t)(1u << (uint8_t)(y >> 3));
}

static void hline(int16_t x, int16_t y, int16_t w, GX_Color color)
{
    if (y < 0 || y >= GX_HEIGHT || w <= 0) return;
    if (x < 0)           { w += x; x = 0; }
    if (x + w > GX_WIDTH)  w  = (int16_t)(GX_WIDTH - x);
    if (w <= 0) return;
    uint8_t *row  = &s_framebuf[(uint16_t)(y >> 3) * GX_WIDTH + x];
    uint8_t  mask = (uint8_t)(1u << (uint8_t)(y & 7u));
    if      (color == GX_COLOR_BLACK)  { for (int16_t i = 0; i < w; i++) row[i] |=  mask; }
    else if (color == GX_COLOR_WHITE)  { uint8_t inv = (uint8_t)~mask; for (int16_t i = 0; i < w; i++) row[i] &= inv; }
    else                               { for (int16_t i = 0; i < w; i++) row[i] ^=  mask; }
    s_dirty |= (uint8_t)(1u << (uint8_t)(y >> 3));
}

static void fill_circle(int16_t cx, int16_t cy, int16_t r, GX_Color color)
{
    int16_t r2 = r * r;
    for (int16_t dy = -r; dy <= r; dy++) {
        int16_t dxmax = 0;
        for (int16_t dx = r; dx >= 0; dx--)
            if (dx*dx + dy*dy <= r2) { dxmax = dx; break; }
        hline(cx - dxmax, cy + dy, 2*dxmax + 1, color);
    }
}

static void thick_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
                       uint8_t thickness, GX_Color color)
{
    int16_t dx =  (x1 > x0) ? x1 - x0 : x0 - x1;
    int16_t sx =  (x0 < x1) ? 1 : -1;
    int16_t dy = -((y1 > y0) ? y1 - y0 : y0 - y1);
    int16_t sy =  (y0 < y1) ? 1 : -1;
    int16_t err = dx + dy;
    for (;;) {
        if (thickness <= 1u) gx_gfx_draw_pixel(x0, y0, color);
        else fill_circle(x0, y0, (int16_t)(thickness >> 1), color);
        if (x0 == x1 && y0 == y1) break;
        int16_t e2 (int16_t)(2 * err);
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

/* --------------------------------------------------------------------------
 * Polygon
 * -------------------------------------------------------------------------- */
void gx_gfx_draw_polygon(int16_t cx, int16_t cy,
                          const int16_t *pts, uint8_t numpts,
                          uint16_t angle, uint8_t roundness,
                          bool fill, bool outline, GX_Color color)
{
    if (!pts || numpts == 0u) return;
    int32_t sa = gx_sin(angle);
    int32_t ca = gx_cos(angle);
    uint8_t n  = (numpts < GX_MAX_POLY_PTS) ? numpts : GX_MAX_POLY_PTS;
    int16_t rx[GX_MAX_POLY_PTS], ry[GX_MAX_POLY_PTS];
    for (uint8_t i = 0u; i < n; i++) {
        int32_t px = pts[i*2u], py = pts[i*2u+1u];
        rx[i] = cx + q8r(px*ca - py*sa);
        ry[i] = cy + q8r(px*sa + py*ca);
    }
    if (n == 1u) {
        if (fill) fill_circle(rx[0], ry[0], (int16_t)roundness, color);
        else if (outline) thick_line(rx[0], (int16_t)(ry[0]-roundness),
                                     rx[0], (int16_t)(ry[0]+roundness),
                                     (uint8_t)(roundness>>1)*2u, color);
        return;
    }
    if (n == 2u) { thick_line(rx[0],ry[0],rx[1],ry[1],roundness,color); return; }

    if (fill) {
        int16_t miny = ry[0], maxy = ry[0];
        for (uint8_t i = 1u; i < n; i++) {
            if (ry[i] < miny) miny = ry[i];
            if (ry[i] > maxy) maxy = ry[i];
        }
        for (int16_t y = miny; y <= maxy; y++) {
            int16_t nodes[GX_MAX_POLY_PTS]; uint8_t nc = 0u;
            for (uint8_t i = 0u, j = (uint8_t)(n-1u); i < n; j = i++) {
                if ((ry[i] < y) == (ry[j] >= y) || (ry[j] < y) == (ry[i] >= y))
                    nodes[nc++] = (int16_t)(rx[i] + (int32_t)(y-ry[i])*(rx[j]-rx[i])/(ry[j]-ry[i]));
            }
            for (uint8_t i = 0u; i+1u < nc; i++)
                for (uint8_t k = i+1u; k < nc; k++)
                    if (nodes[i] > nodes[k]) { int16_t t=nodes[i]; nodes[i]=nodes[k]; nodes[k]=t; }
            for (uint8_t i = 0u; i+1u < nc; i += 2u)
                hline(nodes[i], y, (int16_t)(nodes[i+1u]-nodes[i]+1), color);
        }
    }
    if (outline || !fill) {
        uint8_t thic = (roundness > 0u) ? roundness : 1u;
        for (uint8_t i = 0u; i < n; i++) {
            uint8_t next = (uint8_t)((i+1u) % n);
            thick_line(rx[i],ry[i],rx[next],ry[next],thic,color);
        }
    }
}

/* --------------------------------------------------------------------------
 * Label (UTF-8)
 * -------------------------------------------------------------------------- */
static uint16_t utf8_next(const char **p)
{
    const uint8_t *s = (const uint8_t *)*p;
    uint8_t b0 = s[0];
    if (!b0) return 0u;
    if (b0 < 0x80u) { *p += 1; return (uint16_t)b0; }
    if (b0 < 0xE0u) { (*p) += 2; return (uint16_t)(((uint16_t)(b0 & 0x1Fu) << 6) | (s[1] & 0x3Fu)); }
    if (b0 < 0xF0u) { (*p) += 3; return (uint16_t)(((uint16_t)(b0 & 0x0Fu) << 12) | ((uint16_t)(s[1] & 0x3Fu) << 6) | (s[2] & 0x3Fu)); }
    (*p) += 1; return (uint16_t)'?';
}

static uint16_t compose(uint16_t base, uint16_t mark)
{
    switch (mark) {
    case 0x0301u: switch(base){ case 'a':return 0x00E1u; case 'e':return 0x00E9u; case 'i':return 0x00EDu; case 'o':return 0x00F3u; case 'u':return 0x00FAu; case 'A':return 0x00C1u; case 'E':return 0x00C9u; case 'I':return 0x00CDu; case 'O':return 0x00D3u; case 'U':return 0x00DAu; } break;
    case 0x0300u: switch(base){ case 'a':return 0x00E0u; case 'A':return 0x00C0u; } break;
    case 0x0302u: switch(base){ case 'a':return 0x00E2u; case 'e':return 0x00EAu; case 'o':return 0x00F4u; case 'A':return 0x00C2u; case 'E':return 0x00CAu; case 'O':return 0x00D4u; } break;
    case 0x0303u: switch(base){ case 'a':return 0x00E3u; case 'o':return 0x00F5u; case 'A':return 0x00C3u; case 'O':return 0x00D5u; } break;
    case 0x0327u: switch(base){ case 'c':return 0x00E7u; case 'C':return 0x00C7u; } break;
    }
    return 0u;
}

static uint16_t to_codepoints(const char *text, uint16_t *out, uint16_t max)
{
    uint16_t n = 0u;
    const char *p = text;
    while (*p && n < max) {
        uint16_t cp = utf8_next(&p);
        if (!cp) break;
        if (cp < 0x0300u || cp > 0x036Fu) { out[n++] = cp; continue; }
        if (n > 0u) {
            uint16_t c = compose(out[n-1u], cp);
            if (c) { out[n-1u] = c; continue; }
        }
        out[n++] = cp;
    }
    return n;
}

static bool font_glyph(const GX_Font *font, uint16_t cp,
                       const uint8_t **g, uint8_t *w, uint8_t *pg)
{
    if (!font || !g || !w || !pg) return false;
    if (font == &gx_font_5x7) return gx_font_5x7_get_glyph(cp, g, w, pg);
    if (cp > 0x7Fu) return false;
    uint8_t c   = (uint8_t)cp;
    uint8_t f   = (uint8_t)(uint8_t)font->first_char;
    uint8_t cnt = font->char_count;
    if (c < f || c >= (uint8_t)(f + cnt)) return false;
    *pg = (uint8_t)((font->height + 7u) >> 3);
    *w  = font->width;
    *g  = font->data + (uint16_t)(c - f) * (uint16_t)(font->width * (*pg));
    return true;
}

static bool glyph_bounds(const GX_Font *font, uint16_t cp,
                         uint8_t *left, uint8_t *right)
{
    const uint8_t *g; uint8_t w, pg;
    if (!font_glyph(font, cp, &g, &w, &pg)) return false;
    uint8_t l = w, r = 0u; bool found = false;
    for (uint8_t col = 0u; col < w; col++)
        for (uint8_t p = 0u; p < pg; p++)
            if (g[(uint16_t)col * pg + p]) {
                found = true;
                if (col < l) l = col;
                if (col > r) r = col;
                break;
            }
    if (!found) return false;
    *left = l; *right = r; return true;
}

static int16_t glyph_adv(const GX_Font *font, uint16_t cp, int8_t spacing)
{
    int16_t gap = (spacing < 0) ? 0 : (int16_t)spacing;
    if (cp == (uint16_t)' ') return (int16_t)font->width + gap;
    uint8_t l, r;
    if (!glyph_bounds(font, cp, &l, &r)) return (int16_t)font->width + gap;
    return (int16_t)(r - l + 1u) + gap;
}

static int16_t line_adv(const uint16_t *line, uint16_t len,
                        const GX_Font *font, int8_t spacing)
{
    if (!len) return 1;
    int16_t total = 0, gap = (spacing < 0) ? 0 : (int16_t)spacing;
    for (uint16_t i = 0u; i < len; i++) total += glyph_adv(font, line[i], spacing);
    total -= gap;
    return (total < 1) ? 1 : total;
}

static bool font_px(const uint16_t *text, uint16_t textlen,
                    const GX_Font *font,
                    int16_t gcol, int16_t grow,
                    int8_t spacing, int16_t lineh, int16_t linesp)
{
    if (grow < 0) return false;
    int16_t lth = lineh + linesp;
    if (lth <= 0) return false;
    int16_t li  = grow / lth;
    int16_t ril = grow % lth;
    if (ril >= lineh) return false;
    uint16_t ls = 0u;
    for (int16_t i = 0; i < li; i++) {
        while (ls < textlen && text[ls] != (uint16_t)'\n') ls++;
        if (ls < textlen) ls++;
    }
    if (ls >= textlen) return false;
    uint16_t ll = 0u;
    while (ls + ll < textlen && text[ls + ll] != (uint16_t)'\n') ll++;
    if (!ll) return false;
    int16_t cursor = 0;
    for (uint16_t i = 0u; i < ll; i++) {
        uint16_t cp  = text[ls + i];
        int16_t  adv = glyph_adv(font, cp, spacing);
        if (gcol < cursor || gcol >= cursor + adv) { cursor += adv; continue; }
        uint8_t lb, rb;
        if (!glyph_bounds(font, cp, &lb, &rb)) return false;
        int16_t lc = gcol - cursor;
        if (lc < 0 || lc >= (int16_t)(rb - lb + 1)) return false;
        int16_t gc = (int16_t)lb + lc;
        if (ril < 0 || ril >= (int16_t)font->height) return false;
        const uint8_t *g; uint8_t gw, pg;
        if (!font_glyph(font, cp, &g, &gw, &pg)) return false;
        if (gc < 0 || gc >= (int16_t)gw) return false;
        return (g[(uint16_t)gc * pg + (uint8_t)(ril >> 3)] >> (uint8_t)(ril & 7u)) & 1u;
    }
    return false;
}

void gx_gfx_draw_label(int16_t cx, int16_t cy,
                        const char *text, const GX_Font *font,
                        uint16_t angle, uint8_t scale,
                        uint8_t thickness, int8_t spacing,
                        GX_Align align, bool invert,
                        GX_Color color)
{
    if (!text || !font || scale < GX_SCALE_MIN) return;
    uint16_t cps[GX_LABEL_MAX_CP];
    uint16_t textlen = to_codepoints(text, cps, GX_LABEL_MAX_CP);
    if (!textlen) return;

    uint16_t lines = 1u; int16_t maxadv = 1;
    uint16_t ls = 0u;
    while (ls < textlen) {
        uint16_t ll = 0u;
        while (ls + ll < textlen && cps[ls + ll] != (uint16_t)'\n') ll++;
        int16_t la = line_adv(cps + ls, ll, font, spacing);
        if (la > maxadv) maxadv = la;
        if (ls + ll >= textlen) break;
        ls = ls + ll + 1u; lines++;
    }
    int16_t totalw = (int16_t)((int32_t)maxadv * scale >> 4);
    int16_t lineh  = (int16_t)((int32_t)font->height * scale >> 4);
    int16_t linesp = (scale > GX_SCALE_1X) ? (int16_t)((int32_t)2 * scale >> 4) : 2;
    if (linesp < 2) linesp = 2;
    int16_t totalh = lineh * (int16_t)lines + linesp * (int16_t)(lines - 1u);
    if (totalw < 1) totalw = 1;
    if (totalh < 1) totalh = 1;

    int16_t leftx;
    switch (align) {
    case GX_ALIGN_LEFT:  leftx = -totalw; break;
    case GX_ALIGN_RIGHT: leftx = 0;       break;
    default:             leftx = -totalw / 2; break;
    }
    int16_t rightx = leftx + totalw;
    int16_t hh     = totalh / 2;
    int16_t tp     = (int16_t)thickness;
    int32_t sa = gx_sin(angle), ca = gx_cos(angle);

    if (invert) {
        int16_t bgpts[] = { (int16_t)(leftx-1), (int16_t)(-hh-2),
                            rightx,              (int16_t)(-hh-2),
                            rightx,              (int16_t)(totalh-hh),
                            (int16_t)(leftx-1),  (int16_t)(totalh-hh) };
        gx_gfx_draw_polygon(cx, cy, bgpts, 4u, angle, 0u, true, false, color);
        color = (color == GX_COLOR_BLACK) ? GX_COLOR_WHITE : GX_COLOR_BLACK;
    }

    int16_t ex[4] = { (int16_t)(leftx-tp), rightx+tp, rightx+tp, (int16_t)(leftx-tp) };
    int16_t ey[4] = { (int16_t)(-hh-tp),  (int16_t)(-hh-tp),  (int16_t)(hh+tp),  (int16_t)(hh+tp) };
    int16_t minsx=32767, maxsx=-32767, minsy=32767, maxsy=-32767;
    for (int i = 0; i < 4; i++) {
        int16_t rpx = cx + q8r((int32_t)ex[i]*ca - (int32_t)ey[i]*sa);
        int16_t rpy = cy + q8r((int32_t)ex[i]*sa + (int32_t)ey[i]*ca);
        if (rpx < minsx) minsx = rpx; if (rpx > maxsx) maxsx = rpx;
        if (rpy < minsy) minsy = rpy; if (rpy > maxsy) maxsy = rpy;
    }
    if (minsx < 0)        minsx = 0;
    if (maxsx >= GX_WIDTH)  maxsx = (int16_t)(GX_WIDTH - 1);
    if (minsy < 0)        minsy = 0;
    if (maxsy >= GX_HEIGHT) maxsy = (int16_t)(GX_HEIGHT - 1);

    int16_t lspfont = (scale > 0u) ? (int16_t)((int32_t)linesp * 4 / scale) : linesp;

    for (int16_t sy = minsy; sy <= maxsy; sy++) {
        for (int16_t sx = minsx; sx <= maxsx; sx++) {
            int32_t dx = sx - cx, dy = sy - cy;
            int16_t lx = q8r(dx*ca + dy*sa);
            int16_t ly = q8r(-dx*sa + dy*ca);
            int16_t tx = lx - leftx;
            int16_t ty = ly + hh;
            if (tx < -tp-1 || ty < -tp-1 || tx > totalw+tp || ty > totalh+tp) continue;
            int16_t fc = (int16_t)((int32_t)tx * 4 / scale);
            int16_t fr = (int16_t)((int32_t)ty * 4 / scale);
            bool set;
            if (tp == 0) {
                set = (tx >= 0 && ty >= 0 && tx < totalw && ty < totalh)
                    && font_px(cps, textlen, font, fc, fr, spacing, font->height, lspfont);
            } else {
                set = false;
                for (int16_t dr = -tp; dr <= tp && !set; dr++)
                    for (int16_t dc = -tp; dc <= tp && !set; dc++) {
                        int32_t dx2 = sx+dc-cx, dy2 = sy+dr-cy;
                        int16_t lx2 = q8r(dx2*ca + dy2*sa);
                        int16_t ly2 = q8r(-dx2*sa + dy2*ca);
                        int16_t tx2 = lx2 - leftx, ty2 = ly2 + hh;
                        if (tx2 < 0 || ty2 < 0 || tx2 >= totalw || ty2 >= totalh) continue;
                        int16_t fc2 = (int16_t)((int32_t)tx2 * 4 / scale);
                        int16_t fr2 = (int16_t)((int32_t)ty2 * 4 / scale);
                        set = font_px(cps, textlen, font, fc2, fr2, spacing, font->height, lspfont);
                    }
            }
            if (set) gx_gfx_draw_pixel(sx, sy, color);
        }
    }
}

/* --------------------------------------------------------------------------
 * Image
 * -------------------------------------------------------------------------- */
void gx_gfx_draw_image(int16_t cx, int16_t cy,
                        const GX_Image *img,
                        uint16_t angle, uint8_t scale,
                        bool invert, GX_Color color)
{
    if (!img || !img->data || scale < GX_SCALE_MIN) return;
    int16_t totalw = (int16_t)((int32_t)img->width  * scale >> 4);
    int16_t totalh = (int16_t)((int32_t)img->height * scale >> 4);
    if (totalw < 1) totalw = 1;
    if (totalh < 1) totalh = 1;
    int16_t hw = totalw / 2, hh = totalh / 2;
    int32_t sa = gx_sin(angle), ca = gx_cos(angle);

    if (invert) {
        int16_t bgpts[] = { (int16_t)(-hw-1),(int16_t)(-hh-2), (int16_t)(hw+1),(int16_t)(-hh-2),
                            (int16_t)(hw+1),(int16_t)(hh+1),   (int16_t)(-hw-1),(int16_t)(hh+1) };
        gx_gfx_draw_polygon(cx, cy, bgpts, 4u, angle, 0u, true, false, color);
        color = (color == GX_COLOR_BLACK) ? GX_COLOR_WHITE : GX_COLOR_BLACK;
    }

    int16_t ex[4]={-hw,hw,hw,-hw}, ey[4]={-hh,-hh,hh,hh};
    int16_t minsx=32767,maxsx=-32767,minsy=32767,maxsy=-32767;
    for (int i=0;i<4;i++) {
        int16_t rpx=cx+q8r((int32_t)ex[i]*ca-(int32_t)ey[i]*sa);
        int16_t rpy=cy+q8r((int32_t)ex[i]*sa+(int32_t)ey[i]*ca);
        if(rpx<minsx)minsx=rpx; if(rpx>maxsx)maxsx=rpx;
        if(rpy<minsy)minsy=rpy; if(rpy>maxsy)maxsy=rpy;
    }
    if(minsx<0)minsx=0; if(maxsx>=GX_WIDTH)maxsx=(int16_t)(GX_WIDTH-1);
    if(minsy<0)minsy=0; if(maxsy>=GX_HEIGHT)maxsy=(int16_t)(GX_HEIGHT-1);

    uint8_t imgpages = (uint8_t)((img->height + 7u) >> 3);
    for(int16_t sy=minsy;sy<=maxsy;sy++) {
        for(int16_t sx=minsx;sx<=maxsx;sx++) {
            int32_t dx=sx-cx,dy=sy-cy;
            int16_t lx=q8r(dx*ca+dy*sa), ly=q8r(-dx*sa+dy*ca);
            int16_t tx=lx+hw, ty=ly+hh;
            if(tx<0||ty<0||tx>=totalw||ty>=totalh) continue;
            int16_t ic=(int16_t)((int32_t)tx*4/scale);
            int16_t ir=(int16_t)((int32_t)ty*4/scale);
            if(ic<0||ic>=(int16_t)img->width) continue;
            if(ir<0||ir>=(int16_t)img->height) continue;
            if(img->data[(uint16_t)ic*imgpages+(uint8_t)(ir>>3)] >> (uint8_t)(ir&7u) & 1u)
                gx_gfx_draw_pixel(sx,sy,color);
        }
    }
}

/* --------------------------------------------------------------------------
 * Flush & backlight
 * -------------------------------------------------------------------------- */
void gx_gfx_flush(void)
{
    if (!s_port) return;
    for (uint8_t page = 0u; page < GX_PAGES; page++) {
        if (!(s_dirty & (1u << page))) continue;
        s_port->write_cmd((uint8_t)(GX_ST7565_SET_PAGE | page));
        s_port->write_cmd((uint8_t)(GX_ST7565_SET_COL_HI | ((GX_COL_OFFSET >> 4) & 0x0Fu)));
        s_port->write_cmd((uint8_t)(GX_ST7565_SET_COL_LO |  (GX_COL_OFFSET       & 0x0Fu)));
        s_port->write_data(&s_framebuf[(uint16_t)page * GX_WIDTH], GX_WIDTH);
    }
    s_dirty = 0u;
}

void gx_gfx_set_backlight(bool on)
{
    if (s_port && s_port->set_backlight) s_port->set_backlight(on);
}

void gx_gfx_init(const GX_Mcu *port)
{
    if (!port) return;
    s_port = port;
    port->reset();
    port->delay_ms(5u);
    port->write_cmd(GX_ST7565_SET_BIAS_9);
    port->write_cmd(GX_ST7565_SET_ADC_NORMAL);
    port->write_cmd(GX_ST7565_SET_COM_REVERSE);
    port->write_cmd(GX_ST7565_SET_ALL_PTS_NORMAL);
    port->write_cmd(GX_ST7565_SET_DISP_NORMAL);
    port->write_cmd((uint8_t)(GX_ST7565_SET_DISP_START_LINE | 0x00u));
    port->write_cmd((uint8_t)(GX_ST7565_SET_RESISTOR_RATIO  | 0x05u));
    port->write_cmd(GX_ST7565_SET_VOLUME_FIRST);
    port->write_cmd(GX_CONTRAST);
    port->write_cmd((uint8_t)(GX_ST7565_SET_POWER_CONTROL   | 0x07u));
    gx_gfx_clear();
    gx_gfx_flush();
    port->delay_ms(2u);
    port->write_cmd(GX_ST7565_DISPLAY_ON);
}
