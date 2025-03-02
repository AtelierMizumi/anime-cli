#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <json-c/json.h>
#include "api.h"

#define API_BASE_URL "https://consumet.thuanc177.me/anime/zoro"

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

SearchResult* api_search_anime(const char *query) {
    CURL *curl;
    CURLcode res;
    MemoryStruct chunk = {NULL, 0};
    char url[512];

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Failed to initialize curl\n");
        return NULL;
    }

    // URL encode the query (replace spaces with hyphens)
    char *temp_query = strdup(query);
    if (!temp_query) {
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        return NULL;
    }

    // Replace spaces with hyphens
    for (int i = 0; temp_query[i]; i++) {
        if (temp_query[i] == ' ')
            temp_query[i] = '-';
    }

    // Build the URL
    snprintf(url, sizeof(url), "%s/%s", API_BASE_URL, temp_query);
    free(temp_query); // Free the temporary query string

    // Set up curl options
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/122.0.0.0 Safari/537.36");
    
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

    // Extract results array from JSON
    struct json_object *results_array;
    if (!json_object_object_get_ex(json_obj, "results", &results_array)) {
        fprintf(stderr, "No results field in JSON response\n");
        json_object_put(json_obj);
        free(chunk.data);
        curl_global_cleanup();
        return NULL;
    }

    // Create search result structure
    SearchResult *search_result = (SearchResult *)malloc(sizeof(SearchResult));
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
    search_result->results = (AnimeResult *)malloc(num_results * sizeof(AnimeResult));
    
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
        struct json_object *id_obj, *title_obj, *image_obj, *type_obj;
        
        json_object_object_get_ex(result_obj, "id", &id_obj);
        json_object_object_get_ex(result_obj, "title", &title_obj);
        json_object_object_get_ex(result_obj, "image", &image_obj);
        json_object_object_get_ex(result_obj, "type", &type_obj);
        
        // Copy the values
        search_result->results[i].id = strdup(json_object_get_string(id_obj));
        search_result->results[i].title = strdup(json_object_get_string(title_obj));
        search_result->results[i].image = strdup(json_object_get_string(image_obj));
        search_result->results[i].type = strdup(json_object_get_string(type_obj));
        
        // Get episode count if available
        struct json_object *episodes_obj;
        if (json_object_object_get_ex(result_obj, "episodes", &episodes_obj)) {
            search_result->results[i].episodes = json_object_get_int(episodes_obj);
        } else {
            search_result->results[i].episodes = 0;
        }
    }

    // Clean up
    json_object_put(json_obj);
    free(chunk.data);
    curl_global_cleanup();

    return search_result;
}

