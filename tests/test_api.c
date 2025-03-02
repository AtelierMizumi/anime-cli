#include <stdio.h>
#include <assert.h>
#include "api.h"

void test_api_search_anime() {
    // Test searching for anime
    SearchResult *results = api_search_anime("Naruto");
    assert(results != NULL);
    assert(results->count > 0);
    printf("test_api_search_anime passed.\n");
}

void test_api_get_anime_info() {
    // Test getting anime information
    AnimeInfo *anime = api_get_anime_info("1"); // Assuming "1" is a valid anime ID
    assert(anime != NULL);
    assert(anime->title != NULL);
    printf("test_api_get_anime_info passed.\n");
}

void test_api_get_episode_links() {
    // Test getting episode links
    Episode *episodes = api_get_episode_links("1"); // Assuming "1" is a valid anime ID
    assert(episodes != NULL);
    assert(episodes->count > 0);
    printf("test_api_get_episode_links passed.\n");
}

int main() {
    test_api_search_anime();
    test_api_get_anime_info();
    test_api_get_episode_links();
    printf("All API tests passed.\n");
    return 0;
}