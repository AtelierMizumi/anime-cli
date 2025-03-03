#ifndef MANGADEX_H
#define MANGADEX_H

#include "../api.h"

// Structure to hold chapter information
typedef struct {
    char *id;
    int number;
    char *title;
    char *url;
} MangadexChapter;

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
    MangadexChapter *chapters;
} MangadexMangaInfo;

// Structure to hold chapter page information
typedef struct {
    char **page_urls;
    int page_count;
} MangadexChapterPages;

// Get Mangadex provider API
const ProviderAPI* mangadex_get_api();

// API implementation functions for Mangadex
SearchResult* mangadex_search_manga(const char *query);
MangadexMangaInfo* mangadex_get_manga_info(const char *manga_id);
MangadexChapterPages* mangadex_get_chapter_pages(const char *chapter_id);

// Cleanup functions
void mangadex_free_search_results(SearchResult *results);
void mangadex_free_manga_info(MangadexMangaInfo *info);
void mangadex_free_chapter_pages(MangadexChapterPages *pages);

#endif /* MANGADEX_H */