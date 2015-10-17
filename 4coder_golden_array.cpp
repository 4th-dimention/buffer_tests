/* 
 * Mr. 4th Dimention - Allen Webster
 *  Four Tech
 *
 * 16.10.2015
 * 
 * Buffer data object
 *  type - Golden Array
 * 
 */

// TOP

#define inline_4tech inline
#define internal_4tech static

#ifndef Assert
#define Assert
#endif

typedef struct{
    char *data;
    int size, max;
    
    real32 *line_widths;
    i32 *line_starts;
    i32 line_count, line_max;
    i32 width_max;
} Buffer;

typedef struct{
    Buffer *buffer;
    char *data;
    int size;
} Buffer_Save_Loop;

inline_4tech Buffer_Save_Loop
buffer_save_loop(Buffer *buffer){
    Buffer_Save_Loop result;
    result.buffer = buffer;
    result.data = buffer->data;
    result.size = buffer->size;
    return(result);
}

inline_4tech int
buffer_save_good(Buffer_Save_Loop *loop){
    int result;
    result = (loop->buffer != 0);
    return(result);
}

inline_4tech void
buffer_save_next(Buffer_Save_Loop *loop){
    loop->buffer = 0;
}

internal_4tech int
buffer_count_newlines(Buffer *buffer, int start, int end, int CR, int LF){
    int new_line, count;
    char *data;
    int i;
    
    data = buffer->data;
    new_line = 0;
    count = 0;
    
    for (i = start; i < end; ++i){
        switch(data[i]){
        case '\n': new_line = LF; break;
        case '\r': new_line = CR; break;
        default: new_line = 0; break;
        }
        count += new_line;
    }
    
    return (count);
}

typedef struct{
    int i;
    int count;
    int start;
} Buffer_Measure_Starts;

internal_4tech int
buffer_measure_starts(Buffer_Measure_Starts *state, Buffer *buffer, int CR, int LF){
    int *starts;
    int max;
    char *data;
    int size;
    int start, count, i, new_line;
    int result;
    
    starts = buffer->line_starts;
    max = buffer->line_max;
    
    data = buffer->data;
    size = buffer->size;
    
    result = 0;
    start = state->start;
    count = state->count;
    
    for (i = state->i; i < size; ++i){
        switch (data[i]){
        case '\n': new_line = LF; break;
        case '\r': new_line = CR; break;
        default: new_line = 0; break;
        }
        
        if (new_line){
            if (count == max){
                result = 1;
                break;
            }
            
            starts[count++] = start;
            start = i + 1;
        }
    }
    
    if (i == size){
        if (count == max) result = 1;
        else starts[count++] = start;
    }
    
    state->i = i;
    state->count = count;
    state->start = start;
    
    return (result);
}

inline_4tech float
measure_character_merge_CR(void *data, int offset, int stride, int *new_line, char character, char next){
    char *data_;
    float width;
    
    data_ = (char*)data + offset;
    switch (character){
    case 0: width = 0; *new_line = 1; break;
    case '\n': width = *(float*)(data_ + stride * '\n'); *new_line = 1; break;
    case '\r':
        if (next == '\n') width = 0;
        else width = *(float*)(data_ + stride * '\\') + *(float*)(data_ + stride * '\r');
        break;
    default: width = *(float*)(data_ + stride * character);
    }
    
    return (width);
}

inline_4tech float
measure_character_use_CR(void *data, int offset, int stride, int *new_line, char character){
    char *data_;
    float width;
    
    data_ = (char*)data + offset;
    switch (character){
    case 0: width = 0; *new_line = 1; break;
    case '\r':
    case '\n': width = *(float*)(data_ + stride * '\n'); *new_line = 1; break;
    default: width = *(float*)(data_ + stride * character);
    }
    
    return (width);
}

inline_4tech float
measure_character_show_CR(void *data, int offset, int stride, int *new_line, char character){
    char *data_;
    float width;
    
    data_ = (char*)data + offset;
    switch (character){
    case 0: width = 0; *new_line = 1; break;
    case '\n': width = *(float*)(data_ + stride * '\n'); *new_line = 1; break;
    case '\r': width = *(float*)(data_ + stride * '\\') + *(float*)(data_ + stride * '\r'); break;
    default: width = *(float*)(data_ + stride * character);
    }
    
    return (width);
}

// TODO(allen): Use your meta-program here once you've got one.
internal_4tech void
buffer_measure_widths_merge_CR(Buffer *buffer, void *width_data, int offset, int stride){
    float *widths;
    int *starts;
    int line_count;
    char *data;
    int size;
    int i, j, new_line;
    float width;
    char ch, next;
    
    widths = buffer->line_widths;
    starts = buffer->line_starts;
    line_count = buffer->line_count;
    
    data = buffer->data;
    size = buffer->size;
    
    Assert(size < buffer->max);
    data[size] = 0;
    
    for (i = 0, j = 0; i < line_count; ++i){
        Assert(j == starts[i]);
        new_line = 0;
        width = 0;
        ch = data[j];
        next = data[++j];
        
        while (new_line == 0){
            width += measure_character_merge_CR(width_data, offset, stride, &new_line, ch, next);
            ch = next;
            next = data[++j];
        }
        
        --j;
        widths[i] = width;
    }
}

