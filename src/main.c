#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "plugin_manager.h"
#include <readline/readline.h>
#include <readline/history.h>

int main(void) {
  char output[MAX_RETURN] = {0};
  char args[MAX_RETURN] = {0};
  char *line;

  printf("Type 'help' for commands\n");

  while ((line = readline("> ")) != NULL) {
    if (strlen(line) == 0) {
      free(line);
      continue;
    }
    add_history(line);

    char toks[MAX_TOKS][MAX_TOK_LEN] = {{0}};
    int tok_count = get_args(toks, " ", line);

    if (tok_count == 0) {
      free(line);
      continue;
    }

    if (strcmp(toks[0], "load") == 0 && tok_count > 1) {
      if (load_plugin(toks[1]) == 0) {
        printf("Loaded %s\n", toks[1]);
      }
    }
    else if (strcmp(toks[0], "unload") == 0 && tok_count > 1) {
      if (unload_plugin(toks[1]) == 0) {
        printf("Unloaded %s\n", toks[1]);
      }
    }
    else if (strcmp(toks[0], "run") == 0 && tok_count > 2) {
      strncpy(args, line + strlen(toks[0]) + strlen(toks[1]) + 2, sizeof(args) - 1);
      args[sizeof(args) - 1] = '\0';
      if (run_plugin(toks[1], output, args) == 0) {
        printf("%s\n", output);
      }
    }
    else if (strcmp(toks[0], "info") == 0 && tok_count > 1) {
      if (plugin_info(toks[1], output) == 0) {
        printf("%s\n", output);
      }
    }
    else if (strcmp(toks[0], "list") == 0) {
      list_plugins(output);
      printf("%s\n", output);
    }
    else if (strcmp(toks[0], "exit") == 0) {
      break;
    }
    else if (strcmp(toks[0], "help") == 0) {
      printf("Commands:\n"
        "  load <path>     Load a plugin\n"
        "  unload <name>   Unload a plugin\n"
        "  run <name> <args> Run a plugin with arguments\n"
        "  info <name>     Show plugin info\n"
        "  list        List loaded plugins\n"
        "  exit        Exit and unload all plugins\n");
    }
    else {
      printf("Unknown command: %s\n", toks[0]);
    }

    memset(output, 0, sizeof(output));
    memset(args, 0, sizeof(args));
    free(line);
  }

  unload_all_plugins();
  free(line);
  return 0;
}
