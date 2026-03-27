#define _XOPEN_SOURCE 500
#include "scan_logic.h"
#include "cJSON.h"
#include <ctype.h> // Aggiunto per isspace()
#include <fnmatch.h>
#include <ftw.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint64_t current_total_bytes = 0;
static uint64_t current_file_count = 0;

// Gestione esclusioni
static char **exclude_patterns = NULL;
static int exclude_count = 0;

// Funzione di utilità per pulire i pattern caricati
static void clear_excludes() {
  if (exclude_patterns) {
    for (int i = 0; i < exclude_count; i++) {
      free(exclude_patterns[i]);
    }
    free(exclude_patterns);
    exclude_patterns = NULL;
  }
  exclude_count = 0;
}

static int is_excluded(const char *fpath) {
  for (int i = 0; i < exclude_count; i++) {
    if (fnmatch(exclude_patterns[i], fpath, FNM_PATHNAME) == 0) {
      return 1;
    }
  }
  return 0;
}

static int scan_callback(const char *fpath, const struct stat *sb, int tflag,
                         struct FTW *ftwbuf) {
  if (is_excluded(fpath)) {
    return FTW_SKIP_SUBTREE;
  }

  if (tflag == FTW_F) {
    current_total_bytes += sb->st_size;
    current_file_count++;
  }
  return 0;
}

// Spostiamo qui le funzioni di utility prima di cmd_scan
char *trim_whitespace(char *str) {
  char *end;
  while (isspace((unsigned char)*str))
    str++;
  if (*str == 0)
    return str;
  end = str + strlen(str) - 1;
  while (end > str && isspace((unsigned char)*end))
    end--;
  end[1] = '\0';
  return str;
}

static int load_excludes_from_file(const char *filename) {
  FILE *f = fopen(filename, "r");
  if (!f)
    return -1;

  char line[2048];
  while (fgets(line, sizeof(line), f)) {
    char *comment = strchr(line, '#');
    if (comment)
      *comment = '\0';

    char *token = strtok(line, " \t\r\n");
    while (token != NULL) {
      char *pattern = trim_whitespace(token);
      if (strlen(pattern) > 0) {
        char finalized_pattern[1024];
        // Normalizzazione: deve iniziare con /
        if (pattern[0] != '/' && pattern[0] != '*') {
          snprintf(finalized_pattern, sizeof(finalized_pattern), "/%s",
                   pattern);
        } else {
          strncpy(finalized_pattern, pattern, sizeof(finalized_pattern));
        }

        exclude_patterns =
            realloc(exclude_patterns, sizeof(char *) * (exclude_count + 1));
        exclude_patterns[exclude_count] = strdup(finalized_pattern);
        exclude_count++;
      }
      token = strtok(NULL, " \t\r\n");
    }
  }
  fclose(f);
  return 0;
}

int cmd_scan(cJSON *json) {
  cJSON *path_obj = cJSON_GetObjectItemCaseSensitive(json, "path");
  cJSON *excludes_array = cJSON_GetObjectItemCaseSensitive(json, "exclude");
  cJSON *exclude_file_obj =
      cJSON_GetObjectItemCaseSensitive(json, "exclude_file");

  if (!cJSON_IsString(path_obj))
    return 1;

  // Reset dati e pulizia memoria precedente
  current_total_bytes = 0;
  current_file_count = 0;
  clear_excludes();

  // 1. Carichiamo esclusioni dal file (se presente)
  if (cJSON_IsString(exclude_file_obj)) {
    load_excludes_from_file(exclude_file_obj->valuestring);
  }

  // 2. Carichiamo esclusioni extra dall'array JSON (se presente)
  int extra_count = cJSON_GetArraySize(excludes_array);
  for (int i = 0; i < extra_count; i++) {
    char *p = cJSON_GetArrayItem(excludes_array, i)->valuestring;
    exclude_patterns =
        realloc(exclude_patterns, sizeof(char *) * (exclude_count + 1));
    exclude_patterns[exclude_count] = strdup(p);
    exclude_count++;
  }

  // Scansione
  nftw(path_obj->valuestring, scan_callback, 64, FTW_PHYS | FTW_MOUNT);

  // Risposta
  cJSON *response = cJSON_CreateObject();
  cJSON_AddStringToObject(response, "status", "ok");
  cJSON_AddNumberToObject(response, "total_bytes", (double)current_total_bytes);
  cJSON_AddNumberToObject(response, "file_count", (double)current_file_count);
  cJSON_AddNumberToObject(response, "excludes_count", (double)exclude_count);

  char *out = cJSON_PrintUnformatted(response);
  printf("%s\n", out);

  // Pulizia finale
  clear_excludes();
  free(out);
  cJSON_Delete(response);
  return 0;
}
