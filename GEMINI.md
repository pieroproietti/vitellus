# progetto: oa un motore in C per il remastering

oa is a high-performance core engine written in C, designed for GNU/Linux system remastering. It replaces fragile and slow Bash scripting with the precision and power of native Linux kernel syscalls.

Designed as a **standalone engine** to power **penguins-eggs** and other remastering tools like **MX-Snapshot** — the real ancestor of penguins-eggs (*) — oa provides a clean, JSON-based interface to manage critical system-level operations.

the name oa is simply my dialect word for eggs.

## Filosofia
- usiamo OverlayFS per proiettare il filestem reale nella liveroot e renderla scrivibile (come in [penguins-eggs](https://github.com/pieroproietti/penguins-eggs));
- contiene solo lo stretto necessario che può essere messo a fattor comune per la rimasterizzazione di - idealmente - tutte le distro (arch, debian, manjaro, rhel, opensuse);
- Usare la filosofia yocto per la creazione utenti passando per quanto già fatto in [penguins-eggs](https://github.com/pieroproietti/penguins-eggs/blob/master/src/classes/users.ts)

# Stato
- abbiamo la ISO che si avvia

# Prossimo obiettivo
- Implementare la action_users con il DNA di Yocto per gestire gli utenti nel chroot.

  