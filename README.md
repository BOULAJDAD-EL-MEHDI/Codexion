*This project has been created as part of the 42 curriculum by <login1>.*

# Codexion

## Description

Codexion is a concurrency project where multiple coder threads compete for a limited number of dongles.

The goal is to manage shared resources safely while preventing deadlocks, starvation, race conditions, and other concurrency problems.

## Instructions

Compile the project with:

```bash
make
```

Run it with:

```bash
./codexion [args...]
```

Use the required arguments described in the project subject.

## Blocking cases handled

* **Deadlocks:** resource access is synchronized to prevent circular waiting and unsafe resource allocation.
* **Starvation:** synchronization ensures waiting coders can eventually access the required resources.
* **Cooldown:** coder cooldown periods are correctly handled.
* **Burnout:** the monitor detects coders that reach the burnout condition.
* **Log serialization:** log output is synchronized to prevent messages from different threads from overlapping.
* **Coffman's conditions:** the resource management strategy prevents the conditions necessary for deadlocks.

## Thread synchronization mechanisms

* **`pthread_mutex_t`:** protects shared resources such as dongles, logs, and monitor state.
* **`pthread_cond_t`:** allows threads to wait for and signal changes in shared states without busy waiting.
* **Custom events:** provide synchronized communication between coders and the monitor.

These mechanisms prevent race conditions when accessing shared data and ensure thread-safe communication between coders and the monitor.

## Resources

* Threads theory: linux programing interface book.
* POSIX Threads: https://man7.org/linux/man-pages/man7/pthreads.7.html
* `pthread_mutex`: https://man7.org/linux/man-pages/man3/pthread_mutex_lock.3.html
* `pthread_cond`: https://man7.org/linux/man-pages/man3/pthread_cond_wait.3.html
* 42 Codexion project subject.

### AI usage

AI was used to understand concurrency concepts, reason about synchronization and race conditions.
