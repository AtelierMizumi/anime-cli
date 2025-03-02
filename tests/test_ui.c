#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ui.h"
#include "api.h"

void test_ui_get_search_query() {
    // Simulate user input for search query
    const char *input = "Naruto";
    // Redirect stdin to simulate user input
    FILE *input_stream = fmemopen((void *)input, strlen(input), "r");
    stdin = input_stream;

    char *query = ui_get_search_query();
    printf("Search Query: %s\n", query);
    free(query);
    fclose(input_stream);
}

void test_ui_select_anime() {
    // Create a mock search result
    SearchResult results;
    results.count = 2;
    results.anime = malloc(results.count * sizeof(AnimeInfo));
    strcpy(results.anime[0].title, "Naruto");
    strcpy(results.anime[1].title, "One Piece");

    // Simulate user selection
    int selected_index = 0; // User selects the first anime
    AnimeInfo *selected_anime = ui_select_anime(&results);
    printf("Selected Anime: %s\n", selected_anime->title);

    free(results.anime);
}

void test_ui_select_episode() {
    // Create a mock anime info
    AnimeInfo anime;
    strcpy(anime.title, "Naruto");
    Episode episodes[2] = {{"Episode 1"}, {"Episode 2"}};
    anime.episodes = episodes;
    anime.episode_count = 2;

    // Simulate user selection
    int selected_index = 0; // User selects the first episode
    Episode *selected_episode = ui_select_episode(&anime);
    printf("Selected Episode: %s\n", selected_episode->title);
}

int main() {
    test_ui_get_search_query();
    test_ui_select_anime();
    test_ui_select_episode();
    return 0;
}