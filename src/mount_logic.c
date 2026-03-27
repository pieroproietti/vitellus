#include "cJSON.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/stat.h>

int cmd_mount_binded(cJSON *json) {
  // 1. Estraiamo la workdir (es. /tmp/vitellus_work)
  cJSON *workdir = cJSON_GetObjectItemCaseSensitive(json, "workdir");
  cJSON *mounts = cJSON_GetObjectItemCaseSensitive(json, "mounts");

  if (!cJSON_IsString(workdir) || !cJSON_IsArray(mounts)) {
    fprintf(stderr, "{\"error\": \"Invalid workdir or mounts array\"}\n");
    return 1;
  }

  // Creiamo la workdir se non esiste (permessi 755)
  mkdir(workdir->valuestring, 0755);

  cJSON *mount_item = NULL;
  cJSON_ArrayForEach(mount_item, mounts) {
    cJSON *src = cJSON_GetObjectItemCaseSensitive(mount_item, "source");
    cJSON *tgt = cJSON_GetObjectItemCaseSensitive(mount_item, "target");

    if (cJSON_IsString(src) && cJSON_IsString(tgt)) {
      // Costruiamo il percorso completo: workdir + / + target
      char full_target[512];
      snprintf(full_target, sizeof(full_target), "%s/%s", workdir->valuestring,
               tgt->valuestring);

      // Creiamo la directory di destinazione dentro la workdir
      mkdir(full_target, 0755);

      // Eseguiamo il BIND MOUNT ricorsivo (MS_BIND | MS_REC)
      if (mount(src->valuestring, full_target, NULL, MS_BIND | MS_REC, NULL) ==
          0) {

        // Rende il mount "privato" (non propagato)
        mount(NULL, full_target, NULL, MS_PRIVATE | MS_REC, NULL);
        printf("{\"status\": \"ok\", \"action\": \"mount\", \"source\": "
               "\"%s\", \"target\": \"%s\"}\n",
               src->valuestring, full_target);
      } else {
        fprintf(
            stderr,
            "{\"status\": \"error\", \"source\": \"%s\", \"msg\": \"%s\"}\n",
            src->valuestring, strerror(errno));
      }
    }
  }
  return 0;
}