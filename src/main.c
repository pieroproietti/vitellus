#include "cJSON.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int cmd_mount_binded(cJSON *json);
int cmd_umount_binded(cJSON *json);
int cmd_scan(cJSON *json);

// Funzione per leggere un file in memoria
char *read_file(const char *filename) {
  FILE *f = fopen(filename, "rb");
  if (!f)
    return NULL;
  fseek(f, 0, SEEK_END);
  long len = ftell(f);
  fseek(f, 0, SEEK_SET);
  char *data = malloc(len + 1);
  fread(data, 1, len, f);
  fclose(f);
  data[len] = '\0';
  return data;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Vitellus Core v0.1\nUsage: %s <task.json>\n", argv[0]);
    return 1;
  }

  char *json_data = read_file(argv[1]);
  if (!json_data) {
    fprintf(stderr, "Error: Could not read file %s\n", argv[1]);
    return 1;
  }

  cJSON *json = cJSON_Parse(json_data);
  if (!json) {
    fprintf(stderr, "Error: Invalid JSON format\n");
    return 1;
  }

  // Estraiamo il comando
  cJSON *command = cJSON_GetObjectItemCaseSensitive(json, "command");
  if (cJSON_IsString(command) && (command->valuestring != NULL)) {
    printf("Vitellus: Executing action '%s'...\n", command->valuestring);

    if (strcmp(command->valuestring, "mount_binded") == 0) {
      return cmd_mount_binded(json);
    } else if (strcmp(command->valuestring, "umount_binded") == 0) {
      return cmd_umount_binded(json);
    } else if (strcmp(command->valuestring, "scan") == 0) {
      return cmd_scan(json);
    }
  }

  cJSON_Delete(json);
  free(json_data);
  return 0;
}