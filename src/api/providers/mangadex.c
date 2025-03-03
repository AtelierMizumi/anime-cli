#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <json-c/json.h>
#include "mangadex.h"
#include "../../utils/memory.h"

#define MANGADEX_API_BASE_URL "https://consumet.thuanc177.me/manga/mangadex"

typedef struct {
    char *data;
    size_t size;
} MemoryStruct;

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    MemoryStruct *mem = (MemoryStruct *)userp;
    
    char *ptr = realloc(mem->data, mem->size + realsize + 1);
    if (!ptr) {
        fprintf(stderr, "Not enough memory (realloc returned NULL)\n");
        return 0;
    }
    
    mem->data = ptr;
    memcpy(&(mem->data[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->data[mem->size] = 0;
    
    return realsize;
}

SearchResult* mangadex_search_manga(const char *query) {
    CURL *curl;
    CURLcode res;
    MemoryStruct chunk = {NULL, 0};
    char url[512];
    
    
    // Build URL for manga search endpoint
    snprintf(url, sizeof(url), "%s/%s", 
             MANGADEX_API_BASE_URL, query);
    
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Failed to initialize curl\n");
        return NULL;
    }
    
    // Set up curl options
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0");
    
    // Perform the request
    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    
    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        free(chunk.data);
        curl_global_cleanup();
        return NULL;
    }
    
    // Parse JSON response
    struct json_object *json_obj = json_tokener_parse(chunk.data);
    if (!json_obj) {
        fprintf(stderr, "Failed to parse JSON response\n");
        free(chunk.data);
        curl_global_cleanup();
        return NULL;
    }
    
    // Extract results array
    struct json_object *data_array;
    if (!json_object_object_get_ex(json_obj, "data", &data_array)) {
        fprintf(stderr, "No data field in JSON response\n");
        json_object_put(json_obj);
        free(chunk.data);
        curl_global_cleanup();
        return NULL;
    }
    
    // Create search result structure
    SearchResult *search_result = malloc(sizeof(SearchResult));
    if (!search_result) {
        fprintf(stderr, "Failed to allocate memory for search results\n");
        json_object_put(json_obj);
        free(chunk.data);
        curl_global_cleanup();
        return NULL;
    }
    
    // Get array length
    int num_results = json_object_array_length(data_array);
    search_result->total_results = num_results;
    search_result->results = malloc(num_results * sizeof(SearchResultItem));
    
    if (!search_result->results) {
        fprintf(stderr, "Failed to allocate memory for manga results\n");
        free(search_result);
        json_object_put(json_obj);
        free(chunk.data);
        curl_global_cleanup();
        return NULL;
    }
    
    // Extract each result
    for (int i = 0; i < num_results; i++) {
        struct json_object *manga_obj = json_object_array_get_idx(data_array, i);
        struct json_object *id_obj, *attributes_obj, *title_obj, *en_title_obj;
        
        // Get manga ID
        if (json_object_object_get_ex(manga_obj, "id", &id_obj))
            search_result->results[i].id = strdup(json_object_get_string(id_obj));
        else
            search_result->results[i].id = NULL;
        
        // Get attributes object which contains title
        if (json_object_object_get_ex(manga_obj, "attributes", &attributes_obj)) {
            // Get title object which contains language-specific titles
            if (json_object_object_get_ex(attributes_obj, "title", &title_obj)) {
                // Try to get English title first
                if (json_object_object_get_ex(title_obj, "en", &en_title_obj))
                    search_result->results[i].title = strdup(json_object_get_string(en_title_obj));
                else {
                    // If no English title, get the first available title
                    json_object_object_foreach(title_obj, key, val) {
                        (void)key;  // Add this line to suppress the warning
                        search_result->results[i].title = strdup(json_object_get_string(val));
                        break;
                    }
                }
            } else {
                search_result->results[i].title = strdup("Unknown");
            }
        } else {
            search_result->results[i].title = strdup("Unknown");
        }
        
        // Image URL would require another API call, so we'll set it to NULL for now
        search_result->results[i].image = NULL;
        
        // Set content type to manga
        search_result->results[i].content_type = CONTENT_MANGA;
        
        // For now, we don't know the chapter count without another API call
        search_result->results[i].episodes_or_chapters = 0;
    }
    
    // Clean up
    json_object_put(json_obj);
    free(chunk.data);
    curl_global_cleanup();
    
    return search_result;
}

MangadexMangaInfo* mangadex_get_manga_info(const char *manga_id) {
    // Implement manga info fetching
    // For now, return a placeholder
    fprintf(stderr, "mangadex_get_manga_info not fully implemented yet\n");
    
    MangadexMangaInfo *info = calloc(1, sizeof(MangadexMangaInfo));
    if (info) {
        info->id = strdup(manga_id);
        info->title = strdup("Manga information not yet implemented");
    }
    return info;
}

MangadexChapterPages* mangadex_get_chapter_pages(const char *chapter_id) {
    // Implement chapter pages fetching
    (void)chapter_id;  // Add this line to suppress the warning
    // For now, return a placeholder
    fprintf(stderr, "mangadex_get_chapter_pages not fully implemented yet\n");
    
    MangadexChapterPages *pages = calloc(1, sizeof(MangadexChapterPages));
    return pages;
}

void mangadex_free_search_results(SearchResult *results) {
    if (!results) return;
    
    for (int i = 0; i < results->total_results; i++) {
        free(results->results[i].id);
        free(results->results[i].title);
        free(results->results[i].image);
    }
    
    free(results->results);
    free(results);
}

void mangadex_free_manga_info(MangadexMangaInfo *info) {
    if (!info) return;
    
    free(info->id);
    free(info->title);
    free(info->url);
    free(info->image);
    free(info->description);
    free(info->release_date);
    free(info->status);
    
    if (info->genres) {
        for (int i = 0; i < info->genres_count; i++) {
            free(info->genres[i]);
        }
        free(info->genres);
    }
    
    if (info->chapters) {
        for (int i = 0; i < info->total_chapters; i++) {
            free(info->chapters[i].id);
            free(info->chapters[i].title);
            free(info->chapters[i].url);
        }
        free(info->chapters);
    }
    
    free(info);
}

void mangadex_free_chapter_pages(MangadexChapterPages *pages) {
    if (!pages) return;
    
    if (pages->page_urls) {
        for (int i = 0; i < pages->page_count; i++) {
            free(pages->page_urls[i]);
        }
        free(pages->page_urls);
    }
    
    free(pages);
}

// Provider API function mapping
static ProviderAPI mangadex_api = {
    .search = mangadex_search_manga,
    .free_search_results = mangadex_free_search_results,
    .get_anime_info = NULL,  // Manga provider doesn't support anime
    .get_episode_stream = NULL,
    .free_anime_info = NULL,
    .free_stream_info = NULL,
    // Manga functions
    .get_manga_info = (void* (*)(const char*))mangadex_get_manga_info,
    .get_chapter_pages = (void* (*)(const char*))mangadex_get_chapter_pages,
    .free_manga_info = (void (*)(void*))mangadex_free_manga_info,
    .free_chapter_info = (void (*)(void*))mangadex_free_chapter_pages
};

const ProviderAPI* mangadex_get_api() {
    return &mangadex_api;
}