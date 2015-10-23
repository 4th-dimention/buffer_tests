/* 
 * Mr. 4th Dimention - Allen Webster
 *  Four Tech
 *
 * public domain -- no warranty is offered or implied; use this code at your own risk
 * 
 * 23.10.2015
 * 
 * Buffer data object
 *  type - Gap Buffer
 * 
 */

// TOP

typedef struct{
    char *data;
    int size1, gap_size, size2, max;
    
    float *line_widths;
    int *line_starts;
    int line_count;
    int line_max;
    int widths_max;
} Gap_Buffer;

typedef struct{
    Gap_Buffer *buffer;
    char *data, *base;
    int pos, end;
    int size;
    int page_size;
    int seperated;
} Gap_Buffer_Stringify_Loop;

inline_4tech int
gap_buffer_size(Gap_Buffer *buffer){
    int size;
    size = buffer->size1 + buffer->size2;
    return(size);
}

inline_4tech Gap_Buffer_Stringify_Loop
gap_buffer_stringify_loop(Gap_Buffer *buffer, int start, int end, int page_size){
    Gap_Buffer_Stringify_Loop result;
    if (0 <= start && start < end && end <= buffer->size1 + buffer->size2){
        result.buffer = buffer;
        result.base = buffer->data;
        result.page_size = page_size;
        
        if (end <= buffer->size1) result.end = end;
        else result.end = end + buffer->gap_size;
        
        if (start < buffer->size1){
            if (end <= buffer->size1) result.seperated = 0;
            else result.seperated = 1;
            result.pos = start;
        }
        else{
            result.seperated = 0;
            result.pos = start + buffer->gap_size;
        }
        if (result.seperated) result.size = buffer->size1 - start;
        else result.size = end - start;
        if (result.size > page_size) result.size = page_size;
        result.data = buffer->data + result.pos;
    }
    else result.buffer = 0;
    return(result);
}

inline_4tech int
gap_buffer_stringify_good(Gap_Buffer_Stringify_Loop *loop){
    int result;
    result = (loop->buffer != 0);
    return(result);
}

inline_4tech void
gap_buffer_stringify_next(Gap_Buffer_Stringify_Loop *loop){
    int size1, temp_end;
    if (loop->seperated){
        size1 = loop->buffer->size1;
        if (loop->pos + loop->size == size1){
            loop->seperated = 0;
            loop->pos = loop->buffer->gap_size + size1;
            temp_end = loop->end;
        }
        else{
            loop->pos += loop->page_size;
            temp_end = size1;
        }
    }
    else{
        if (loop->pos + loop->size == loop->end){
            loop->buffer = 0;
            temp_end = loop->pos;
        }
        else{
            loop->pos += loop->page_size;
            temp_end = loop->end;
        }
    }
    loop->size = temp_end - loop->pos;
    if (loop->size > loop->page_size) loop->size = loop->page_size;
    loop->data = loop->base + loop->pos;
}

inline_4tech void
gap_buffer_stringify(Gap_Buffer *buffer, int start, int end, char *out){
    for (Gap_Buffer_Stringify_Loop loop = gap_buffer_stringify_loop(buffer, start, end, end - start);
         gap_buffer_stringify_good(&loop);
         gap_buffer_stringify_next(&loop)){
        memcpy_4tech(out, loop.data, loop.size);
        out += loop.size;
    }
}

internal_4tech int
gap_buffer_count_newlines(Gap_Buffer *buffer, int start, int end){
    char *data;
    int i;
    int count;
    int stop;
    
    assert_4tech(0 <= start);
    assert_4tech(start <= end);
    assert_4tech(end < buffer->size1 + buffer->size2);
    
    data = buffer->data;
    count = 0;
    
    i = start;
    stop = buffer->size1;
    if (stop > end) stop = end;

    for (steps = 0; steps < 2; ++steps){
        for (; i < stop; ++i){
            count += (data[i] == '\n');
        }
        stop = end + buffer->gap_size;
        i += buffer->gap_size;
    }
}

typedef struct{
    int index;
    int glyphid;
    float x0, y0;
    float x1, y1;
} Gap_Buffer_Render_Item;

inline_4tech void
write_render_item(Gap_Buffer_Render_Item *item, int index, int glyphid,
                  float x, float y, float w, float h){
    item->index = index;
    item->glyphid = glyphid;
    item->x0 = x;
    item->y0 = y;
    item->x1 = x + w;
    item->y1 = y + h;
}

