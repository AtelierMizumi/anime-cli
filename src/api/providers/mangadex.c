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
    
    // URL encode the query
    CURL *curl_handle = curl_easy_init();
    if (!curl_handle) {
        fprintf(stderr, "Failed to initialize curl for URL encoding\n");
        return NULL;
    }
    
    char *encoded_query = curl_easy_escape(curl_handle, query, 0);
    curl_easy_cleanup(curl_handle);
    
    // Build URL for manga search endpoint
    snprintf(url, sizeof(url), "%s/%s", 
             MANGADEX_API_BASE_URL, encoded_query);
    fprintf(stderr, "DEBUG: Requesting URL: %s\n", url);
    
    curl_free(encoded_query);
    
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
    
    // Create search result structure
    SearchResult *search_result = malloc(sizeof(SearchResult));
    if (!search_result) {
        fprintf(stderr, "Failed to allocate memory for search results\n");
        json_object_put(json_obj);
        free(chunk.data);
        curl_global_cleanup();
        return NULL;
    }
    
    // Initialize search result
    search_result->total_results = 0;
    search_result->results = NULL;
    
    // Extract results array
    struct json_object *results_array;
    if (!json_object_object_get_ex(json_obj, "results", &results_array)) {
        fprintf(stderr, "No results field in JSON response\n");
        free(search_result);
        json_object_put(json_obj);
        free(chunk.data);
        curl_global_cleanup();
        return NULL;
    }
    
    // Get array length
    int num_results = json_object_array_length(results_array);
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
        struct json_object *manga_obj = json_object_array_get_idx(results_array, i);
        struct json_object *field;
        
        // Initialize item
        search_result->results[i].id = NULL;
        search_result->results[i].title = NULL;
        search_result->results[i].image = NULL;
        search_result->results[i].content_type = CONTENT_MANGA;
        search_result->results[i].episodes_or_chapters = 0;
        
        // Get manga ID
        if (json_object_object_get_ex(manga_obj, "id", &field))
            search_result->results[i].id = strdup(json_object_get_string(field));
        
        // Get title
        if (json_object_object_get_ex(manga_obj, "title", &field))
            search_result->results[i].title = strdup(json_object_get_string(field));
        
        // Get image
        if (json_object_object_get_ex(manga_obj, "image", &field))
            search_result->results[i].image = strdup(json_object_get_string(field));
        
        // Try to extract chapter count from lastChapter field
    if (json_object_object_get_ex(manga_obj, "lastChapter", &field)) {
        const char *lastChapter = json_object_get_string(field);
        if (lastChapter && *lastChapter) {  // Make sure it's not NULL or empty string
            search_result->results[i].episodes_or_chapters = atoi(lastChapter);
        }
    }

    // If lastChapter is empty or not found, check for other fields
    if (search_result->results[i].episodes_or_chapters == 0) {
        // Try lastVolume field
        if (json_object_object_get_ex(manga_obj, "lastVolume", &field)) {
            const char *lastVolume = json_object_get_string(field);
            if (lastVolume && *lastVolume) {
                // Just indicate that volumes exist
                search_result->results[i].episodes_or_chapters = 1;
            }
        }
    }

    // If we still don't have chapter info, check status
    if (search_result->results[i].episodes_or_chapters == 0) {
        // For completed manga with no chapter count, show at least 1
        struct json_object *status_field;
        if (json_object_object_get_ex(manga_obj, "status", &status_field)) {
            const char *status = json_object_get_string(status_field);
            if (status && (strcmp(status, "completed") == 0 || strcmp(status, "finished") == 0)) {
                search_result->results[i].episodes_or_chapters = 1;
            }
        }
    }
    }
    
    // Clean up
    json_object_put(json_obj);
    free(chunk.data);
    curl_global_cleanup();
    
    return search_result;
}

