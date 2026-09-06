/*
 * Copyright (C) 2026 Suhas Dissanayake <suhasdissa@gmail.com>
 *
 * Golden tests for the Kesara phonetic engine.
 *
 * This file is part of Kesara.
 *
 * Kesara is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#include "kesara.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *compose(const char *in)
{
    KesaraState *s = kesara_new();
    KesaraResult r;
    char *out = calloc(8192, 1);
    size_t used = 0;

    if (!s || !out) {
        abort();
    }

    for (const unsigned char *p = (const unsigned char *)in; *p; p++) {
        kesara_process(s, *p, 0, &r);
        size_t n = strlen(r.commit);
        memcpy(out + used, r.commit, n);
        used += n;
    }
    kesara_flush(s, &r);
    size_t n = strlen(r.commit);
    memcpy(out + used, r.commit, n);
    used += n;
    n = strlen(r.preedit);
    memcpy(out + used, r.preedit, n);
    used += n;
    out[used] = '\0';
    kesara_free(s);
    return out;
}

static int expect(const char *input, const char *want)
{
    char *got = compose(input);
    int ok = strcmp(got, want) == 0;
    if (!ok) {
        fprintf(stderr, "FAIL  input=%s\n  got:  %s\n  want: %s\n", input, got, want);
    }
    free(got);
    return ok ? 0 : 1;
}

int main(void)
{
    int fails = 0;

    fails += expect("k", "ක්");
    fails += expect("ka", "ක");
    fails += expect("kaa", "කා");
    fails += expect("ki", "කි");
    fails += expect("kii", "කී");
    fails += expect("ku", "කු");
    fails += expect("kuu", "කූ");
    fails += expect("ke", "කෙ");
    fails += expect("kee", "කේ");
    fails += expect("kai", "කෛ");
    fails += expect("ko", "කො");
    fails += expect("koo", "කෝ");
    fails += expect("kau", "කෞ");
    fails += expect("kA", "කැ");
    fails += expect("kAA", "කෑ");

    fails += expect("kh", "ඛ්");
    fails += expect("kha", "ඛ");
    fails += expect("ch", "ච්");
    fails += expect("cha", "ච");
    fails += expect("th", "ත්");
    fails += expect("tha", "ත");
    fails += expect("t", "ට්");
    fails += expect("ta", "ට");
    fails += expect("sh", "ශ්");
    fails += expect("sha", "ශ");

    fails += expect("kra", "ක්‍ර");
    fails += expect("kya", "ක්‍ය");
    fails += expect("kru", "කෘ");
    fails += expect("kruu", "කෲ");

    fails += expect("a", "අ");
    fails += expect("aa", "ආ");
    fails += expect("A", "ඇ");
    fails += expect("AA", "ඈ");
    fails += expect("i", "ඉ");
    fails += expect("ii", "ඊ");
    fails += expect("u", "උ");
    fails += expect("uu", "ඌ");
    fails += expect("e", "එ");
    fails += expect("ee", "ඒ");
    fails += expect("o", "ඔ");
    fails += expect("oo", "ඕ");
    fails += expect("ou", "ඖ");
    fails += expect("x", "ං");
    fails += expect("H", "ඃ");
    fails += expect("sRu", "ඍ");

    fails += expect("am", "අම්");
    fails += expect("amma", "අම්ම");
    fails += expect("lanka", "ලන්ක");
    fails += expect("laxkaa", "ලංකා");
    fails += expect("sri", "ස්‍රි");
    fails += expect("shri", "ශ්‍රි");
    fails += expect("NG", "ක්‍ෂ‍්");

    fails += expect("n", "න්");
    fails += expect("na", "න");
    fails += expect("dh", "ද්");
    fails += expect("dha", "ද");

    /* Space commits and includes a space. */
    {
        KesaraState *s = kesara_new();
        KesaraResult r;
        kesara_process(s, 'k', 0, &r);
        kesara_process(s, 'a', 0, &r);
        kesara_process(s, KESARA_KEY_SPACE, 0, &r);
        if (strcmp(r.commit, "ක ") != 0 || r.consumed != 1) {
            fprintf(stderr, "FAIL  space commit got '%s'\n", r.commit);
            fails++;
        }
        kesara_free(s);
    }

    /* Backspace after ka should remove the syllable. */
    {
        KesaraState *s = kesara_new();
        KesaraResult r;
        kesara_process(s, 'k', 0, &r);
        kesara_process(s, 'a', 0, &r);
        kesara_flush(s, &r);
        kesara_process(s, KESARA_KEY_BACKSPACE, 0, &r);
        if (r.preedit[0] != '\0' || r.consumed != 1) {
            fprintf(stderr, "FAIL  backspace after ka, preedit='%s'\n", r.preedit);
            fails++;
        }
        kesara_free(s);
    }

    if (fails) {
        fprintf(stderr, "%d test(s) failed\n", fails);
        return 1;
    }
    puts("kesara tests passed");
    return 0;
}
