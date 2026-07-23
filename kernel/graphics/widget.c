#include "widget.h"
#include "../drivers/vbe.h"

static widget_t widget_pool[MAX_WIDGETS];
static size_t widget_count = 0;

void widget_system_init(void) {
    widget_count = 0;
    for (size_t i = 0; i < MAX_WIDGETS; i++) {
        widget_pool[i].visible = 0;
        widget_pool[i].title[0] = '\0';
        widget_pool[i].draw_content = NULL;
        widget_pool[i].user_data = NULL;
    }
}

widget_t* widget_create(uint32_t x, uint32_t y, uint32_t w, uint32_t h, const char* title, void (*draw_content)(widget_t*, uint64_t)) {
    if (widget_count >= MAX_WIDGETS) return NULL;

    widget_t* w_ptr = &widget_pool[widget_count++];
    w_ptr->x = x;
    w_ptr->y = y;
    w_ptr->w = w;
    w_ptr->h = h;
    w_ptr->visible = 1;
    w_ptr->style = WIDGET_STYLE_BORDERED;
    w_ptr->bg_color = COLOR_OBSIDIAN;
    w_ptr->border_color = COLOR_AZTEC_GOLD;
    w_ptr->header_color = COLOR_JADE;
    w_ptr->text_color = COLOR_FEATHER_WHITE;
    w_ptr->draw_content = draw_content;
    w_ptr->user_data = NULL;

    size_t i = 0;
    if (title) {
        for (; i < WIDGET_TITLE_MAX - 1 && title[i] != '\0'; i++) {
            w_ptr->title[i] = title[i];
        }
    }
    w_ptr->title[i] = '\0';

    return w_ptr;
}

void widget_set_style(widget_t* w, widget_style_t style, uint32_t bg, uint32_t border, uint32_t header, uint32_t text) {
    if (!w) return;
    w->style = style;
    w->bg_color = bg;
    w->border_color = border;
    w->header_color = header;
    w->text_color = text;
}

void widget_draw_all(uint64_t tick) {
    for (size_t i = 0; i < widget_count; i++) {
        widget_t* w = &widget_pool[i];
        if (!w->visible) continue;

        /* Draw Container Background */
        if (w->bg_color != COLOR_TRANSPARENT) {
            vbe_fill_rect(w->x, w->y, w->w, w->h, w->bg_color);
        }

        /* Draw Style Frame / Border */
        if (w->style == WIDGET_STYLE_BORDERED || w->style == WIDGET_STYLE_HEADER) {
            /* Outer border rectangle */
            vbe_fill_rect(w->x, w->y, w->w, 2, w->border_color);
            vbe_fill_rect(w->x, w->y + w->h - 2, w->w, 2, w->border_color);
            vbe_fill_rect(w->x, w->y, 2, w->h, w->border_color);
            vbe_fill_rect(w->x + w->w - 2, w->y, 2, w->h, w->border_color);
        }

        /* Draw Header Banner if present */
        if (w->style == WIDGET_STYLE_HEADER && w->h >= 24) {
            vbe_fill_rect(w->x + 2, w->y + 2, w->w - 4, 20, w->header_color);
            if (w->title[0] != '\0') {
                vbe_draw_string(w->x + 8, w->y + 6, w->title, w->text_color, w->header_color);
            }
        }

        /* Invoke custom widget renderer */
        if (w->draw_content) {
            w->draw_content(w, tick);
        }
    }
}
