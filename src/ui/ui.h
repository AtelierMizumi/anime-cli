#ifndef UI_H
#define UI_H

#include <stdbool.h>
#include "../api/api.h"

// Content selection options
typedef enum {
    CONTENT_SELECTION_ANIME,
    CONTENT_SELECTION_MANGA,
    CONTENT_SELECTION_EXIT
} ContentSelectionOption;

// Provider selection result
typedef struct {
    ProviderType selected_provider;
    bool canceled;
} ProviderSelectionResult;

// Initialize UI system
void ui_init();

// Clean up UI resources
void ui_cleanup();

// Display loading indicator
void ui_show_loading(const char *message);

// Display error message
void ui_show_error(const char *message);

// Main content type selection menu
ContentSelectionOption ui_content_selection();

// Provider selection menu
ProviderSelectionResult ui_provider_selection(ContentType content_type);

// Settings menu
void ui_settings_menu();

// About screen
void ui_about_screen();

#endif /* UI_H */