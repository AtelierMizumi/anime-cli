#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <json-c/json.h>
#include "aniwatch.h"
#include "../../config.h"
#include "../../utils/memory.h"

#define ANIWATCH_API_BASE_URL "https://aniwatch-api-2.thuanc177.me"

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

SearchResult* aniwatch_search_anime(const char *query) {
    CURL *curl;
    CURLcode res;
    MemoryStruct chunk = {NULL, 0};
    char url[512];
    
    // Build URL for anime search endpoint
    snprintf(url, sizeof(url), "%s/api/v2/hianime/search?q=%s", ANIWATCH_API_BASE_URL, query);
    
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
    
    // Check if the response was successful
    struct json_object *success_obj;
    if (!json_object_object_get_ex(json_obj, "success", &success_obj) || 
        !json_object_get_boolean(success_obj)) {
        fprintf(stderr, "API returned unsuccessful response\n");
        json_object_put(json_obj);
        free(chunk.data);
        curl_global_cleanup();
        return NULL;
    }
    
    // Get data object
    struct json_object *data_obj;
    if (!json_object_object_get_ex(json_obj, "data", &data_obj)) {
        fprintf(stderr, "No data field in response\n");
        json_object_put(json_obj);
        free(chunk.data);
        curl_global_cleanup();
        return NULL;
    }
    
    // Get animes array
    struct json_object *animes_array;
    if (!json_object_object_get_ex(data_obj, "animes", &animes_array)) {
        fprintf(stderr, "No animes field in data\n");
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
    int num_results = json_object_array_length(animes_array);
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
        struct json_object *result_obj = json_object_array_get_idx(animes_array, i);
        struct json_object *field;
        
        if (json_object_object_get_ex(result_obj, "id", &field))
            search_result->results[i].id = safe_strdup(json_object_get_string(field));
        else
            search_result->results[i].id = NULL;
        
        if (json_object_object_get_ex(result_obj, "name", &field))
            search_result->results[i].title = safe_strdup(json_object_get_string(field));
        else
            search_result->results[i].title = NULL;
        
        if (json_object_object_get_ex(result_obj, "poster", &field))
            search_result->results[i].image = safe_strdup(json_object_get_string(field));
        else
            search_result->results[i].image = NULL;
        
        search_result->results[i].content_type = CONTENT_ANIME;
        
        // Extract episodes count - AniWatch provides sub and dub counts
        struct json_object *episodes_obj;
        if (json_object_object_get_ex(result_obj, "episodes", &episodes_obj)) {
            struct json_object *sub_obj, *dub_obj;
            int sub_count = 0, dub_count = 0;
            
            if (json_object_object_get_ex(episodes_obj, "sub", &sub_obj))
                sub_count = json_object_get_int(sub_obj);
                
            if (json_object_object_get_ex(episodes_obj, "dub", &dub_obj))
                dub_count = json_object_get_int(dub_obj);
            
            // Use the maximum of sub and dub counts to avoid double-counting
            search_result->results[i].episodes_or_chapters = (sub_count > dub_count) ? sub_count : dub_count;
        } else {
            search_result->results[i].episodes_or_chapters = 0;
        }
    }
    
    // Clean up
    json_object_put(json_obj);
    free(chunk.data);
    curl_global_cleanup();
    
    return search_result;
}

AnimeInfo* aniwatch_get_anime_info(const char *anime_id) {
    CURL *curl;
    CURLcode res;
    MemoryStruct chunk = {NULL, 0};
    char url[512];
    
    // Build URL for anime episodes endpoint
    snprintf(url, sizeof(url), "%s/api/v2/hianime/anime/%s/episodes", ANIWATCH_API_BASE_URL, anime_id);
    
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
    
    // Check if the response was successful
    struct json_object *success_obj;
    if (!json_object_object_get_ex(json_obj, "success", &success_obj) || 
        !json_object_get_boolean(success_obj)) {
        fprintf(stderr, "API returned unsuccessful response\n");
        json_object_put(json_obj);
        free(chunk.data);
        curl_global_cleanup();
        return NULL;
    }
    
    // Get data object
    struct json_object *data_obj;
    if (!json_object_object_get_ex(json_obj, "data", &data_obj)) {
        fprintf(stderr, "No data field in response\n");
        json_object_put(json_obj);
        free(chunk.data);
        curl_global_cleanup();
        return NULL;
    }
    
    // Create anime info structure
    AnimeInfo *info = calloc(1, sizeof(AnimeInfo));
    if (!info) {
        fprintf(stderr, "Failed to allocate memory for anime info\n");
        json_object_put(json_obj);
        free(chunk.data);
        curl_global_cleanup();
        return NULL;
    }
    
    // Set basic anime info
    info->id = safe_strdup(anime_id);
    info->title = safe_strdup(anime_id); // Use anime_id as title since we don't have title in the episodes endpoint
    
    // Extract total episodes
    struct json_object *total_episodes_obj;
    if (json_object_object_get_ex(data_obj, "totalEpisodes", &total_episodes_obj)) {
        info->total_episodes = json_object_get_int(total_episodes_obj);
    } else {
        info->total_episodes = 0;
    }
    
    // Extract episodes array
    struct json_object *episodes_array;
    if (json_object_object_get_ex(data_obj, "episodes", &episodes_array)) {
        int num_episodes = json_object_array_length(episodes_array);
        info->episodes = calloc(num_episodes, sizeof(Episode));
        
        if (!info->episodes) {
            fprintf(stderr, "Failed to allocate memory for episodes\n");
            free(info->id);
            free(info->title);
            free(info);
            json_object_put(json_obj);
            free(chunk.data);
            curl_global_cleanup();
            return NULL;
        }
        
        for (int i = 0; i < num_episodes; i++) {
            struct json_object *episode_obj = json_object_array_get_idx(episodes_array, i);
            struct json_object *field;
            
            if (json_object_object_get_ex(episode_obj, "episodeId", &field))
                info->episodes[i].id = safe_strdup(json_object_get_string(field));
            else
                info->episodes[i].id = NULL;
                
            if (json_object_object_get_ex(episode_obj, "number", &field))
                info->episodes[i].number = json_object_get_int(field);
            else
                info->episodes[i].number = i + 1;
                
            if (json_object_object_get_ex(episode_obj, "title", &field))
                info->episodes[i].title = safe_strdup(json_object_get_string(field));
            else
                info->episodes[i].title = NULL;
        }
    }
    
    // Clean up
    json_object_put(json_obj);
    free(chunk.data);
    curl_global_cleanup();
    
    return info;
}

