#ifndef QUETZAL_WIDGET_H
#define QUETZAL_WIDGET_H

#include <stdint.h>
#include <stddef.h>

#define MAX_WIDGETS 16
#define WIDGET_TITLE_MAX 80

typedef enum {
    WIDGET_STYLE_CONTAINER,
    WIDGET_STYLE_BORDERED,
    WIDGET_STYLE_HEADER
} widget_style_t;

typedef struct widget {
    uint32_t x;
    uint32_t y;
    uint32_t w;
    uint32_t h;
    char title[WIDGET_TITLE_MAX];
    uint32_t bg_color;
    uint32_t border_color;
    uint32_t header_color;
    uint32_t text_color;
    widget_style_t style;
    int visible;
    void (*draw_content)(struct widget* w, uint64_t tick);
    void* user_data;
} widget_t;

void widget_system_init(void);
widget_t* widget_create(uint32_t x, uint32_t y, uint32_t w, uint32_t h, const char* title, void (*draw_content)(widget_t*, uint64_t));
void widget_set_style(widget_t* w, widget_style_t style, uint32_t bg, uint32_t border, uint32_t header, uint32_t text);
void widget_draw_all(uint64_t tick);

#endif /* QUETZAL_WIDGET_H */
