#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <version.h>
#include "config.h"
#include "ui/ui.h"
#include "ui/anime_ui.h"
#include "ui/manga_ui.h"
#include "api/api.h"
#include "api/anime.h"
#include "api/manga.h"

int main(int argc, char *argv[]) {
    (void)argc;  // Suppress unused parameter warning
    (void)argv;  // Suppress unused parameter warning
    
    // Initialize systems
    config_init();
    api_init();
    ui_init();
    
    while (1) {
        // Show content selection menu
        ContentSelectionOption content_option = ui_content_selection();
        
        // Exit if requested
        if (content_option == CONTENT_SELECTION_EXIT) {
            break;
        }
        
        // Select provider for the chosen content type
        ProviderSelectionResult provider_result;
        
        if (content_option == CONTENT_SELECTION_ANIME) {
            provider_result = ui_provider_selection(CONTENT_ANIME);
            if (!provider_result.canceled) {
                set_current_provider(provider_result.selected_provider);
                anime_ui_main_loop();
            }
        } else if (content_option == CONTENT_SELECTION_MANGA) {
            provider_result = ui_provider_selection(CONTENT_MANGA);
            if (!provider_result.canceled) {
                set_current_provider(provider_result.selected_provider);
                manga_ui_main_loop();
            }
        }
    }
    
    // Clean up systems
    ui_cleanup();
    api_cleanup();
    config_cleanup();
    
    return EXIT_SUCCESS;
}