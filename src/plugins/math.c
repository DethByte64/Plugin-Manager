#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../plugin_manager.h"

static int do_math(char *output, char *args);
int plugin_deinit(void);

int plugin_init(PluginManager *pm) {
  strncpy(pm->name[pm->loaded_plugins], "math", sizeof(pm->name[0]) - 1);
  strncpy(pm->version[pm->loaded_plugins], "1.0", sizeof(pm->version[0]) - 1);
  pm->api_ver[pm->loaded_plugins] = API_VER;
  strncpy(pm->desc[pm->loaded_plugins],
      "Does maths.\nUsage: <num1> <op> <num2>\n"
      "  ops: + (add), - (subtract), * (multiply), / (divide)",
      sizeof(pm->desc[0]) - 1);
  pm->callback[pm->loaded_plugins] = do_math;
  printf("Math plugin initialized\n");
  return 0;
}

static int do_math(char *output, char *args) {
  char toks[MAX_TOKS][MAX_TOK_LEN] = {{0}};
  int tok_count = get_args(toks, " ", args);

  if (tok_count != 3) {
    strncpy(output, "Error: Expected 3 arguments (num1 op num2)", MAX_RETURN - 1);
    return 1;
  }

  int num1 = atoi(toks[0]);
  int num2 = atoi(toks[2]);
  int result;

  if (strcmp(toks[1], "+") == 0) {
    result = num1 + num2;
  } else if (strcmp(toks[1], "-") == 0) {
    result = num1 - num2;
  } else if (strcmp(toks[1], "*") == 0) {
    result = num1 * num2;
  } else if (strcmp(toks[1], "/") == 0) {
    if (num2 == 0) {
      strncpy(output, "Error: Division by zero", MAX_RETURN - 1);
      return 1;
    }
    result = num1 / num2;
  } else {
    strncpy(output, "Error: Unknown operator", MAX_RETURN - 1);
    return 1;
  }

  snprintf(output, MAX_RETURN, "%d", result);
  return 0;
}

int plugin_deinit(void) {
  printf("Math plugin deinitialized\n");
  return 0;
}
