#ifndef ANIME_API_H
#define ANIME_API_H

#include "api.h"

// Structure to hold episode information
typedef struct {
    char *id;
    int number;
    char *title;
    char *url;
} Episode;

// Structure to hold complete anime information
typedef struct {
    char *id;
    char *title;
    char *url;
    char *image;
    char *description;
    char *release_date;
    char *status;
    char **genres;
    int genres_count;
    char *sub_or_dub;
    int total_episodes;
    Episode *episodes;
} AnimeInfo;

// Structure to hold streaming source information
typedef struct {
    char *url;
    char *quality;
    int is_m3u8;
} StreamSource;

// Structure to hold subtitle information
typedef struct {
    char *url;
    char *lang;
} Subtitle;

// Structure to hold streaming information
typedef struct {
    char *referer;
    char *user_agent;
    StreamSource *sources;
    int sources_count;
    Subtitle *subtitles;
    int subtitles_count;
} StreamInfo;

// Search for anime with the current provider
SearchResult* anime_search(const char *query);

// Get detailed anime information
AnimeInfo* anime_get_info(const char *id);

// Get streaming information for an episode
StreamInfo* anime_get_episode_stream(const char *episode_id, const char *server);

// Free resources
void anime_free_search_results(SearchResult *results);
void anime_free_info(AnimeInfo *info);
void anime_free_stream_info(StreamInfo *info);

#endif /* ANIME_API_H */