#ifndef API_H
#define API_H

// Structure to hold an anime result from search
typedef struct {
    char *id;
    char *title;
    char *image;
    char *type;
    int episodes;
} AnimeResult;

// Structure to hold search results
typedef struct {
    int total_results;
    AnimeResult *results;
} SearchResult;

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
} Source;

// Structure to hold subtitle information
typedef struct {
    char *url;
    char *lang;
} Subtitle;

// Structure to hold streaming information
typedef struct {
    char *referer;
    char *user_agent;
    Source *sources;
    int sources_count;
    Subtitle *subtitles;
    int subtitles_count;
} StreamInfo;

// Function declarations
SearchResult* api_search_anime(const char *query);
AnimeInfo* api_get_anime_info(const char *anime_id);
StreamInfo* api_get_episode_stream(const char *episode_id, const char *server);

// Cleanup functions
void api_free_search_results(SearchResult *results);
void api_free_anime_info(AnimeInfo *info);
void api_free_stream_info(StreamInfo *info);

#endif /* API_H */