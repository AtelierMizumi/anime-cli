#ifndef MANGA_H
#define MANGA_H

#include "api.h"

// Structure to hold chapter information
typedef struct {
    char *id;
    int number;
    char *title;
    char *url;
} MangaChapter;

// Structure to hold complete manga information
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
    int total_chapters;
    MangaChapter *chapters;
} MangaInfo;

// Structure to hold chapter page information
typedef struct {
    char **page_urls;
    int page_count;
    char *referer;  // HTTP referer for image loading
    char *base_url; // Base URL for relative paths
} ChapterPages;

// Search for manga with the current provider
SearchResult* manga_search(const char *query);

// Get detailed manga information
MangaInfo* manga_get_info(const char *id);

// Get chapter pages
ChapterPages* manga_get_chapter_pages(const char *chapter_id);

// Free resources
void manga_free_search_results(SearchResult *results);
void manga_free_info(MangaInfo *info);
void manga_free_chapter_pages(ChapterPages *pages);

#endif /* MANGA_API_H */