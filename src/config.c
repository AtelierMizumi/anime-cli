#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"
#include "utils/memory.h"

#define CONFIG_FILE_PATH "anime-cli.conf"

// Global configuration instance
Config app_config;

// Currently active provider
static ProviderType current_provider;

void config_init() {
    // Set default configuration
    app_config.default_provider = PROVIDER_ZORO;
    app_config.ui_refresh_rate = 100;
    app_config.mpv_additional_args = safe_strdup("--force-window=immediate --cache=yes");
    app_config.download_directory = safe_strdup("./downloads");
    app_config.cache_enabled = true;
    
    // Set initial provider to default
    current_provider = app_config.default_provider;
    
    // Try to load from config file
    config_load();
}

bool config_save() {
    FILE *config_file = fopen(CONFIG_FILE_PATH, "w");
    if (!config_file) {
        return false;
    }
    
    fprintf(config_file, "default_provider=%d\n", app_config.default_provider);
    fprintf(config_file, "ui_refresh_rate=%d\n", app_config.ui_refresh_rate);
    fprintf(config_file, "mpv_additional_args=%s\n", app_config.mpv_additional_args);
    fprintf(config_file, "download_directory=%s\n", app_config.download_directory);
    fprintf(config_file, "cache_enabled=%d\n", app_config.cache_enabled);
    
    fclose(config_file);
    return true;
}

bool config_load() {
    FILE *config_file = fopen(CONFIG_FILE_PATH, "r");
    if (!config_file) {
        return false;
    }
    
    char line[512];
    char value[384]; // Remove the 'key' variable as it's not used
    
    while (fgets(line, sizeof(line), config_file)) {
        // Fix the provider type mismatch
        int provider_value;
        if (sscanf(line, "default_provider=%d", &provider_value) == 1) {
            app_config.default_provider = (ProviderType)provider_value;
            continue;
        }
        
        if (sscanf(line, "ui_refresh_rate=%d", &app_config.ui_refresh_rate) == 1) {
            continue;
        }
        
        // Fix the boolean type mismatch
        int cache_value;
        if (sscanf(line, "cache_enabled=%d", &cache_value) == 1) {
            app_config.cache_enabled = (cache_value != 0);
            continue;
        }
        
        if (sscanf(line, "mpv_additional_args=%[^\n]", value) == 1) {
            free(app_config.mpv_additional_args);
            app_config.mpv_additional_args = safe_strdup(value);
            continue;
        }
        
        if (sscanf(line, "download_directory=%[^\n]", value) == 1) {
            free(app_config.download_directory);
            app_config.download_directory = safe_strdup(value);
            continue;
        }
    }
    
    fclose(config_file);
    current_provider = app_config.default_provider;
    return true;
}

void config_cleanup() {
    free(app_config.mpv_additional_args);
    free(app_config.download_directory);
}

ProviderType get_current_provider() {
    return current_provider;
}

void set_current_provider(ProviderType provider) {
    if (provider >= 0 && provider < PROVIDER_COUNT) {
        current_provider = provider;
    }
}