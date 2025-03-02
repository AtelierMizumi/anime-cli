#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ui.h"
#include "api.h"

int main(int argc, char *argv[]) {
    (void)argc;  // Suppress unused parameter warning
    (void)argv;  // Suppress unused parameter warning
    
    ui_init();

    // Get search query from user
    char *search_query = ui_get_search_query();
    if (!search_query || strlen(search_query) == 0) {
        free(search_query);
        ui_cleanup();
        return EXIT_FAILURE;
    }

    // Show loading indicator and search for anime
    ui_show_loading();
    SearchResult *results = api_search_anime(search_query);
    
    if (!results || results->total_results == 0) {
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg), "No results found for: %s", search_query);
        ui_show_error(error_msg);
        free(search_query);
        ui_cleanup();
        return EXIT_FAILURE;
    }

    // Let user select an anime
    AnimeInfo *selected_anime = ui_select_anime(results);
    api_free_search_results(results);
    
    if (!selected_anime) {
        free(search_query);
        ui_cleanup();
        return EXIT_SUCCESS; // User cancelled
    }

    // Get detailed anime info including episodes
    ui_show_loading();
    AnimeInfo *detailed_anime = api_get_anime_info(selected_anime->id);
    api_free_anime_info(selected_anime); // Free the simple version
    
    if (!detailed_anime || detailed_anime->total_episodes <= 0) {
        ui_show_error("No episodes available for this anime.");
        free(search_query);
        if (detailed_anime) api_free_anime_info(detailed_anime);
        ui_cleanup();
        return EXIT_FAILURE;
    }

    // Loop for selecting and watching episodes
    int stay_in_episode_menu = 1;
    while (stay_in_episode_menu) {
        // Let user select an episode
        Episode *selected_episode = ui_select_episode(detailed_anime);
        
        if (selected_episode) {
            // Get streaming link for the episode
            ui_show_loading();
            StreamInfo *stream_info = api_get_episode_stream(selected_episode->id, "vidstreaming");
            
            if (stream_info && stream_info->sources_count > 0) {
                // Play the episode
                ui_play_episode(stream_info);
                api_free_stream_info(stream_info);
                
                // After playing, we'll loop back to episode selection
                // No need to do anything here, just continue the loop
            } else {
                ui_show_error("Failed to get streaming link.");
                // Still stay in the episode menu
            }
        } else {
            // User cancelled episode selection, exit the loop
            stay_in_episode_menu = 0;
        }
    }

    // Clean up
    api_free_anime_info(detailed_anime);
    free(search_query);
    ui_cleanup();
    return EXIT_SUCCESS;
}