/*
 * Copyright (C) 2026 Suhas Dissanayake <suhasdissa@gmail.com>
 *
 * Shared m17n-compatible state machine for Kesara. The maps and states
 * follow m17n/si-kesara.mim (Realtime Singlish / madura.x86).
 *
 * This file is part of Kesara.
 *
 * Kesara is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#include "kesara.h"

#include <stdlib.h>
#include <string.h>

#define KESARA_BUF 4096
#define KESARA_PENDING 16

#define ST_INIT 0
#define ST_SECOND 1
#define ST_THIRD 2
#define SHIFT_STAY -1

typedef struct {
    const char *key;
    const char *out;
    int del;
} Rule;

typedef struct {
    const Rule *rules;
    size_t n;
    int shift;
} Map;

struct KesaraState {
    char buf[KESARA_BUF];
    size_t len;
    size_t stable;
    char pending[KESARA_PENDING];
    size_t plen;
    int state;
};

/* ---- maps (literal port of si-kesara.mim) ---- */

static const Rule consonants[] = {
    {"k", "ක්", 0},
    {"c", "ක්", 0},
    {"C", "ක්", 0},
    {"K", "ඛ්", 0},
    {"kh", "ඛ්", 0},
    {"zk", "ඤ්", 0},
    {"Zk", "ඤ්", 0},
    {"g", "ග්", 0},
    {"G", "ඝ්", 0},
    {"gh", "ඝ්", 0},
    {"zD", "ඞ්", 0},
    {"ZD", "ඞ්", 0},
    {"X", "ඞ්", 0},
    {"zg", "ඟ්", 0},
    {"Zg", "ඟ්", 0},
    {"ch", "ච්", 0},
    {"Ch", "ඡ්", 0},
    {"j", "ජ්", 0},
    {"J", "ඣ්", 0},
    {"zh", "ඥ්", 0},
    {"Zh", "ඥ්", 0},
    {"zj", "ඦ්", 0},
    {"Zj", "ඦ්", 0},
    {"t", "ට්", 0},
    {"T", "ඨ්", 0},
    {"d", "ඩ්", 0},
    {"D", "ඪ්", 0},
    {"N", "ණ්", 0},
    {"zd", "ඬ්", 0},
    {"Zd", "ඬ්", 0},
    {"th", "ත්", 0},
    {"Th", "ථ්", 0},
    {"dh", "ද්", 0},
    {"Dh", "ධ්", 0},
    {"q", "ද්", 0},
    {"Q", "ධ්", 0},
    {"n", "න්", 0},
    {"zq", "ඳ්", 0},
    {"Zq", "ඳ්", 0},
    {"zdh", "ඳ්", 0},
    {"Zdh", "ඳ්", 0},
    {"p", "ප්", 0},
    {"P", "ඵ්", 0},
    {"ph", "ඵ්", 0},
    {"b", "බ්", 0},
    {"Bh", "භ්", 0},
    {"bh", "භ්", 0},
    {"m", "ම්", 0},
    {"M", "ම්", 0},
    {"B", "ඹ්", 0},
    {"y", "ය්", 0},
    {"r", "ර්", 0},
    {"R", "ර්", 0},
    {"l", "ල්", 0},
    {"w", "ව්", 0},
    {"W", "ව්", 0},
    {"v", "ව්", 0},
    {"V", "ව්", 0},
    {"sh", "ශ්", 0},
    {"S", "ෂ්", 0},
    {"s", "ස්", 0},
    {"h", "හ්", 0},
    {"L", "ළ්", 0},
    {"f", "ෆ්", 0},
    {"F", "ෆ්", 0},
    {"NG", "ක්‍ෂ‍්", 0},
};

static const Rule specialc[] = {
    {"m", "ම්", 0},
};

static const Rule special[] = {
    {"ri", "රි", 0},
};

static const Rule independent[] = {
    {"a", "අ", 0},
    {"aa", "ආ", 0},
    {"A", "ඇ", 0},
    {"AA", "ඈ", 0},
    {"Aa", "ඈ", 0},
    {"i", "ඉ", 0},
    {"I", "ඉ", 0},
    {"ii", "ඊ", 0},
    {"u", "උ", 0},
    {"U", "උ", 0},
    {"uu", "ඌ", 0},
    {"sRu", "ඍ", 0},
    {"sRuu", "ඎ", 0},
    {"e", "එ", 0},
    {"ee", "ඒ", 0},
    {"E", "ඓ", 0},
    {"o", "ඔ", 0},
    {"O", "ඔ", 0},
    {"oo", "ඕ", 0},
    {"ou", "ඖ", 0},
    {"x", "ං", 0},
    {"zn", "ං", 0},
    {"H", "ඃ", 0},
    {"z", "‌", 0},
    {"Z", "‌", 0},
};

