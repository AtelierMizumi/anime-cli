#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "api.h"
#include "providers/aniwatch.h"
#include "providers/zoro.h"
#include "providers/mangadex.h"

// Provider API interfaces
static const ProviderAPI* provider_apis[PROVIDER_COUNT] = { NULL };

// Provider support for content types
static const bool provider_content_support[PROVIDER_COUNT][3] = {
    // ANIME, MANGA
    { true,  false }, // PROVIDER_ANIWATCH
    { true,  false }, // PROVIDER_ZORO
    { false, true  }  // PROVIDER_MANGADEX
};

// Provider names
static const char* provider_names[PROVIDER_COUNT] = {
    "AniWatch",
    "Zoro",
    "MangaDex"
};

// Content type names
static const char* content_type_names[2] = {
    "Anime",
    "Manga"
};

void api_init() {
    // Initialize provider APIs
    provider_apis[PROVIDER_ANIWATCH] = aniwatch_get_api();
    provider_apis[PROVIDER_ZORO] = zoro_get_api();
    provider_apis[PROVIDER_MANGADEX] = mangadex_get_api();
    // Add more providers as they are implemented
}

void api_cleanup() {
    // Any API cleanup needed
}

const ProviderAPI* get_provider_api(ProviderType provider) {
    if (provider < 0 || provider >= PROVIDER_COUNT) {
        return NULL;
    }
    return provider_apis[provider];
}

const char** get_available_providers(ContentType content_type, int *count) {
    static const char* available[PROVIDER_COUNT];
    int available_count = 0;
    
    for (int i = 0; i < PROVIDER_COUNT; i++) {
        if (provider_content_support[i][content_type]) {
            available[available_count++] = provider_names[i];
        }
    }
    
    *count = available_count;
    return available;
}

const char* content_type_to_string(ContentType type) {
    if (type < 0 || type > CONTENT_MANGA) {
        return "Unknown";
    }
    return content_type_names[type];
}

const char* provider_type_to_string(ProviderType provider) {
    if (provider < 0 || provider >= PROVIDER_COUNT) {
        return "Unknown";
    }
    return provider_names[provider];
}