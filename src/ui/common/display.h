#ifndef DISPLAY_H
#define DISPLAY_H

// Display error message
void ui_show_error(const char *message);

// Display loading message
void ui_show_loading(const char *message);

// Draw a progress bar
void ui_draw_progress_bar(int percentage, int width);

#endif /* DISPLAY_H */