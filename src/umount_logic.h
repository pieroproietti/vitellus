#ifndef UMOUNT_LOGIC_H
#define UMOUNT_LOGIC_H

#include "cJSON.h" // Necessario perché usiamo il tipo cJSON nel prototipo

/**
 * Smonta ricorsivamente i punti di mount definiti nel JSON.
 * Restituisce 0 in caso di successo, 1 in caso di errore.
 */
int cmd_umount_binded(cJSON *json);

#endif