inline_4tech float
write_render_item_inline(Gap_Buffer_Render_Item *item, int index, int glyphid,
                         float x, float y, void *advance_data, int stride, float h){
    float ch_width;
    ch_width = measure_character(advance_data, stride, (char)glyphid);
    write_render_item(item, index, glyphid, x, y, ch_width, h);
    return(ch_width);
}

internal_4tech void
gap_buffer_get_render_data(Gap_Buffer *buffer, float *wraps, Gap_Buffer_Render_Item *items, int max, int *count,
                           float port_x, float port_y, float scroll_x, float scroll_y, int wrapped,
                           float width, float height, void *advance_data, int stride, float font_height){
    Gap_Buffer_Render_Item *item;
    char *data;
    int size;
    float shift_x, shift_y;
    float x, y;
    int steps;
    int i, item_i;
    float ch_width, ch_width_sub;
    char ch;

    data = buffer->data;

    shift_x = port_x - scroll_x;
    shift_y = port_y - scroll_y;
    i = 0;

    x = shift_x;
    y = shift_y;
    item_i = 0;
    item = items + item_i;
    size = buffer->size1;

    for (steps = 0; steps < 2; ++steps){
        for (; i <= size; ++i){
            ch = data[i];
            ch_width = measure_character(advance_data, stride, ch);

            if (ch_width + x > width + shift_x && wrapped){
                x = shift_x;
                y += font_height;
            }
            if (y > height + shift_y) break;

            switch (ch){
            case '\n':
                write_render_item_inline(item, i, ' ', x, y, advance_data, stride, font_height);
                ++item_i;
                ++item;

                x = shift_x;
                y += font_height;
                break;

            case 0:
                ch_width = write_render_item_inline(item, i, '\\', x, y, advance_data, stride, font_height);
                ++item_i;
                ++item;
                x += ch_width;

                ch_width = write_render_item_inline(item, i, '0', x, y, advance_data, stride, font_height);
                ++item_i;
                ++item;
                x += ch_width;
                break;

            case '\r':
                ch_width = write_render_item_inline(item, i, '\\', x, y, advance_data, stride, font_height);
                ++item_i;
                ++item;
                x += ch_width;

                ch_width = write_render_item_inline(item, i, 'r', x, y, advance_data, stride, font_height);
                ++item_i;
                ++item;
                x += ch_width;
                break;

            case '\t':
                ch_width_sub = write_render_item_inline(item, i, '\\', x, y, advance_data, stride, font_height);
                ++item_i;
                ++item;

                write_render_item_inline(item, i, 't', x + ch_width_sub, y, advance_data, stride, font_height);
                ++item_i;
                ++item;
                x += ch_width;
                break;

            default:
                write_render_item(item, i, ch, x, y, ch_width, font_height);
                ++item_i;
                ++item;
                x += ch_width;
                break;
            }
            if (y > height + shift_y) break;
        }
        i += buffer->gap_size;
        size += buffer->gap_size;
    }

    // TODO(allen): handle this with a control state
    assert_4tech(item_i <= max);
    *count = item_i;
}

// TODO(allen): unfinished below here

internal_4tech int
gap_buffer_replace_range(Gap_Buffer *buffer, int start, int end, char *str, int len, int *shift_amount){
    return(0);
}

internal_4tech Full_Cursor
gap_buffer_cursor_seek(Gap_Buffer *buffer, Buffer_Seek seek, float max_width, float font_height,
                       void *advance_data, int stride, Full_Cursor cursor){
    return(cursor);
}

internal_4tech Full_Cursor
gap_buffer_cursor_from_pos(Gap_Buffer *buffer, int pos, float *wraps,
                           float max_width, float font_height, void *advance_data, int stride){
    Full_Cursor result;
    result = {};
    return(result);
}

internal_4tech Full_Cursor
gap_buffer_cursor_from_unwrapped_xy(Gap_Buffer *buffer, float x, float y, int round_down, float *wraps,
                                    float max_width, float font_height, void *advance_data, int stride){
    Full_Cursor result;
    result = {};
    return(result);
}

internal_4tech Full_Cursor
gap_buffer_cursor_from_wrapped_xy(Gap_Buffer *buffer, float x, float y, int round_down, float *wraps,
                                  float max_width, float font_height, void *advance_data, int stride){
    Full_Cursor result;
    result = {};
    return(result);
}

// BOTTOM

