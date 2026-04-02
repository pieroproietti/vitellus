# 🐧 oa: Action Reference Manual

Every operation in **oa** is driven by a JSON "Plan."
This document defines the available actions, their parameters, and their expected behavior on the system.

---

## 🧹 1. action_cleanup
**Purpose**: Safely unmounts all projections and tears down the environment.

| Parameter | Type | Description |
| :--- | :--- | :--- |
| `pathLiveFs`| String | The base directory for the remastering process. |

**Behavior**:
1. Detaches the `tmpfs` Anti-Recursion mask from the working directory.
2. Unmounts all bind-mounts and OverlayFS directories (`/dev/pts`, `/dev`, `/run`, `/sys`, `/proc`, `/usr`, `/var`, `/home`) in reverse order.
3. Uses the `MNT_DETACH` flag with `umount2` to safely dismantle the mounts even if they are currently busy, preventing host locking.

---

## 🔐 2. action_crypted
**Purpose**: Encapsulates the compressed filesystem into a LUKS2 encrypted container.

| Parameter | Type | Description |
| :--- | :--- | :--- |
| `pathLiveFs`| String | The base directory for the remastering process. |
| `crypted_password` | String | The password for the LUKS volume (defaults to `"evolution"`). |

**Behavior**:
1. Calculates the required size based on `filesystem.squashfs` with a generous overhead margin.
2. Allocates a `root.img` file using `fallocate` (with `dd` fallback).
3. Formats the image as a LUKS2 container using the provided password.
4. Opens the LUKS container, formats it as `ext4`, and moves the `filesystem.squashfs` inside.
5. Unmounts and securely closes the LUKS volume.

---

## ⚙️ 3. action_initrd
**Purpose**: Generates the Initial RAM Disk for the live session via template substitution.

| Parameter | Type | Description |
| :--- | :--- | :--- |
| `pathLiveFs` | String | The base directory for the remastering process. |
| `initrd_cmd` | String | Shell template to generate the initrd (e.g., `mkinitramfs -o {{out}} {{ver}}`). |

**Behavior**:
1. Detects the host's kernel version using the `uname` syscall.
2. Replaces the `{{out}}` placeholder with the target `initrd.img` path.
3. Replaces the `{{ver}}` placeholder with the detected kernel version.
4. Executes the finalized command to build the initramfs.

---

## 💿 4. action_iso
**Purpose**: Masters the final bootable ISO image.

| Parameter | Type | Description |
| :--- | :--- | :--- |
| `pathLiveFs`| String | The base directory for the remastering process. |
| `volid` | String | The label of the ISO (e.g., `OA_LIVE`). |
| `output_iso` | String | The output filename (e.g., `live-system.iso`). |

**Behavior**:
1. Constructs the `xorriso` command using a large `CMD_MAX` buffer.
2. Configures the ISO with hybrid boot capabilities (`-isohybrid-mbr`) and an ISOLINUX bootloader.
3. Writes the output file to the root of `pathLiveFs`.

---

## 🖥️ 5. action_isolinux
**Purpose**: Populates legacy BIOS bootloader binaries and configuration.

| Parameter | Type | Description |
| :--- | :--- | :--- |
| `pathLiveFs` | String | The base directory for the remastering process. |

**Behavior**:
1. Creates the `iso/isolinux` directory.
2. Copies `isolinux.bin` and BIOS modules (`*.c32`) from `/usr/lib/`.
3. Generates a default `isolinux.cfg` boot menu if it does not already exist.

---

## 🗂️ 6. action_livestruct
**Purpose**: Prepares the core live directory structure and extracts the host kernel.

| Parameter | Type | Description |
| :--- | :--- | :--- |
| `pathLiveFs` | String | The base directory for the remastering process. |

**Behavior**:
1. Creates the `iso/live` directory.
2. Detects the host's running kernel version via `uname`.
3. Copies the corresponding `vmlinuz` from `/boot` into the live directory (with fallback to `/vmlinuz` symlink).

---

## 🏗️ 7. action_prepare
**Purpose**: Initializes the Zero-Copy environment using OverlayFS and bind mounts, with built-in protections against infinite scanning loops.

| Parameter | Type | Description |
| :--- | :--- | :--- |
| `pathLiveFs` | String | The base directory for the remastering process. |
| `mode` | String | Operation mode: `""` (default), `"clone"`, or `"crypted"`. |