static const Rule dependent[] = {
    {"a", "", 1},
    {"E", "", 1},
    {"aa", "ා", 1},
    {"A", "ැ", 1},
    {"AA", "ෑ", 1},
    {"i", "ි", 1},
    {"ii", "ී", 1},
    {"u", "ු", 1},
    {"uu", "ූ", 1},
    {"ru", "ෘ", 1},
    {"ruu", "ෲ", 1},
    {"e", "ෙ", 1},
    {"ee", "ේ", 1},
    {"ai", "ෛ", 1},
    {"o", "ො", 1},
    {"oo", "ෝ", 1},
    {"au", "ෞ", 1},
    {"Q", "‍්", 1},
};

static const Rule rakyan[] = {
    {"r", "්‍ර්", 1},
    {"y", "්‍ය්", 1},
};

#define N(a) (sizeof(a) / sizeof((a)[0]))

static const Map maps_init[] = {
    {consonants, N(consonants), ST_SECOND},
    {specialc, N(specialc), ST_THIRD},
    {independent, N(independent), SHIFT_STAY},
};

static const Map maps_second[] = {
    {rakyan, N(rakyan), SHIFT_STAY},
    {consonants, N(consonants), SHIFT_STAY},
    {dependent, N(dependent), ST_INIT},
};

static const Map maps_third[] = {
    {rakyan, N(rakyan), SHIFT_STAY},
    {special, N(special), ST_INIT},
    {dependent, N(dependent), ST_INIT},
};

static void maps_for_state(int state, const Map **maps, size_t *nmaps)
{
    switch (state) {
    case ST_SECOND:
        *maps = maps_second;
        *nmaps = N(maps_second);
        break;
    case ST_THIRD:
        *maps = maps_third;
        *nmaps = N(maps_third);
        break;
    default:
        *maps = maps_init;
        *nmaps = N(maps_init);
        break;
    }
}

/* ---- UTF-8 helpers ---- */

static size_t utf8_prev(const char *s, size_t len)
{
    if (len == 0) {
        return 0;
    }
    size_t i = len - 1;
    while (i > 0 && ((unsigned char)s[i] & 0xC0) == 0x80) {
        i--;
    }
    return i;
}

static int buf_append(KesaraState *s, const char *str)
{
    size_t n = strlen(str);
    if (s->len + n + 1 > KESARA_BUF) {
        return -1;
    }
    memcpy(s->buf + s->len, str, n);
    s->len += n;
    s->buf[s->len] = '\0';
    return 0;
}

static void buf_delete_last(KesaraState *s)
{
    s->len = utf8_prev(s->buf, s->len);
    s->buf[s->len] = '\0';
}

static void rollback_tentative(KesaraState *s)
{
    s->len = s->stable;
    s->buf[s->len] = '\0';
}

static void apply_rule(KesaraState *s, const Rule *r, const Map *map, int permanent)
{
    if (r->del) {
        buf_delete_last(s);
    }
    if (r->out && r->out[0]) {
        buf_append(s, r->out);
    }
    if (permanent) {
        s->stable = s->len;
        if (map->shift != SHIFT_STAY) {
            s->state = map->shift;
        }
    }
}

static int can_extend(const Map *maps, size_t nmaps, const char *pending, size_t plen)
{
    for (size_t m = 0; m < nmaps; m++) {
        for (size_t i = 0; i < maps[m].n; i++) {
            const char *k = maps[m].rules[i].key;
            size_t kl = strlen(k);
            if (kl > plen && strncmp(k, pending, plen) == 0) {
                return 1;
            }
        }
    }
    return 0;
}

static int find_exact(const Map *maps, size_t nmaps, const char *pending, size_t plen,
                      const Rule **orule, const Map **omap)
{
    for (size_t m = 0; m < nmaps; m++) {
        for (size_t i = 0; i < maps[m].n; i++) {
            const char *k = maps[m].rules[i].key;
            if (strlen(k) == plen && memcmp(k, pending, plen) == 0) {
                *orule = &maps[m].rules[i];
                *omap = &maps[m];
                return 1;
            }
        }
    }
    return 0;
}