StreamInfo* aniwatch_get_episode_stream(const char *episode_id, const char *server) {
    CURL *curl;
    CURLcode res;
    MemoryStruct chunk = {NULL, 0};
    char url[512];
    char *encoded_id = NULL;
    
    // Use default server if none provided
    if (!server) server = "hd-1";
    
    // URL encode the episode_id to handle special characters like "?"
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Failed to initialize curl\n");
        return NULL;
    }
    
    // Properly encode the episode ID
    encoded_id = curl_easy_escape(curl, episode_id, 0);
    if (!encoded_id) {
        fprintf(stderr, "Failed to URL-encode episode ID\n");
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        return NULL;
    }
    
    // Build URL for episode streaming info endpoint
    snprintf(url, sizeof(url), "%s/api/v2/hianime/episode/sources?animeEpisodeId=%s&server=%s&category=sub", 
             ANIWATCH_API_BASE_URL, encoded_id, server);
    
    fprintf(stderr, "DEBUG: Requesting URL: %s\n", url);
    
    // Set up curl options
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0");
    
    // Perform the request
    res = curl_easy_perform(curl);
    curl_free(encoded_id); // Free encoded ID
    
    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        curl_easy_cleanup(curl);
        free(chunk.data);
        curl_global_cleanup();
        return NULL;
    }
    
    curl_easy_cleanup(curl);
    
    // Print the first part of the response for debugging
    fprintf(stderr, "DEBUG: Response start: %.100s...\n", chunk.data);
    
    // Parse JSON response
    struct json_object *json_obj = json_tokener_parse(chunk.data);
    if (!json_obj) {
        fprintf(stderr, "Failed to parse JSON response\n");
        free(chunk.data);
        curl_global_cleanup();
        return NULL;
    }
    
    // Check if the response was successful
    struct json_object *success_obj;
    if (!json_object_object_get_ex(json_obj, "success", &success_obj) || 
        !json_object_get_boolean(success_obj)) {
        fprintf(stderr, "API returned unsuccessful response\n");
        json_object_put(json_obj);
        free(chunk.data);
        curl_global_cleanup();
        return NULL;
    }
    
    // Get data object
    struct json_object *data_obj;
    if (!json_object_object_get_ex(json_obj, "data", &data_obj)) {
        fprintf(stderr, "No data field in response\n");
        json_object_put(json_obj);
        free(chunk.data);
        curl_global_cleanup();
        return NULL;
    }
    
    // Create stream info structure
    StreamInfo *stream_info = calloc(1, sizeof(StreamInfo));
    if (!stream_info) {
        fprintf(stderr, "Failed to allocate memory for stream info\n");
        json_object_put(json_obj);
        free(chunk.data);
        curl_global_cleanup();
        return NULL;
    }
    
    // Extract sources array
    struct json_object *sources_array;
    if (!json_object_object_get_ex(data_obj, "sources", &sources_array)) {
        fprintf(stderr, "No sources field in data\n");
        free(stream_info);
        json_object_put(json_obj);
        free(chunk.data);
        curl_global_cleanup();
        return NULL;
    }
    
    // Get number of sources
    int num_sources = json_object_array_length(sources_array);
    if (num_sources <= 0) {
        fprintf(stderr, "No streaming sources available\n");
        free(stream_info);
        json_object_put(json_obj);
        free(chunk.data);
        curl_global_cleanup();
        return NULL;
    }
    
    fprintf(stderr, "DEBUG: Found %d sources\n", num_sources);
    
    stream_info->sources_count = num_sources;
    stream_info->sources = calloc(num_sources, sizeof(StreamSource));
    if (!stream_info->sources) {
        fprintf(stderr, "Memory allocation failed\n");
        free(stream_info);
        json_object_put(json_obj);
        free(chunk.data);
        curl_global_cleanup();
        return NULL;
    }
    
    // Fill sources from the API response
    for (int i = 0; i < num_sources; i++) {
        struct json_object *source_obj = json_object_array_get_idx(sources_array, i);
        struct json_object *url_obj, *type_obj;
        
        if (json_object_object_get_ex(source_obj, "url", &url_obj)) {
            const char *url_str = json_object_get_string(url_obj);
            stream_info->sources[i].url = safe_strdup(url_str);
            fprintf(stderr, "DEBUG: Source %d URL: %s\n", i, url_str);
        }
        
        // Use "type" field instead of "quality"
        if (json_object_object_get_ex(source_obj, "type", &type_obj)) {
            stream_info->sources[i].quality = safe_strdup(json_object_get_string(type_obj));
        }
        
        // Set is_m3u8 to true if type is "hls" or if URL ends with .m3u8
        if (stream_info->sources[i].url) {
            if (stream_info->sources[i].quality && strcmp(stream_info->sources[i].quality, "hls") == 0) {
                stream_info->sources[i].is_m3u8 = 1;
            } else {
                size_t url_len = strlen(stream_info->sources[i].url);
                if (url_len > 5 && strcmp(stream_info->sources[i].url + url_len - 5, ".m3u8") == 0) {
                    stream_info->sources[i].is_m3u8 = 1;
                }
            }
        }
    }
    
    // No headers in this API response, but set some defaults for playback compatibility
    stream_info->referer = safe_strdup("https://aniwatch.to/");
    stream_info->user_agent = safe_strdup("Mozilla/5.0");
    
    // Get subtitles from "tracks" array instead of "subtitles"
    struct json_object *tracks_array;
    if (json_object_object_get_ex(data_obj, "tracks", &tracks_array)) {
        int num_tracks = json_object_array_length(tracks_array);
        fprintf(stderr, "DEBUG: Found %d tracks\n", num_tracks);
        
        // Count actual subtitles (non-thumbnail tracks)
        int num_subtitles = 0;
        for (int i = 0; i < num_tracks; i++) {
            struct json_object *track_obj = json_object_array_get_idx(tracks_array, i);
            struct json_object *kind_obj;
            
            if (json_object_object_get_ex(track_obj, "kind", &kind_obj)) {
                const char *kind = json_object_get_string(kind_obj);
                if (kind && strcmp(kind, "thumbnails") != 0) {
                    num_subtitles++;
                }
            }
        }
        
        fprintf(stderr, "DEBUG: Found %d subtitle tracks\n", num_subtitles);
        
        if (num_subtitles > 0) {
            stream_info->subtitles_count = num_subtitles;
            stream_info->subtitles = calloc(num_subtitles, sizeof(Subtitle));
            
            if (!stream_info->subtitles) {
                fprintf(stderr, "Memory allocation for subtitles failed\n");
                // Continue without subtitles rather than failing completely
                stream_info->subtitles_count = 0;
            } else {
                int subtitle_index = 0;
                for (int i = 0; i < num_tracks; i++) {
                    struct json_object *track_obj = json_object_array_get_idx(tracks_array, i);
                    struct json_object *kind_obj, *file_obj, *label_obj;
                    
                    // Skip thumbnail tracks
                    if (json_object_object_get_ex(track_obj, "kind", &kind_obj)) {
                        const char *kind = json_object_get_string(kind_obj);
                        if (kind && strcmp(kind, "thumbnails") == 0) {
                            continue;
                        }
                    }
                    
                    if (json_object_object_get_ex(track_obj, "label", &label_obj)) {
                        stream_info->subtitles[subtitle_index].lang = safe_strdup(json_object_get_string(label_obj));
                    }
                    
                    if (json_object_object_get_ex(track_obj, "file", &file_obj)) {
                        stream_info->subtitles[subtitle_index].url = safe_strdup(json_object_get_string(file_obj));
                    }
                    
                    subtitle_index++;
                }
            }
        }
    }
    
    // Clean up
    json_object_put(json_obj);
    free(chunk.data);
    curl_global_cleanup();
    
    return stream_info;
}

// Define the Provider API
const ProviderAPI aniwatch_provider_api = {
    .search = aniwatch_search_anime,
    .free_search_results = NULL, // Use generic free function
    .get_anime_info = (void* (*)(const char*))aniwatch_get_anime_info,
    .get_episode_stream = (void* (*)(const char*, const char*))aniwatch_get_episode_stream,
    .free_anime_info = NULL, // Use generic free function
    .free_stream_info = NULL, // Use generic free function
    // Manga functions are not supported
    .get_manga_info = NULL,
    .get_chapter_pages = NULL,
    .free_manga_info = NULL,
    .free_chapter_info = NULL
};

// Get AniWatch provider API
const ProviderAPI* aniwatch_get_api() {
    return &aniwatch_provider_api;
}