**Behavior**:
1. Creates the base directory structure: `liveroot/` and `.overlay/` (with `lowerdir`, `upperdir`, and `workdir` inside).
2. Performs a physical copy of `/etc` to the `liveroot`.
3. Bind-mounts root entries (e.g., `/bin`, `/sbin`, `/lib`) in read-only mode using `MS_PRIVATE` propagation.
4. **Smart `/home` Handling**: If mode is `"clone"` or `"crypted"`, `/home` is bind-mounted read-only. For `"standard"`, it is created as an empty directory.
5. Projects `/usr` and `/var` using **OverlayFS** to allow modifications without touching the host.
6. Bind-mounts kernel API filesystems: `/proc`, `/sys`, `/run`, `/dev` into `liveroot/`.
7. **Anti-Recursion Mask**: Mounts a `tmpfs` layer over the working directory path inside the `liveroot` to completely shield tools like `mksquashfs` or `nftw` from falling into infinite "Inception" loops.

---

## 🚀 8. action_run
**Purpose**: Safely executes commands inside the `liveroot` chroot environment.

| Parameter | Type | Description |
| :--- | :--- | :--- |
| `pathLiveFs`| String | The base directory for the remastering process. |
| `run_command`| String | The command binary to execute (e.g., `apt-get`). |
| `args` | Array of Strings| Arguments to pass to the command (e.g., `["install", "-y", "nano"]`). |

**Behavior**:
1. Forks a new process.
2. The child process changes directory to `liveroot` and executes a native `chroot()` syscall.
3. Executes the requested command and arguments safely inside the isolated environment via `execvp`.

---

## 🔍 9. action_scan
**Purpose**: Scans a specific path to calculate the total filesystem size and file count.

| Parameter | Type | Description |
| :--- | :--- | :--- |
| `path` | String | The directory path to scan. |

**Behavior**:
1. Utilizes the POSIX `nftw` (file tree walk) function for a high-performance recursive scan.
2. Accumulates file sizes and counts.
3. Outputs a JSON response containing `total_bytes`, `total_mb`, and `file_count`.

---

## 📦 10. action_squash
**Purpose**: Compresses the `liveroot` into a high-performance SquashFS image.

| Parameter | Type | Description |
| :--- | :--- | :--- |
| `pathLiveFs`| String | The base directory for the remastering process. |
| `compression` | String | Algorithm (`zstd`, `xz`, `gzip`). Default: `zstd`. |
| `compression_level` | Integer | Compression level (e.g., 1-22 for zstd). Defaults to 3. |
| `exclude_list` | String | Path to a custom exclusion list file. |
| `mode` | String | `""`, `"clone"`, or `"crypted"`. |

**Behavior**:
1. Detects available online CPU cores to pass to `mksquashfs`.
2. Applies session exclusions including `/proc`, `/sys`, `/dev`, `/run`, and `/tmp`.
3. **Logic by Mode**: If `mode` is **NOT** `"clone"`, it automatically excludes `root/*`.
4. Uses the specified `exclude_list` if valid, otherwise falls back to `/usr/share/oa/exclusion.list`.
5. Generates the `filesystem.squashfs` with the specified compression options.

---

## ⏸️ 11. action_suspend
**Purpose**: Pauses the engine to allow for manual inspection or custom `chroot` commands.

| Parameter | Type | Description |
| :--- | :--- | :--- |
| `pathLiveFs` | String | The base directory for the remastering process. |
| `message` | String | Custom message to display to the user. |

**Behavior**:
1. Halts the execution of the JSON plan.
2. Prints the path to the `liveroot` chroot environment.
3. Waits for the user to press `ENTER` before resuming the workflow.

---

## 🛡️ 12. action_uefi
**Purpose**: Prepares the directory structure for UEFI booting.

| Parameter | Type | Description |
| :--- | :--- | :--- |
| `pathLiveFs` | String | The base directory for the remastering process. |

**Behavior**:
1. Creates the `iso/EFI/BOOT` directory.
2. Creates the `iso/boot/grub` directory.

---

## 👤 13. action_users
**Purpose**: Creates the Live user identity within the `liveroot` independently, without relying on host binaries.

| Parameter | Type | Description |
| :--- | :--- | :--- |
| `pathLiveFs` | String | The base directory for the remastering process. |
| `users` | Array of Objects | Contains user definitions (`login`, `password`, `home`, `shell`, `gecos`, `groups`). |
| `mode` | String | Operation mode: `""` (default), `"clone"`, or `"crypted"`. |

**Behavior**:
1. If `mode` is `"standard"`, purges host identities by sanitizing `/etc/passwd`, `/etc/shadow`, and `/etc/group` (removing UIDs between 1000 and 59999).
2. Opens `liveroot/etc/passwd` and `liveroot/etc/shadow` directly via C file streams.
3. Writes the new user identities and passwords natively using Yocto-inspired helper functions.
4. Injects secondary groups (e.g., `sudo`, `cdrom`) directly into `/etc/group`.
5. Creates the user's home directory, populates it from `/etc/skel` (including hidden files), and sets recursive ownership to the new UID/GID.