static int find_longest_prefix(const Map *maps, size_t nmaps, const char *pending,
                               size_t plen, const Rule **orule, const Map **omap)
{
    size_t best = 0;
    const Rule *br = NULL;
    const Map *bm = NULL;
    for (size_t m = 0; m < nmaps; m++) {
        for (size_t i = 0; i < maps[m].n; i++) {
            const char *k = maps[m].rules[i].key;
            size_t kl = strlen(k);
            if (kl > best && kl <= plen && memcmp(k, pending, kl) == 0) {
                best = kl;
                br = &maps[m].rules[i];
                bm = &maps[m];
            }
        }
    }
    if (!br) {
        return 0;
    }
    *orule = br;
    *omap = bm;
    return (int)best;
}

static void fill_preedit(const KesaraState *s, KesaraResult *r)
{
    memcpy(r->preedit, s->buf, s->len + 1);
}

static void clear_pending(KesaraState *s)
{
    s->plen = 0;
    s->pending[0] = '\0';
}

static void commit_buf(KesaraState *s, KesaraResult *r)
{
    if (s->len > 0) {
        memcpy(r->commit, s->buf, s->len + 1);
    }
    s->len = 0;
    s->stable = 0;
    s->buf[0] = '\0';
    clear_pending(s);
    s->state = ST_INIT;
}

/* Consume as much of pending as can be finalized. Returns 1 if any leftover
 * unmatched single key should be passed through. */
static int reduce_pending(KesaraState *s, int force)
{
    const Map *maps;
    size_t nmaps;
    const Rule *rule;
    const Map *map;

    while (s->plen > 0) {
        maps_for_state(s->state, &maps, &nmaps);
        rollback_tentative(s);

        if (!force && can_extend(maps, nmaps, s->pending, s->plen)) {
            if (find_exact(maps, nmaps, s->pending, s->plen, &rule, &map)) {
                apply_rule(s, rule, map, 0);
            }
            return 0;
        }

        int n = find_longest_prefix(maps, nmaps, s->pending, s->plen, &rule, &map);
        if (n > 0) {
            apply_rule(s, rule, map, 1);
            size_t rest = s->plen - (size_t)n;
            memmove(s->pending, s->pending + n, rest);
            s->plen = rest;
            s->pending[s->plen] = '\0';
            continue;
        }

        /* No mapping for pending. */
        return 1;
    }
    return 0;
}

static void recompute_display(KesaraState *s)
{
    const Map *maps;
    size_t nmaps;
    const Rule *rule;
    const Map *map;

    rollback_tentative(s);
    if (s->plen == 0) {
        return;
    }
    maps_for_state(s->state, &maps, &nmaps);
    if (find_exact(maps, nmaps, s->pending, s->plen, &rule, &map)) {
        apply_rule(s, rule, map, 0);
    }
}

static int is_ascii_key(uint32_t key)
{
    return key >= 0x21 && key <= 0x7E;
}

static void copy_result_preedit(KesaraState *s, KesaraResult *r)
{
    fill_preedit(s, r);
}

KesaraState *kesara_new(void)
{
    KesaraState *s = calloc(1, sizeof(*s));
    return s;
}

void kesara_free(KesaraState *s)
{
    free(s);
}

void kesara_reset(KesaraState *s)
{
    if (!s) {
        return;
    }
    s->len = 0;
    s->stable = 0;
    s->buf[0] = '\0';
    clear_pending(s);
    s->state = ST_INIT;
}

int kesara_has_preedit(const KesaraState *s)
{
    return s && (s->len > 0 || s->plen > 0);
}

void kesara_flush(KesaraState *s, KesaraResult *result)
{
    memset(result, 0, sizeof(*result));
    reduce_pending(s, 1);
    /* Unmatched leftover pending is dropped from the key buffer but not
     * injected as Latin into Sinhala text. */
    clear_pending(s);
    rollback_tentative(s);
    s->stable = s->len;
    copy_result_preedit(s, result);
}

void kesara_commit_all(KesaraState *s, KesaraResult *result)
{
    kesara_flush(s, result);
    commit_buf(s, result);
    result->consumed = 0;
    result->preedit[0] = '\0';
}

