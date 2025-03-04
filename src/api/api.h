#ifndef API_H
#define API_H

#include <stdbool.h>

// Provider type enumeration
typedef enum {
    PROVIDER_ANIWATCH,
    PROVIDER_ZORO,
    PROVIDER_GOGOANIME,
    PROVIDER_MANGADEX,
    PROVIDER_COUNT
} ProviderType;

// Content type enumeration
typedef enum {
    CONTENT_ANIME,
    CONTENT_MANGA
} ContentType;

// Base result structure
typedef struct {
    char *id;
    char *title;
    char *image;
    int episodes_or_chapters;
    ContentType content_type;
} SearchResultItem;

// Search results
typedef struct {
    int total_results;
    SearchResultItem *results;
} SearchResult;

// Provider API functions
typedef struct {
    // Common functions
    SearchResult* (*search)(const char *query);
    void (*free_search_results)(SearchResult *results);
    
    // Anime specific functions
    void *(*get_anime_info)(const char *id);
    void *(*get_episode_stream)(const char *episode_id, const char *server);
    void (*free_anime_info)(void *info);
    void (*free_stream_info)(void *info);
    
    // Manga specific functions
    void *(*get_manga_info)(const char *id);
    void *(*get_chapter_pages)(const char *chapter_id);
    void (*free_manga_info)(void *info);
    void (*free_chapter_info)(void *info);
} ProviderAPI;

// Initialize API system
void api_init();

// Clean up API system
void api_cleanup();

// Get API interface for a specific provider
const ProviderAPI* get_provider_api(ProviderType provider);

// Get list of available providers for a content type
const char** get_available_providers(ContentType content_type, int *count);

// Convert content type to string
const char* content_type_to_string(ContentType type);

// Convert provider type to string
const char* provider_type_to_string(ProviderType provider);

#endif /* API_H */