MangadexMangaInfo* mangadex_get_manga_info(const char *manga_id) {
    CURL *curl;
    CURLcode res;
    MemoryStruct chunk = {NULL, 0};
    char url[512];
    
    // Build URL for manga info endpoint - UPDATED FORMAT
    snprintf(url, sizeof(url), "%s/info/%s", 
             MANGADEX_API_BASE_URL, manga_id);
    fprintf(stderr, "DEBUG: Requesting URL: %s\n", url);
    
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
    
    // Add timeout to prevent hanging
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L); // 15 seconds timeout
    
    // Perform the request
    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    
    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        free(chunk.data);
        curl_global_cleanup();
        return NULL;
    }
    
    // Debug response
    fprintf(stderr, "Response size: %zu bytes\n", chunk.size);
    if (chunk.size > 0) {
        // Print first 200 characters for debug
        char preview[201] = {0};
        strncpy(preview, chunk.data, 200);
        fprintf(stderr, "Response preview: %s\n", preview);
    }
    
    // Parse JSON response
    struct json_object *json_obj = json_tokener_parse(chunk.data);
    if (!json_obj) {
        fprintf(stderr, "Failed to parse JSON response\n");
        free(chunk.data);
        curl_global_cleanup();
        return NULL;
    }
    
    // Create manga info structure
    MangadexMangaInfo *info = calloc(1, sizeof(MangadexMangaInfo));
    if (!info) {
        fprintf(stderr, "Failed to allocate memory for manga info\n");
        json_object_put(json_obj);
        free(chunk.data);
        curl_global_cleanup();
        return NULL;
    }
    
    // Initialize all pointers to NULL
    info->id = NULL;
    info->title = NULL;
    info->url = NULL;
    info->image = NULL;
    info->description = NULL;
    info->release_date = NULL;
    info->status = NULL;
    info->genres = NULL;
    info->genres_count = 0;
    info->chapters = NULL;
    info->total_chapters = 0;
    
    // The response is now directly a manga object, not wrapped in a results array
    struct json_object *field;
    
    // Get manga ID
    if (json_object_object_get_ex(json_obj, "id", &field) && field)
        info->id = strdup(json_object_get_string(field));
    else
        info->id = strdup(manga_id); // Fallback to requested ID
    
    // Get title
    if (json_object_object_get_ex(json_obj, "title", &field) && field)
        info->title = strdup(json_object_get_string(field));
    else
        info->title = strdup("Unknown Title");
    
    // Get image
    if (json_object_object_get_ex(json_obj, "image", &field) && field)
        info->image = strdup(json_object_get_string(field));
    
    // Get description - Handle description as an object with language keys
    struct json_object *desc_obj;
    if (json_object_object_get_ex(json_obj, "description", &desc_obj)) {
        if (json_object_is_type(desc_obj, json_type_object)) {
            // Try to get English description first
            struct json_object *en_desc;
            if (json_object_object_get_ex(desc_obj, "en", &en_desc)) {
                info->description = strdup(json_object_get_string(en_desc));
            } else {
                // If no English description, use the first available language
                json_object_object_foreach(desc_obj, key, val) {
                    info->description = strdup(json_object_get_string(val));
                    break;
                }
            }
        } else if (json_object_is_type(desc_obj, json_type_string)) {
            // If description is directly a string
            info->description = strdup(json_object_get_string(desc_obj));
        }
    }
    
    // Get release date
    if (json_object_object_get_ex(json_obj, "releaseDate", &field) && field)
        info->release_date = strdup(json_object_get_string(field));
    
    // Get status
    if (json_object_object_get_ex(json_obj, "status", &field) && field)
        info->status = strdup(json_object_get_string(field));
    
    // Extract genres
    struct json_object *genres_array;
    if (json_object_object_get_ex(json_obj, "genres", &genres_array) && 
        genres_array && json_object_is_type(genres_array, json_type_array)) {
        
        info->genres_count = json_object_array_length(genres_array);
        if (info->genres_count > 0) {
            info->genres = calloc(info->genres_count, sizeof(char*));
            if (info->genres) {
                for (int i = 0; i < info->genres_count; i++) {
                    struct json_object *genre = json_object_array_get_idx(genres_array, i);
                    if (genre)
                        info->genres[i] = strdup(json_object_get_string(genre));
                }
            } else {
                info->genres_count = 0;
            }
        }
    }
    
    // Extract chapters - format has changed in the new API response
    struct json_object *chapters_array;
    if (json_object_object_get_ex(json_obj, "chapters", &chapters_array) && 
        chapters_array && json_object_is_type(chapters_array, json_type_array)) {
        
        info->total_chapters = json_object_array_length(chapters_array);
        fprintf(stderr, "Found %d chapters for manga\n", info->total_chapters);
        
        if (info->total_chapters > 0) {
            info->chapters = calloc(info->total_chapters, sizeof(MangadexChapter));
            if (info->chapters) {
                for (int i = 0; i < info->total_chapters; i++) {
                    struct json_object *chapter = json_object_array_get_idx(chapters_array, i);
                    if (!chapter) continue;
                    
                    // Initialize chapter data
                    info->chapters[i].id = NULL;
                    info->chapters[i].title = NULL;
                    info->chapters[i].releaseDate = NULL;
                    info->chapters[i].number = i + 1;  // Default to index + 1
                    
                    // Get chapter ID - this is important for reading
                    if (json_object_object_get_ex(chapter, "id", &field) && field)
                        info->chapters[i].id = strdup(json_object_get_string(field));
                    
                    // Get chapter title
                    if (json_object_object_get_ex(chapter, "title", &field) && field && 
                        !json_object_is_type(field, json_type_null)) {
                        info->chapters[i].title = strdup(json_object_get_string(field));
                    } else {
                        // If title is null, create a default title
                        char default_title[32];
                        snprintf(default_title, sizeof(default_title), "Chapter %d", i + 1);
                        info->chapters[i].title = strdup(default_title);
                    }
                    
                    // Get chapter number
                    if (json_object_object_get_ex(chapter, "chapterNumber", &field) && field && 
                        !json_object_is_type(field, json_type_null)) {
                        const char *chapterNumStr = json_object_get_string(field);
                        if (chapterNumStr)
                            info->chapters[i].number = atof(chapterNumStr); // Using atof for decimal chapter numbers
                    }
                }
            } else {
                fprintf(stderr, "Failed to allocate memory for chapters\n");
                info->total_chapters = 0;
            }
        } else {
            fprintf(stderr, "No chapters found in array\n");
        }
    } else {
        fprintf(stderr, "No chapters field found in manga object or not an array\n");
    }
    
    // Clean up
    json_object_put(json_obj);
    free(chunk.data);
    curl_global_cleanup();
    
    return info;
}

