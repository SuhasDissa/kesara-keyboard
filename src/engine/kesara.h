/*
 * Copyright (C) 2026 Suhas Dissanayake <suhasdissa@gmail.com>
 *
 * This file is part of Kesara.
 *
 * Kesara is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef KESARA_ENGINE_H
#define KESARA_ENGINE_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define KESARA_KEY_BACKSPACE 0x08u
#define KESARA_KEY_RETURN 0x0Du
#define KESARA_KEY_ESCAPE 0x1Bu
#define KESARA_KEY_SPACE 0x20u

#define KESARA_MOD_SHIFT 1u
#define KESARA_MOD_ALT 2u
#define KESARA_MOD_CTRL 4u

typedef struct KesaraState KesaraState;

typedef struct {
    char commit[4096];
    char preedit[4096];
    int consumed;
} KesaraResult;

KesaraState *kesara_new(void);
void kesara_free(KesaraState *s);
void kesara_reset(KesaraState *s);

/* Process one key. Fills result; result->consumed is 1 if the key should
 * not be forwarded to the application. */
void kesara_process(KesaraState *s, uint32_t key, unsigned mods,
                    KesaraResult *result);

/* Treat remaining pending keys as complete (timeout / end of input). */
void kesara_flush(KesaraState *s, KesaraResult *result);

/* Copy composing text into dst and clear the engine (for deactivate). */
void kesara_commit_all(KesaraState *s, KesaraResult *result);

int kesara_has_preedit(const KesaraState *s);

#ifdef __cplusplus
}
#endif

#endif /* KESARA_ENGINE_H */
