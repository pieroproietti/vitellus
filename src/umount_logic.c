#include <stdio.h>
#include <string.h>
#include <sys/mount.h>
#include <errno.h>
#include <unistd.h>     // Serve per rmdir()
#include "cJSON.h"      // <--- Questa è la riga che manca!
#include "umount_logic.h"

int cmd_umount_binded(cJSON *json) {
    cJSON *workdir = cJSON_GetObjectItemCaseSensitive(json, "workdir");
    cJSON *mounts = cJSON_GetObjectItemCaseSensitive(json, "mounts");

    if (!cJSON_IsString(workdir) || !cJSON_IsArray(mounts)) {
        fprintf(stderr, "{\"error\": \"Invalid JSON for umount\"}\n");
        return 1;
    }

    cJSON *mount_item = NULL;
    // IMPORTANTE: Smontiamo in ordine inverso (dall'ultimo al primo)
    int i = cJSON_GetArraySize(mounts);
    while (i > 0) {
        mount_item = cJSON_GetArrayItem(mounts, --i);
        cJSON *tgt = cJSON_GetObjectItemCaseSensitive(mount_item, "target");

        if (cJSON_IsString(tgt)) {
            char full_target[1024];
            snprintf(full_target, sizeof(full_target), "%s/%s", workdir->valuestring, tgt->valuestring);

            // MNT_DETACH permette di smontare anche se il mount è occupato
            if (umount2(full_target, MNT_DETACH) == 0) {
                printf("{\"status\": \"ok\", \"action\": \"umount\", \"target\": \"%s\"}\n", full_target);
                // Dopo lo smontaggio, possiamo rimuovere la directory vuota
                rmdir(full_target);
            } else {
                fprintf(stderr, "{\"status\": \"error\", \"target\": \"%s\", \"msg\": \"%s\"}\n", 
                        full_target, strerror(errno));
            }
        }
    }
    return 0;
}