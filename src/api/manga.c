#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "manga.h"
#include "../config.h"

SearchResult* manga_search(const char *query) {
    const ProviderAPI *api = get_provider_api(get_current_provider());
    if (!api || !api->search) {
        return NULL;
    }
    
    return api->search(query);
}

MangaInfo* manga_get_info(const char *id) {
    const ProviderAPI *api = get_provider_api(get_current_provider());
    if (!api || !api->get_manga_info) {
        return NULL;
    }
    
    return (MangaInfo*)api->get_manga_info(id);
}

ChapterPages* manga_get_chapter_pages(const char *chapter_id) {
    const ProviderAPI *api = get_provider_api(get_current_provider());
    if (!api || !api->get_chapter_pages) {
        return NULL;
    }
    
    return (ChapterPages*)api->get_chapter_pages(chapter_id);
}

void manga_free_search_results(SearchResult *results) {
    const ProviderAPI *api = get_provider_api(get_current_provider());
    if (!api || !api->free_search_results || !results) {
        return;
    }
    
    api->free_search_results(results);
}

void manga_free_info(MangaInfo *info) {
    const ProviderAPI *api = get_provider_api(get_current_provider());
    if (!api || !api->free_manga_info || !info) {
        return;
    }
    
    api->free_manga_info(info);
}

void manga_free_chapter_pages(ChapterPages *pages) {
    const ProviderAPI *api = get_provider_api(get_current_provider());
    if (!api || !api->free_chapter_info || !pages) {
        return;
    }
    
    api->free_chapter_info(pages);
}