void kesara_process(KesaraState *s, uint32_t key, unsigned mods, KesaraResult *result)
{
    memset(result, 0, sizeof(*result));

    if (mods & KESARA_MOD_CTRL) {
        if (kesara_has_preedit(s)) {
            kesara_flush(s, result);
            commit_buf(s, result);
        }
        result->consumed = 0;
        return;
    }

    if (key == KESARA_KEY_ESCAPE) {
        if (kesara_has_preedit(s)) {
            kesara_reset(s);
            result->consumed = 1;
        }
        copy_result_preedit(s, result);
        return;
    }

    if (key == KESARA_KEY_BACKSPACE) {
        if (s->plen > 0) {
            s->plen--;
            s->pending[s->plen] = '\0';
            recompute_display(s);
            result->consumed = 1;
            copy_result_preedit(s, result);
            return;
        }
        if (s->stable > 0) {
            buf_delete_last(s);
            s->stable = s->len;
            result->consumed = 1;
            copy_result_preedit(s, result);
            return;
        }
        result->consumed = 0;
        return;
    }

    if (key == KESARA_KEY_RETURN) {
        kesara_flush(s, result);
        if (s->len > 0) {
            memcpy(result->commit, s->buf, s->len + 1);
            s->len = 0;
            s->stable = 0;
            s->buf[0] = '\0';
            s->state = ST_INIT;
        }
        result->consumed = 0;
        result->preedit[0] = '\0';
        return;
    }

    /* Independent special keys (init only, like the .mim independent map). */
    if (s->state == ST_INIT && s->plen == 0) {
        if ((mods & KESARA_MOD_ALT) && key == '.') {
            buf_append(s, "෴");
            s->stable = s->len;
            result->consumed = 1;
            copy_result_preedit(s, result);
            return;
        }
        if ((mods & KESARA_MOD_ALT) && key == KESARA_KEY_SPACE) {
            buf_append(s, "‌");
            s->stable = s->len;
            result->consumed = 1;
            copy_result_preedit(s, result);
            return;
        }
        if ((mods & KESARA_MOD_SHIFT) && key == KESARA_KEY_SPACE) {
            buf_append(s, " ");
            s->stable = s->len;
            result->consumed = 1;
            copy_result_preedit(s, result);
            return;
        }
    }

    if (key == KESARA_KEY_SPACE && !(mods & (KESARA_MOD_SHIFT | KESARA_MOD_ALT))) {
        kesara_flush(s, result);
        size_t n = s->len;
        if (n + 2 <= sizeof(result->commit)) {
            memcpy(result->commit, s->buf, n);
            result->commit[n] = ' ';
            result->commit[n + 1] = '\0';
        }
        s->len = 0;
        s->stable = 0;
        s->buf[0] = '\0';
        clear_pending(s);
        s->state = ST_INIT;
        result->consumed = 1;
        result->preedit[0] = '\0';
        return;
    }

    if (!is_ascii_key(key) || (mods & KESARA_MOD_ALT)) {
        kesara_flush(s, result);
        if (s->len > 0) {
            memcpy(result->commit, s->buf, s->len + 1);
            s->len = 0;
            s->stable = 0;
            s->buf[0] = '\0';
            s->state = ST_INIT;
        }
        result->consumed = 0;
        result->preedit[0] = '\0';
        return;
    }

    if (s->plen + 1 >= KESARA_PENDING) {
        reduce_pending(s, 1);
        clear_pending(s);
    }

    s->pending[s->plen++] = (char)key;
    s->pending[s->plen] = '\0';

    if (reduce_pending(s, 0) != 0) {
        /* Unmatched leftover: finalize what we can, then pass this key. */
        char leftover[KESARA_PENDING];
        size_t llen = s->plen;
        memcpy(leftover, s->pending, llen + 1);
        clear_pending(s);
        rollback_tentative(s);
        s->stable = s->len;

        if (llen == 1) {
            if (s->len > 0) {
                memcpy(result->commit, s->buf, s->len + 1);
                s->len = 0;
                s->stable = 0;
                s->buf[0] = '\0';
                s->state = ST_INIT;
            }
            result->consumed = 0;
            result->preedit[0] = '\0';
            return;
        }

        /* Multi-char unmatched: drop first char as pass-through after commit. */
        if (s->len > 0) {
            memcpy(result->commit, s->buf, s->len + 1);
            s->len = 0;
            s->stable = 0;
            s->buf[0] = '\0';
            s->state = ST_INIT;
        }
        result->consumed = 0;
        result->preedit[0] = '\0';
        return;
    }

    result->consumed = 1;
    copy_result_preedit(s, result);
}