internal_4tech void
buffer_measure_widths_use_CR(Buffer *buffer, void *width_data, int offset, int stride){
    float *widths;
    int *starts;
    int line_count;
    char *data;
    int size;
    int i, j, new_line;
    float width;
    char ch, next;
    
    widths = buffer->line_widths;
    starts = buffer->line_starts;
    line_count = buffer->line_count;
    
    data = buffer->data;
    size = buffer->size;
    
    Assert(size < buffer->max);
    data[size] = 0;
    
    for (i = 0, j = 0; i < line_count; ++i){
        Assert(j == starts[i]);
        new_line = 0;
        width = 0;
        ch = data[j];
        next = data[++j];
        
        while (new_line == 0){
            width += measure_character_use_CR(width_data, offset, stride, &new_line, ch);
            ch = next;
            next = data[++j];
        }
        
        --j;
        widths[i] = width;
    }
}

internal_4tech void
buffer_measure_widths_show_CR(Buffer *buffer, void *width_data, int offset, int stride){
    float *widths;
    int *starts;
    int line_count;
    char *data;
    int size;
    int i, j, new_line;
    float width;
    char ch, next;
    
    widths = buffer->line_widths;
    starts = buffer->line_starts;
    line_count = buffer->line_count;
    
    data = buffer->data;
    size = buffer->size;
    
    Assert(size < buffer->max);
    data[size] = 0;
    
    for (i = 0, j = 0; i < line_count; ++i){
        Assert(j == starts[i]);
        new_line = 0;
        width = 0;
        ch = data[j];
        next = data[++j];
        
        while (new_line == 0){
            width += measure_character_show_CR(width_data, offset, stride, &new_line, ch);
            ch = next;
            next = data[++j];
        }
        
        --j;
        widths[i] = width;
    }
}

internal_4tech void
buffer_remeasure_starts(Buffer *buffer, int line_start, int line_end, int line_shift, int text_shift, int CR, int LF){
    int *lines;
    int line_count;
    char *data;
    int size;
    int line_i, char_i, start;
    int new_line;
    char character;
    
    lines = buffer->line_starts;
    line_count = buffer->line_count;
    
    if (line_shift != 0){
        memmove(lines + line_end + line_shift + 1, lines + line_end + 1,
                sizeof(int)*(line_count - line_end - 1));
        line_count += line_shift;
    }
    
    if (text_shift != 0){
        line_i = line_end + 1;
        lines = lines + line_i;
        for (; line_i < line_count; ++line_i, ++lines){
            *lines += text_shift;
        }
        lines = buffer->line_starts;
    }
    
    size = buffer->size;
    data = buffer->data;
    char_i = lines[line_start];
    line_i = line_start;
    
    Assert(size < buffer->max);
    data[size] = '\n';
    
    start = char_i;
    for (; char_i <= size; ++char_i){
        character = data[char_i];
        
        switch(data[char_i]){
        case '\n': new_line = LF; break;
        case '\r': new_line = CR; break;
        default: new_line = 0; break;
        }
        
        if (new_line){
            if (line_i > line_end && start == lines[line_i]) break;
            lines[line_i++] = start;
            start = char_i + 1;
        }
    }
    
    buffer->line_count = line_count;
}

internal_4tech int
buffer_get_line_index(Buffer *buffer, int pos){
    int *lines;
    int start, end;
    int i;
    
    start = 0;
    end = buffer->line_count;
    lines = buffer->line_starts;
    for (;;){
        i = (start + end) >> 1;
        if (lines[i] < pos) start = i;
        else if (lines[i] > pos) end = i;
        else{
            start = i;
            break;
        }
        Assert(start < end);
        if (start == end - 1) break;
    }
    
    return start;
}

internal_4tech int
buffer_replace_range(Buffer *buffer, int start, int end, char *str, int len, int *shift_amount){
    char *data;
    int result;
    
    *shift_amount = (len - (end - start));
    if (*shift_amount + buffer->size + 1 <= buffer->max){
        data = (char*)buffer->data;
        memmove(data + end + *shift_amount, data + end, buffer->size - end);
        buffer->size += *shift_amount;
        data[buffer->size] = 0;
        memcpy(data + start, str, len);
        
        result = 0;
    }
    else{
        result = *shift_amount + buffer->size + 1;
    }
    
    return result;
}

typedef enum{
    buffer_seek_pos,
    buffer_seek_wrapped_xy,
    buffer_seek_unwrapped_xy,
    buffer_seek_line_char
} Buffer_Seek_Type;

