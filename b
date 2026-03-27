gcc src/main.c \
    src/cJSON.c \
    src/mount_logic.c \
    src/umount_logic.c \
    src/scan_logic.c \
    -o vitellus \
    -lm