AnimeInfo* api_get_anime_info(const char *anime_id) {
    CURL *curl;
    CURLcode res;
    MemoryStruct chunk = {NULL, 0};
    char url[512];

    // Build URL for anime info endpoint
    snprintf(url, sizeof(url), "%s/info?id=%s", API_BASE_URL, anime_id);

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
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/122.0.0.0 Safari/537.36");
    
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

    // Create AnimeInfo structure
    AnimeInfo *info = (AnimeInfo *)malloc(sizeof(AnimeInfo));
    if (!info) {
        fprintf(stderr, "Failed to allocate memory for anime info\n");
        json_object_put(json_obj);
        free(chunk.data);
        curl_global_cleanup();
        return NULL;
    }
    
    // Initialize all pointers to NULL for safety
    info->id = NULL;
    info->title = NULL;
    info->url = NULL;
    info->image = NULL;
    info->description = NULL;
    info->release_date = NULL;
    info->status = NULL;
    info->genres = NULL;
    info->genres_count = 0;
    info->sub_or_dub = NULL;
    info->total_episodes = 0;
    info->episodes = NULL;

    // Extract fields from JSON
    struct json_object *field;
    
    if (json_object_object_get_ex(json_obj, "id", &field))
        info->id = strdup(json_object_get_string(field));
        
    if (json_object_object_get_ex(json_obj, "title", &field))
        info->title = strdup(json_object_get_string(field));
        
    if (json_object_object_get_ex(json_obj, "url", &field))
        info->url = strdup(json_object_get_string(field));
        
    if (json_object_object_get_ex(json_obj, "image", &field))
        info->image = strdup(json_object_get_string(field));
        
    if (json_object_object_get_ex(json_obj, "description", &field) && 
        !json_object_is_type(field, json_type_null))
        info->description = strdup(json_object_get_string(field));
        
    if (json_object_object_get_ex(json_obj, "releaseDate", &field) && 
        !json_object_is_type(field, json_type_null))
        info->release_date = strdup(json_object_get_string(field));
        
    if (json_object_object_get_ex(json_obj, "status", &field))
        info->status = strdup(json_object_get_string(field));
        
    if (json_object_object_get_ex(json_obj, "subOrDub", &field))
        info->sub_or_dub = strdup(json_object_get_string(field));
    
    // Extract genres array
    struct json_object *genres_array;
    if (json_object_object_get_ex(json_obj, "genres", &genres_array) && 
        json_object_is_type(genres_array, json_type_array)) {
        
        info->genres_count = json_object_array_length(genres_array);
        info->genres = malloc(info->genres_count * sizeof(char *));
        
        if (info->genres) {
            for (int i = 0; i < info->genres_count; i++) {
                struct json_object *genre = json_object_array_get_idx(genres_array, i);
                info->genres[i] = strdup(json_object_get_string(genre));
            }
        }
    }
    
    // Extract episodes array
    if (json_object_object_get_ex(json_obj, "totalEpisodes", &field))
        info->total_episodes = json_object_get_int(field);
    
    struct json_object *episodes_array;
    if (json_object_object_get_ex(json_obj, "episodes", &episodes_array) && 
        json_object_is_type(episodes_array, json_type_array)) {
        
        int episodes_count = json_object_array_length(episodes_array);
        info->total_episodes = episodes_count; // Update count from actual array length
        
        info->episodes = malloc(episodes_count * sizeof(Episode));
        if (info->episodes) {
            for (int i = 0; i < episodes_count; i++) {
                struct json_object *episode_obj = json_object_array_get_idx(episodes_array, i);
                struct json_object *ep_field;
                
                if (json_object_object_get_ex(episode_obj, "id", &ep_field))
                    info->episodes[i].id = strdup(json_object_get_string(ep_field));
                else
                    info->episodes[i].id = NULL;
                
                if (json_object_object_get_ex(episode_obj, "number", &ep_field))
                    info->episodes[i].number = json_object_get_int(ep_field);
                else
                    info->episodes[i].number = i + 1; // Default to index + 1
                
                if (json_object_object_get_ex(episode_obj, "title", &ep_field) &&
                    !json_object_is_type(ep_field, json_type_null))
                    info->episodes[i].title = strdup(json_object_get_string(ep_field));
                else
                    info->episodes[i].title = NULL;
                
                if (json_object_object_get_ex(episode_obj, "url", &ep_field))
                    info->episodes[i].url = strdup(json_object_get_string(ep_field));
                else
                    info->episodes[i].url = NULL;
            }
        }
    }
    
    // Clean up
    json_object_put(json_obj);
    free(chunk.data);
    curl_global_cleanup();
    
    return info;
}