typedef struct{
    Buffer_Seek_Type type;
    union{
        struct { int pos; };
        struct { int round_down; float x, y; };
        struct { int line, character; };
    };
} Buffer_Seek;

inline_4tech Buffer_Seek
seek_pos(int pos){
	Buffer_Seek result;
	result.type = buffer_seek_pos;
	result.pos = pos;
	return (result);
}

inline_4tech Buffer_Seek
seek_wrapped_xy(float x, float y, int round_down){
	Buffer_Seek result;
	result.type = buffer_seek_wrapped_xy;
	result.x = x;
	result.y = y;
    result.round_down = round_down;
	return (result);
}

inline_4tech Buffer_Seek
seek_unwrapped_xy(float x, float y, int round_down){
	Buffer_Seek result;
	result.type = buffer_seek_unwrapped_xy;
	result.x = x;
	result.y = y;
    result.round_down = round_down;
	return (result);
}

inline_4tech Buffer_Seek
seek_line_char(int line, int character){
	Buffer_Seek result;
	result.type = buffer_seek_line_char;
	result.line = line;
	result.character = character;
	return (result);
}

struct Full_Cursor{
    int pos;
    int line, character;
    float unwrapped_x, unwrapped_y;
    float wrapped_x, wrapped_y;
};

internal_4tech Full_Cursor
buffer_cursor_seek(Buffer *buffer, Buffer_Seek seek, int CR_mode,
                   float max_width, float font_height, void *width_data, int offset, int stride, Full_Cursor cursor){
    Full_Cursor prev_cursor;
    char *data, *data_;
    int size;
    int do_newline, do_slashr;
    char ch, next;
    float ch_width;
    
    int get_out;
    int xy_seek;
    float x, y, px;
    
    data = buffer->data;
    size = buffer->size;
    Assert(size < buffer->max);
    data[size] = 0;
    
    data_ = (char*)width_data + offset;
    
    xy_seek = (seek.type == buffer_seek_wrapped_xy || seek.type == buffer_seek_unwrapped_xy);
    
	for (;;){
		prev_cursor = cursor;
        do_newline = 0;
        do_slashr = 0;
        ch_width = 0;
        ch = data[cursor.pos];
        next = data[cursor.pos+1];
        
		switch (ch){
        case '\r':
            do_newline = (CR_mode == 1);
            do_slashr = (!do_newline && (next != '\n' || CR_mode == 2));
            break;
		
        case '\n': do_newline = 1; break;
		
        default:
			++cursor.character;
            ch_width = *(float*)(data_ + stride * ch);
            break;
		}
        
        if (do_slashr){
            ++cursor.character;
            ch_width = *(float*)(data_ + stride * '\\') + *(float*)(data_ + stride * 'r');
        }
        
        if (cursor.wrapped_x + ch_width >= max_width){
            cursor.wrapped_y += font_height;
            cursor.wrapped_x = 0;
            prev_cursor = cursor;
        }
        
        cursor.unwrapped_x += ch_width;
        cursor.wrapped_x += ch_width;
        
        if (do_newline){
			++cursor.line;
			cursor.unwrapped_y += font_height;
            cursor.wrapped_y += font_height;
            cursor.character = 0;
			cursor.unwrapped_x = 0;
			cursor.wrapped_x = 0;
        }
        
		++cursor.pos;
		
		if (cursor.pos > size){
			cursor = prev_cursor;
			break;
		}
        
        get_out = 0;
        x = 0;
        y = 0;
        px = 0;
        
        switch (seek.type){
        case buffer_seek_pos:
            if (cursor.pos > seek.pos){
                cursor = prev_cursor;
                get_out = 1;
            }break;
            
        case buffer_seek_wrapped_xy:
            x = cursor.wrapped_x; px = prev_cursor.wrapped_x;
            y = cursor.wrapped_y; break;
            
        case buffer_seek_unwrapped_xy:
            x = cursor.unwrapped_x; px = prev_cursor.unwrapped_x;
            y = cursor.unwrapped_y; break;
                
        case buffer_seek_line_char:
            if (cursor.line == seek.line && cursor.character >= seek.character){
                get_out = 1;
            }
            else if (cursor.line > seek.line){
                cursor = prev_cursor;
                get_out = 1;
            }break;
        }
        
        if (get_out) break;
        if (xy_seek){
            if (y > seek.y){
                cursor = prev_cursor;
                break;
            }
            
            if (seek.round_down){
                if (y > seek.y - font_height && x > seek.x){
                    cursor = prev_cursor;
                    break;
                }
            }
            else{
                if (y > seek.y - font_height && x >= seek.x){
                    if ((seek.x - px) < (x - seek.x)) cursor = prev_cursor;
                    break;
                }
            }
        }
	}
	
	return cursor;
}

// BOTTOM

