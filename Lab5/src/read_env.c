#include "read_env.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE 1024

void load_env(const char *filename) {
  FILE *file = fopen(filename, "r");
  if (!file) {
    perror("Could not open .env file");
    return;
  }

  char line[MAX_LINE];
  while (fgets(line, sizeof(line), file)) {
    // Skip comments and empty lines
    if (line[0] == '#' || line[0] == '\n')
      continue;

    // Remove trailing newline
    line[strcspn(line, "\n")] = 0;

    char *equals = strchr(line, '=');
    if (!equals)
      continue;

    *equals = '\0'; // Split the string into key and value
    char *key = line;
    char *value = equals + 1;

    // Set environment variable
    if (setenv(key, value, 1) != 0) {
      perror("setenv failed");
    }
  }

  fclose(file);
}
