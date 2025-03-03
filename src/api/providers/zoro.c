#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <json-c/json.h>
#include "zoro.h"
#include "../../utils/memory.h"

#define ZORO_API_BASE_URL "https://consumet.thuanc177.me/anime/zoro"

typedef struct {
    char *data;
    size_t size;
} MemoryStruct;

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, MemoryStruct *userp) {
    size_t realsize = size * nmemb;
    userp->data = realloc(userp->data, userp->size + realsize + 1);
    if (userp->data == NULL) {
        printf("Not enough memory (realloc returned NULL)\n");
        return 0;
    }
    memcpy(&(userp->data[userp->size]), contents, realsize);
    userp->size += realsize;
    userp->data[userp->size] = 0;
    return realsize;
}

SearchResult* zoro_search_anime(const char *query) {
    CURL *curl;
    CURLcode res;
    MemoryStruct chunk = {NULL, 0};
    char url[512];
    
    // Build URL for anime search endpoint
    snprintf(url, sizeof(url), "%s/%s?query=%s", ZORO_API_BASE_URL, "search", query);
    
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
    struct json_object *results_array;
    if (!json_object_object_get_ex(json_obj, "results", &results_array)) {
        fprintf(stderr, "No results field in JSON response\n");
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
    int num_results = json_object_array_length(results_array);
    search_result->total_results = num_results;
    search_result->results = malloc(num_results * sizeof(SearchResultItem));
    
    if (!search_result->results) {
        fprintf(stderr, "Failed to allocate memory for anime results\n");
        free(search_result);
        json_object_put(json_obj);
        free(chunk.data);
        curl_global_cleanup();
        return NULL;
    }
    
    // Extract each result
    for (int i = 0; i < num_results; i++) {
        struct json_object *result_obj = json_object_array_get_idx(results_array, i);
        struct json_object *field;
        
        if (json_object_object_get_ex(result_obj, "id", &field))
            search_result->results[i].id = strdup(json_object_get_string(field));
        else
            search_result->results[i].id = NULL;
        
        if (json_object_object_get_ex(result_obj, "title", &field))
            search_result->results[i].title = strdup(json_object_get_string(field));
        else
            search_result->results[i].title = NULL;
        
        if (json_object_object_get_ex(result_obj, "image", &field))
            search_result->results[i].image = strdup(json_object_get_string(field));
        else
            search_result->results[i].image = NULL;
        
        search_result->results[i].content_type = CONTENT_ANIME;
        
        if (json_object_object_get_ex(result_obj, "episodes", &field))
            search_result->results[i].episodes_or_chapters = json_object_get_int(field);
        else
            search_result->results[i].episodes_or_chapters = 0;
    }
    
    // Clean up
    json_object_put(json_obj);
    free(chunk.data);
    curl_global_cleanup();
    
    return search_result;
}

// Implement the rest of Zoro provider functions here
// ...

// Provider API function mapping
static ProviderAPI zoro_api = {
    .search = zoro_search_anime,
    .free_search_results = zoro_free_search_results,
    .get_anime_info = (void* (*)(const char*))zoro_get_anime_info,
    .get_episode_stream = (void* (*)(const char*, const char*))zoro_get_episode_stream,
    .free_anime_info = (void (*)(void*))zoro_free_anime_info,
    .free_stream_info = (void (*)(void*))zoro_free_stream_info,
    // Manga functions are not supported
    .get_manga_info = NULL,
    .get_chapter_pages = NULL,
    .free_manga_info = NULL,
    .free_chapter_info = NULL
};

const ProviderAPI* zoro_get_api() {
    return &zoro_api;
}

// Implementation of cleanup functions
void zoro_free_search_results(SearchResult *results) {
    if (!results) return;
    
    for (int i = 0; i < results->total_results; i++) {
        free(results->results[i].id);
        free(results->results[i].title);
        free(results->results[i].image);
    }
    
    free(results->results);
    free(results);
}

ZoroAnimeInfo* zoro_get_anime_info(const char *anime_id) {
    CURL *curl;
    CURLcode res;
    MemoryStruct chunk = {NULL, 0};
    char url[512];
    
    // Build URL for anime info endpoint
    snprintf(url, sizeof(url), "%s/info/%s", ZORO_API_BASE_URL, anime_id);
    
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
    
    // Create anime info structure
    ZoroAnimeInfo *info = calloc(1, sizeof(ZoroAnimeInfo));
    if (!info) {
        fprintf(stderr, "Failed to allocate memory for anime info\n");
        json_object_put(json_obj);
        free(chunk.data);
        curl_global_cleanup();
        return NULL;
    }
    
    // Extract anime details
    struct json_object *field;
    if (json_object_object_get_ex(json_obj, "id", &field))
        info->id = strdup(json_object_get_string(field));
    
    if (json_object_object_get_ex(json_obj, "title", &field))
        info->title = strdup(json_object_get_string(field));
        
    if (json_object_object_get_ex(json_obj, "url", &field))
        info->url = strdup(json_object_get_string(field));
        
    if (json_object_object_get_ex(json_obj, "image", &field))
        info->image = strdup(json_object_get_string(field));
        
    if (json_object_object_get_ex(json_obj, "description", &field))
        info->description = strdup(json_object_get_string(field));
        
    if (json_object_object_get_ex(json_obj, "releaseDate", &field))
        info->release_date = strdup(json_object_get_string(field));
        
    if (json_object_object_get_ex(json_obj, "status", &field))
        info->status = strdup(json_object_get_string(field));
    
    if (json_object_object_get_ex(json_obj, "type", &field))
        info->sub_or_dub = strdup(json_object_get_string(field));
    
    // Extract genres
    struct json_object *genres_array;
    if (json_object_object_get_ex(json_obj, "genres", &genres_array)) {
        int num_genres = json_object_array_length(genres_array);
        info->genres_count = num_genres;
        info->genres = calloc(num_genres, sizeof(char*));
        
        for (int i = 0; i < num_genres; i++) {
            struct json_object *genre_obj = json_object_array_get_idx(genres_array, i);
            info->genres[i] = strdup(json_object_get_string(genre_obj));
        }
    }
    
    // Extract episodes
    struct json_object *episodes_array;
    if (json_object_object_get_ex(json_obj, "episodes", &episodes_array)) {
        int num_episodes = json_object_array_length(episodes_array);
        info->total_episodes = num_episodes;
        info->episodes = calloc(num_episodes, sizeof(ZoroEpisode));
        
        for (int i = 0; i < num_episodes; i++) {
            struct json_object *episode_obj = json_object_array_get_idx(episodes_array, i);
            
            if (json_object_object_get_ex(episode_obj, "id", &field))
                info->episodes[i].id = strdup(json_object_get_string(field));
                
            if (json_object_object_get_ex(episode_obj, "number", &field))
                info->episodes[i].number = json_object_get_int(field);
                
            if (json_object_object_get_ex(episode_obj, "title", &field))
                info->episodes[i].title = strdup(json_object_get_string(field));
                
            if (json_object_object_get_ex(episode_obj, "url", &field))
                info->episodes[i].url = strdup(json_object_get_string(field));
        }
    }
    
    // Clean up
    json_object_put(json_obj);
    free(chunk.data);
    curl_global_cleanup();
    
    return info;
}

ZoroStreamInfo* zoro_get_episode_stream(const char *episode_id, const char *server) {
    CURL *curl;
    CURLcode res;
    MemoryStruct chunk = {NULL, 0};
    char url[512];
    
    // Build URL for episode streaming info endpoint
    snprintf(url, sizeof(url), "%s/watch/%s?server=%s", 
             ZORO_API_BASE_URL, episode_id, server);
    
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
    
    // Create stream info structure
    ZoroStreamInfo *info = calloc(1, sizeof(ZoroStreamInfo));
    if (!info) {
        fprintf(stderr, "Failed to allocate memory for stream info\n");
        json_object_put(json_obj);
        free(chunk.data);
        curl_global_cleanup();
        return NULL;
    }
    
    // Extract streaming details
    struct json_object *field;
    if (json_object_object_get_ex(json_obj, "headers", &field)) {
        struct json_object *referer;
        if (json_object_object_get_ex(field, "Referer", &referer))
            info->referer = strdup(json_object_get_string(referer));
        
        struct json_object *user_agent;
        if (json_object_object_get_ex(field, "User-Agent", &user_agent))
            info->user_agent = strdup(json_object_get_string(user_agent));
    }
    
    // Extract sources
    struct json_object *sources_array;
    if (json_object_object_get_ex(json_obj, "sources", &sources_array)) {
        int num_sources = json_object_array_length(sources_array);
        info->sources_count = num_sources;
        info->sources = calloc(num_sources, sizeof(ZoroSource));
        
        for (int i = 0; i < num_sources; i++) {
            struct json_object *source_obj = json_object_array_get_idx(sources_array, i);
            
            if (json_object_object_get_ex(source_obj, "url", &field))
                info->sources[i].url = strdup(json_object_get_string(field));
                
            if (json_object_object_get_ex(source_obj, "quality", &field))
                info->sources[i].quality = strdup(json_object_get_string(field));
                
            if (json_object_object_get_ex(source_obj, "isM3U8", &field))
                info->sources[i].is_m3u8 = json_object_get_boolean(field);
        }
    }
    
    // Extract subtitles
    struct json_object *subtitles_array;
    if (json_object_object_get_ex(json_obj, "subtitles", &subtitles_array)) {
        int num_subtitles = json_object_array_length(subtitles_array);
        info->subtitles_count = num_subtitles;
        info->subtitles = calloc(num_subtitles, sizeof(ZoroSubtitle));
        
        for (int i = 0; i < num_subtitles; i++) {
            struct json_object *subtitle_obj = json_object_array_get_idx(subtitles_array, i);
            
            if (json_object_object_get_ex(subtitle_obj, "url", &field))
                info->subtitles[i].url = strdup(json_object_get_string(field));
                
            if (json_object_object_get_ex(subtitle_obj, "lang", &field))
                info->subtitles[i].lang = strdup(json_object_get_string(field));
        }
    }
    
    // Clean up
    json_object_put(json_obj);
    free(chunk.data);
    curl_global_cleanup();
    
    return info;
}

void zoro_free_anime_info(ZoroAnimeInfo *info) {
    if (!info) return;
    
    free(info->id);
    free(info->title);
    free(info->url);
    free(info->image);
    free(info->description);
    free(info->release_date);
    free(info->status);
    free(info->sub_or_dub);
    
    if (info->genres) {
        for (int i = 0; i < info->genres_count; i++) {
            free(info->genres[i]);
        }
        free(info->genres);
    }
    
    if (info->episodes) {
        for (int i = 0; i < info->total_episodes; i++) {
            free(info->episodes[i].id);
            free(info->episodes[i].title);
            free(info->episodes[i].url);
        }
        free(info->episodes);
    }
    
    free(info);
}

void zoro_free_stream_info(ZoroStreamInfo *info) {
    if (!info) return;
    
    free(info->referer);
    free(info->user_agent);
    
    if (info->sources) {
        for (int i = 0; i < info->sources_count; i++) {
            free(info->sources[i].url);
            free(info->sources[i].quality);
        }
        free(info->sources);
    }
    
    if (info->subtitles) {
        for (int i = 0; i < info->subtitles_count; i++) {
            free(info->subtitles[i].url);
            free(info->subtitles[i].lang);
        }
        free(info->subtitles);
    }
    
    free(info);
}