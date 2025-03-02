#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../plugin_manager.h"

static int echo(char *output, char *input);
int plugin_deinit(void);

int plugin_init(PluginManager *pm) {
  strncpy(pm->name[pm->loaded_plugins], "echo", sizeof(pm->name[0]) - 1);
  strncpy(pm->version[pm->loaded_plugins], "1.0", sizeof(pm->version[0]) - 1);
  pm->api_ver[pm->loaded_plugins] = API_VER;
  strncpy(pm->desc[pm->loaded_plugins], "echoes back input\n", sizeof(pm->desc[0]) - 1);
  pm->callback[pm->loaded_plugins] = echo;
  printf("echo plugin initialized\n");
  return 0;
}

static int echo(char *output, char *input) {
//  char args[MAX_TOKS][MAX_TOK_LEN] = {{0}};
//  int arg_count = get_args(args, " ", input);

   printf("%s\n", input);

  /*

    main plugin code here
    args are available as args[0] args[1] args[2] and so on

  */

  return 0;
}

int plugin_deinit(void) {
  printf("echo plugin deinitialized\n");
  return 0;
}
