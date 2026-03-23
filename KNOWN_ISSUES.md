# Known Issues

## librdkafka build requires libcurl headers on Linux

**Symptom:** Building on Linux fails with:

```
fatal error: curl/curl.h: No such file or directory
```

**Cause:** The librdkafka submodule has a bug in `src/rdkafka_conf.c` line 59 — it uses `#ifdef WITH_OAUTHBEARER_OIDC` instead of `#if WITH_OAUTHBEARER_OIDC`. The CMake build generates `#define WITH_OAUTHBEARER_OIDC 0` in `config.h` (via `#cmakedefine01`), which means the macro is always *defined* even when disabled. The `#ifdef` check passes, triggering `#include <curl/curl.h>`.

The rest of the codebase correctly uses `#if WITH_OAUTHBEARER_OIDC` (lines 197, 206, 4210), so no curl code is actually compiled or linked — only the header inclusion is affected.

**Fix:** Install the libcurl development headers:

```bash
sudo apt install libcurl4-openssl-dev
```

This is a **build-time dependency only**. The extension does not use or link against libcurl at runtime.
