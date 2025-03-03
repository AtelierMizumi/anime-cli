#ifndef ZORO_H
#define ZORO_H

#include "../api.h"

// Structure to hold episode information
typedef struct {
    char *id;
    int number;
    char *title;
    char *url;
} ZoroEpisode;

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
    ZoroEpisode *episodes;
} ZoroAnimeInfo;

// Structure to hold streaming source information
typedef struct {
    char *url;
    char *quality;
    int is_m3u8;
} ZoroSource;

// Structure to hold subtitle information
typedef struct {
    char *url;
    char *lang;
} ZoroSubtitle;

// Structure to hold streaming information
typedef struct {
    char *referer;
    char *user_agent;
    ZoroSource *sources;
    int sources_count;
    ZoroSubtitle *subtitles;
    int subtitles_count;
} ZoroStreamInfo;

// Get Zoro provider API
const ProviderAPI* zoro_get_api();

// API implementation functions for Zoro
SearchResult* zoro_search_anime(const char *query);
ZoroAnimeInfo* zoro_get_anime_info(const char *anime_id);
ZoroStreamInfo* zoro_get_episode_stream(const char *episode_id, const char *server);

// Cleanup functions
void zoro_free_search_results(SearchResult *results);
void zoro_free_anime_info(ZoroAnimeInfo *info);
void zoro_free_stream_info(ZoroStreamInfo *info);

#endif /* ZORO_H */