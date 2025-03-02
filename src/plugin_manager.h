#ifndef PLUGIN_MANAGER_H
#define PLUGIN_MANAGER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

#define API_VER 1
#define MAX_PLUGINS 50
#define MAX_TOK_LEN 64
#define MAX_TOKS 16
#define MAX_RETURN 1024

typedef struct {
  int loaded_plugins;
  char name[MAX_PLUGINS][64];
  char version[MAX_PLUGINS][16];
  int api_ver[MAX_PLUGINS];
  char desc[MAX_PLUGINS][256];
  int (*callback[MAX_PLUGINS])(char *output, char *args);
  int (*deinit[MAX_PLUGINS])(void);
  void *handle[MAX_PLUGINS];
} PluginManager;

static PluginManager pm = {0};

// Function prototypes
int load_plugin(const char *path);
int unload_plugin(const char *name);
int run_plugin(const char *name, char *output, char *args);
int plugin_info(const char *name, char *output);
void list_plugins(char *output);
void unload_all_plugins(void);
int get_args(char out[MAX_TOKS][MAX_TOK_LEN], const char *delim, const char *str);

// Load a plugin from a shared object file
int load_plugin(const char *path) {
  if (pm.loaded_plugins >= MAX_PLUGINS) {
    fprintf(stderr, "Error: Maximum plugins reached\n");
    return 1;
  }

  void *handle = dlopen(path, RTLD_NOW);
  if (!handle) {
    fprintf(stderr, "Error: dlopen failed: %s\n", dlerror());
    return 2;
  }

  int (*plugin_init)(PluginManager *) = dlsym(handle, "plugin_init");
  char *err = dlerror();
  if (err || !plugin_init) {
    fprintf(stderr, "Error: Failed to load plugin_init: %s\n", err ? err : "unknown");
    dlclose(handle);
    return 3;
  }

  int (*plugin_deinit)(void) = dlsym(handle, "plugin_deinit");
  err = dlerror();
  if (err || !plugin_deinit) {
    fprintf(stderr, "Error: Failed to load plugin_deinit: %s\n", err ? err : "unknown");
    dlclose(handle);
    return 3;
  }

  pm.handle[pm.loaded_plugins] = handle;
  if (plugin_init(&pm) != 0) {
    fprintf(stderr, "Error: plugin_init failed\n");
    dlclose(handle);
    return 4;
  }

  if (pm.api_ver[pm.loaded_plugins] != API_VER) {
    fprintf(stderr, "Error: API version mismatch (%d != %d)\n", pm.api_ver[pm.loaded_plugins], API_VER);
    plugin_deinit();
    dlclose(handle);
    return 5;
  }

  // Store the plugin path as its name if not set by plugin_init
  if (strlen(pm.name[pm.loaded_plugins]) == 0) {
    strncpy(pm.name[pm.loaded_plugins], path, sizeof(pm.name[0]) - 1);
    pm.name[pm.loaded_plugins][sizeof(pm.name[0]) - 1] = '\0';
  }

  pm.deinit[pm.loaded_plugins] = plugin_deinit;
  pm.loaded_plugins++;
  return 0;
}

// Find plugin by name and return its index
static int find_plugin(const char *name) {
  for (int i = 0; i < pm.loaded_plugins; i++) {
    if (strcmp(pm.name[i], name) == 0) {
      return i;
    }
  }
  return -1;
}

// Unload a plugin by name
int unload_plugin(const char *name) {
  int id = find_plugin(name);
  if (id == -1) {
    fprintf(stderr, "Error: Plugin %s not found\n", name);
    return 1;
  }

  if (pm.deinit[id]) {
    pm.deinit[id]();
  }
  dlclose(pm.handle[id]);

  // Shift remaining plugins down
  for (int i = id; i < pm.loaded_plugins - 1; i++) {
    memcpy(pm.name[i], pm.name[i + 1], sizeof(pm.name[i]));
    memcpy(pm.version[i], pm.version[i + 1], sizeof(pm.version[i]));
    pm.api_ver[i] = pm.api_ver[i + 1];
    memcpy(pm.desc[i], pm.desc[i + 1], sizeof(pm.desc[i]));
    pm.callback[i] = pm.callback[i + 1];
    pm.deinit[i] = pm.deinit[i + 1];
    pm.handle[i] = pm.handle[i + 1];
  }

  pm.loaded_plugins--;
  return 0;
}

// Run a plugin's callback function
int run_plugin(const char *name, char *output, char *args) {
  int id = find_plugin(name);
  if (id == -1) {
    fprintf(stderr, "Error: Plugin %s not found\n", name);
    return 1;
  }
  if (!pm.callback[id]) {
    fprintf(stderr, "Error: No callback for plugin %s\n", name);
    return 2;
  }
  return pm.callback[id](output, args);
}

// Get plugin info
int plugin_info(const char *name, char *output) {
  int id = find_plugin(name);
  if (id == -1) {
    fprintf(stderr, "Error: Plugin %s not found\n", name);
    return 1;
  }

  snprintf(output, MAX_RETURN,
    "Name: %s\nVersion: %s\nAPI Version: %d\nDescription: %s",
    pm.name[id], pm.version[id], pm.api_ver[id], pm.desc[id]);
  return 0;
}

// List all loaded plugins
void list_plugins(char *output) {
  char temp[MAX_RETURN];
  snprintf(output, MAX_RETURN, "Loaded plugins: %d\n", pm.loaded_plugins);
  for (int i = 0; i < pm.loaded_plugins; i++) {
    snprintf(temp, sizeof(temp), "  %s (v%s)\n", pm.name[i], pm.version[i]);
    strncat(output, temp, MAX_RETURN - strlen(output) - 1);
  }
}

// Unload all plugins
void unload_all_plugins(void) {
  for (int i = 0; i < pm.loaded_plugins; i++) {
    if (pm.deinit[i]) {
      pm.deinit[i]();
    }
    dlclose(pm.handle[i]);
  }
  pm.loaded_plugins = 0;
}

// Split arguments into tokens
int get_args(char out[MAX_TOKS][MAX_TOK_LEN], const char *delim, const char *str) {
  char temp[MAX_RETURN];
  strncpy(temp, str, sizeof(temp) - 1);
  temp[sizeof(temp) - 1] = '\0';

  int tok_count = 0;
  char *token = strtok(temp, delim);
  while (token && tok_count < MAX_TOKS) {
    strncpy(out[tok_count], token, MAX_TOK_LEN - 1);
    out[tok_count][MAX_TOK_LEN - 1] = '\0';
    tok_count++;
    token = strtok(NULL, delim);
  }
  return tok_count;
}

#endif // PLUGIN_MANAGER_H