// Implement the streaming endpoint
StreamInfo* api_get_episode_stream(const char *episode_id, const char *server) {
    CURL *curl;
    CURLcode res;
    MemoryStruct chunk = {NULL, 0};
    char url[512];

    // Build URL for streaming endpoint
    snprintf(url, sizeof(url), "%s/watch?episodeId=%s&server=%s", 
             API_BASE_URL, episode_id, server ? server : "vidstreaming");

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
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/122.0.0.0 Safari/537.36");
    
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

    // Create StreamInfo structure
    StreamInfo *info = (StreamInfo *)malloc(sizeof(StreamInfo));
    if (!info) {
        fprintf(stderr, "Failed to allocate memory for stream info\n");
        json_object_put(json_obj);
        free(chunk.data);
        curl_global_cleanup();
        return NULL;
    }
    
    // Initialize subtitles
    info->subtitles = NULL;
    info->subtitles_count = 0;

    // Extract subtitles array
    struct json_object *subtitles_array;
    if (json_object_object_get_ex(json_obj, "subtitles", &subtitles_array) && 
        json_object_is_type(subtitles_array, json_type_array)) {
        
        info->subtitles_count = json_object_array_length(subtitles_array);
        info->subtitles = malloc(info->subtitles_count * sizeof(Subtitle));
        
        if (info->subtitles) {
            for (int i = 0; i < info->subtitles_count; i++) {
                struct json_object *subtitle_obj = json_object_array_get_idx(subtitles_array, i);
                struct json_object *sub_field;
                
                if (json_object_object_get_ex(subtitle_obj, "url", &sub_field))
                    info->subtitles[i].url = strdup(json_object_get_string(sub_field));
                else
                    info->subtitles[i].url = NULL;
                
                if (json_object_object_get_ex(subtitle_obj, "lang", &sub_field))
                    info->subtitles[i].lang = strdup(json_object_get_string(sub_field));
                else
                    info->subtitles[i].lang = NULL;
            }
        }
    }

    // Initialize all pointers to NULL for safety
    info->referer = NULL;
    info->user_agent = NULL;
    info->sources = NULL;
    info->sources_count = 0;

    // Extract headers
    struct json_object *headers_obj;
    if (json_object_object_get_ex(json_obj, "headers", &headers_obj)) {
        struct json_object *field;
        
        if (json_object_object_get_ex(headers_obj, "Referer", &field))
            info->referer = strdup(json_object_get_string(field));
            
        if (json_object_object_get_ex(headers_obj, "User-Agent", &field))
            info->user_agent = strdup(json_object_get_string(field));
    }
    
    // Extract sources array
    struct json_object *sources_array;
    if (json_object_object_get_ex(json_obj, "sources", &sources_array) && 
        json_object_is_type(sources_array, json_type_array)) {
        
        info->sources_count = json_object_array_length(sources_array);
        info->sources = malloc(info->sources_count * sizeof(Source));
        
        if (info->sources) {
            for (int i = 0; i < info->sources_count; i++) {
                struct json_object *source_obj = json_object_array_get_idx(sources_array, i);
                struct json_object *src_field;
                
                if (json_object_object_get_ex(source_obj, "url", &src_field))
                    info->sources[i].url = strdup(json_object_get_string(src_field));
                else
                    info->sources[i].url = NULL;
                
                if (json_object_object_get_ex(source_obj, "quality", &src_field))
                    info->sources[i].quality = strdup(json_object_get_string(src_field));
                else
                    info->sources[i].quality = NULL;
                
                if (json_object_object_get_ex(source_obj, "isM3U8", &src_field))
                    info->sources[i].is_m3u8 = json_object_get_boolean(src_field);
                else
                    info->sources[i].is_m3u8 = 0;
            }
        }
    }
    
    // Clean up
    json_object_put(json_obj);
    free(chunk.data);
    curl_global_cleanup();
    
    return info;
}

// Implement cleanup functions
void api_free_anime_info(AnimeInfo *info) {
    if (!info) return;
    
    free(info->id);
    free(info->title);
    free(info->url);
    free(info->image);
    free(info->description);
    free(info->release_date);
    free(info->status);
    free(info->sub_or_dub);
    
    if (info->genres && info->genres_count > 0) {
        for (int i = 0; i < info->genres_count; i++) {
            free(info->genres[i]);
        }
        free(info->genres);
    }
    
    if (info->episodes && info->total_episodes > 0) {
        for (int i = 0; i < info->total_episodes; i++) {
            free(info->episodes[i].id);
            free(info->episodes[i].title);
            free(info->episodes[i].url);
        }
        free(info->episodes);
    }
    
    free(info);
}

void api_free_stream_info(StreamInfo *info) {
    if (!info) return;
    
    free(info->referer);
    free(info->user_agent);
    
    if (info->sources && info->sources_count > 0) {
        for (int i = 0; i < info->sources_count; i++) {
            free(info->sources[i].url);
            free(info->sources[i].quality);
        }
        free(info->sources);
    }
    
    if (info->subtitles && info->subtitles_count > 0) {
        for (int i = 0; i < info->subtitles_count; i++) {
            free(info->subtitles[i].url);
            free(info->subtitles[i].lang);
        }
        free(info->subtitles);
    }
    
    free(info);
}

void api_free_search_results(SearchResult *results) {
    if (!results) return;
    
    for (int i = 0; i < results->total_results; i++) {
        free(results->results[i].id);
        free(results->results[i].title);
        free(results->results[i].image);
        free(results->results[i].type);
    }
    
    free(results->results);
    free(results);
}