MangadexChapterPages* mangadex_get_chapter_pages(const char *chapter_id) {
    CURL *curl;
    CURLcode res;
    MemoryStruct chunk = {NULL, 0};
    char url[512];
    
    // Build URL for chapter pages endpoint
    snprintf(url, sizeof(url), "%s/read/%s", 
             MANGADEX_API_BASE_URL, chapter_id);
    fprintf(stderr, "DEBUG: Requesting URL: %s\n", url);
    
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
    struct json_object *json_array = json_tokener_parse(chunk.data);
    if (!json_array || !json_object_is_type(json_array, json_type_array)) {
        fprintf(stderr, "Failed to parse JSON response or not an array\n");
        if (json_array) json_object_put(json_array);
        free(chunk.data);
        curl_global_cleanup();
        return NULL;
    }
    
    // Create chapter pages structure
    MangadexChapterPages *pages = calloc(1, sizeof(MangadexChapterPages));
    if (!pages) {
        fprintf(stderr, "Failed to allocate memory for chapter pages\n");
        json_object_put(json_array);
        free(chunk.data);
        curl_global_cleanup();
        return NULL;
    }
    
    // Get array length
    int num_pages = json_object_array_length(json_array);
    pages->page_count = num_pages;
    pages->page_urls = calloc(num_pages, sizeof(char*));
    
    if (!pages->page_urls) {
        fprintf(stderr, "Failed to allocate memory for page URLs\n");
        free(pages);
        json_object_put(json_array);
        free(chunk.data);
        curl_global_cleanup();
        return NULL;
    }
    
    // Extract pages
    for (int i = 0; i < num_pages; i++) {
        struct json_object *page_obj = json_object_array_get_idx(json_array, i);
        struct json_object *img_field;
        
        if (json_object_object_get_ex(page_obj, "img", &img_field)) {
            pages->page_urls[i] = strdup(json_object_get_string(img_field));
        }
        
        // Get the referer header if available (for the first page is enough)
        if (i == 0 && !pages->referer) {
            struct json_object *header_field;
            if (json_object_object_get_ex(page_obj, "headerForImage", &header_field)) {
                struct json_object *referer;
                if (json_object_is_type(header_field, json_type_object) && 
                    json_object_object_get_ex(header_field, "Referer", &referer)) {
                    pages->referer = strdup(json_object_get_string(referer));
                }
            }
        }
    }
    
    // Clean up
    json_object_put(json_array);
    free(chunk.data);
    curl_global_cleanup();
    
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
    
    if (info->id) free(info->id);
    if (info->title) free(info->title);
    if (info->url) free(info->url);
    if (info->image) free(info->image);
    if (info->description) free(info->description);
    if (info->release_date) free(info->release_date);
    if (info->status) free(info->status);
    
    if (info->genres) {
        for (int i = 0; i < info->genres_count; i++) {
            if (info->genres[i]) free(info->genres[i]);
        }
        free(info->genres);
    }
    
    if (info->chapters) {
        for (int i = 0; i < info->total_chapters; i++) {
            if (info->chapters[i].id) free(info->chapters[i].id);
            if (info->chapters[i].title) free(info->chapters[i].title);
            if (info->chapters[i].releaseDate) free(info->chapters[i].releaseDate);
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
    
    free(pages->referer);
    free(pages);
}

// Provider API function mapping
static ProviderAPI mangadex_api = {
    .search = mangadex_search_manga,
    .free_search_results = mangadex_free_search_results,
    .get_anime_info = NULL, // Not supported for manga provider
    .get_episode_stream = NULL, // Not supported for manga provider
    .free_anime_info = NULL, // Not supported for manga provider
    .free_stream_info = NULL, // Not supported for manga provider
    // Manga functions
    .get_manga_info = (void* (*)(const char*))mangadex_get_manga_info,
    .get_chapter_pages = (void* (*)(const char*))mangadex_get_chapter_pages,
    .free_manga_info = (void (*)(void*))mangadex_free_manga_info,
    .free_chapter_info = (void (*)(void*))mangadex_free_chapter_pages
};

const ProviderAPI* mangadex_get_api() {
    return &mangadex_api;
}