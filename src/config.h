#ifndef CONFIG_H
#define CONFIG_H

#include "api/api.h"

// Configuration options
typedef struct {
    ProviderType default_provider;
    int ui_refresh_rate;
    char *mpv_additional_args;
    char *download_directory;
    bool cache_enabled;
} Config;

// Global configuration
extern Config app_config;

// Initialize configuration with defaults
void config_init();

// Save configuration to file
bool config_save();

// Load configuration from file
bool config_load();

// Free configuration resources
void config_cleanup();

// Get currently active provider
ProviderType get_current_provider();

// Set active provider
void set_current_provider(ProviderType provider);

#endif /* CONFIG_H */