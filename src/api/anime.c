#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "anime.h"
#include "../config.h"

SearchResult* anime_search(const char *query) {
    const ProviderAPI *api = get_provider_api(get_current_provider());
    if (!api || !api->search) {
        return NULL;
    }
    
    return api->search(query);
}

AnimeInfo* anime_get_info(const char *id) {
    const ProviderAPI *api = get_provider_api(get_current_provider());
    if (!api || !api->get_anime_info) {
        return NULL;
    }
    
    return (AnimeInfo*)api->get_anime_info(id);
}

StreamInfo* anime_get_episode_stream(const char *episode_id, const char *server) {
    const ProviderAPI *api = get_provider_api(get_current_provider());
    if (!api || !api->get_episode_stream) {
        return NULL;
    }
    
    return (StreamInfo*)api->get_episode_stream(episode_id, server);
}

void anime_free_search_results(SearchResult *results) {
    const ProviderAPI *api = get_provider_api(get_current_provider());
    if (!api || !api->free_search_results || !results) {
        return;
    }
    
    api->free_search_results(results);
}

void anime_free_info(AnimeInfo *info) {
    const ProviderAPI *api = get_provider_api(get_current_provider());
    if (!api || !api->free_anime_info || !info) {
        return;
    }
    
    api->free_anime_info(info);
}

void anime_free_stream_info(StreamInfo *info) {
    const ProviderAPI *api = get_provider_api(get_current_provider());
    if (!api || !api->free_stream_info || !info) {
        return;
    }
    
    api->free_stream_info(info);
}