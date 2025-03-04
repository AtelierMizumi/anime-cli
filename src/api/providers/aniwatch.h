#ifndef ANIWATCH_H
#define ANIWATCH_H

#include "../anime.h"

/**
 * Search for anime on AniWatch
 * @param query The search query
 * @return Search results structure or NULL on error
 */
SearchResult* aniwatch_search_anime(const char *query);

/**
 * Get detailed information about an anime
 * @param id The anime ID
 * @return Anime information structure or NULL on error
 */
AnimeInfo* aniwatch_get_anime_info(const char *id);

/**
 * Get streaming links for an episode
 * @param episode_id The episode ID
 * @param server Optional server name (defaults to "hd-1" if NULL)
 * @return Stream information structure or NULL on error
 */
StreamInfo* aniwatch_get_episode_stream(const char *episode_id, const char *server);

// Get Zoro provider API
const ProviderAPI* aniwatch_get_api();

#endif // ANIWATCH_H