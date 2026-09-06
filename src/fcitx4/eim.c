/*
 * Copyright (C) 2026 Suhas Dissanayake <suhasdissa@gmail.com>
 *
 * Native Fcitx 4 frontend for the shared Kesara engine.
 *
 * This file is part of Kesara.
 *
 * Kesara is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#include <fcitx-config/hotkey.h>
#include <fcitx-utils/utils.h>
#include <fcitx/candidate.h>
#include <fcitx/context.h>
#include <fcitx/fcitx.h>
#include <fcitx/hook.h>
#include <fcitx/ime.h>
#include <fcitx/instance.h>
#include <fcitx/keys.h>
#include <fcitx/profile.h>
#include <fcitx/ui.h>
#include <string.h>

#include "kesara.h"

typedef struct {
    FcitxInstance *owner;
    KesaraState *engine;
    char preedit[4096];
    int forward;
} FcitxKesara;

static void *FcitxKesaraCreate(FcitxInstance *instance);
static void FcitxKesaraDestroy(void *arg);
static boolean FcitxKesaraInit(void *arg);
static INPUT_RETURN_VALUE FcitxKesaraDoInput(void *arg, FcitxKeySym sym,
                                             unsigned int state);
static INPUT_RETURN_VALUE FcitxKesaraGetCandWords(void *arg);
static void FcitxKesaraReset(void *arg);

FCITX_EXPORT_API
const FcitxIMClass ime = {
    .Create = FcitxKesaraCreate,
    .Destroy = FcitxKesaraDestroy,
};

FCITX_EXPORT_API
const int ABI_VERSION = FCITX_ABI_VERSION;

static const FcitxIMIFace kesara_iface = {
    .ResetIM = FcitxKesaraReset,
    .DoInput = FcitxKesaraDoInput,
    .GetCandWords = FcitxKesaraGetCandWords,
    .PhraseTips = NULL,
    .Save = NULL,
    .Init = FcitxKesaraInit,
    .ReloadConfig = NULL,
    .KeyBlocker = NULL,
    .UpdateSurroundingText = NULL,
    .DoReleaseInput = NULL,
};

static void *FcitxKesaraCreate(FcitxInstance *instance)
{
    FcitxKesara *kesara = fcitx_utils_malloc0(sizeof(FcitxKesara));
    kesara->owner = instance;
    kesara->engine = kesara_new();
    if (!kesara->engine) {
        free(kesara);
        return NULL;
    }
    FcitxInstanceRegisterIMv2(instance, kesara, "kesara", "Kesara", "kesara",
                              kesara_iface, 1, "si");
    return kesara;
}

static void FcitxKesaraDestroy(void *arg)
{
    FcitxKesara *kesara = (FcitxKesara *)arg;
    if (!kesara) {
        return;
    }
    kesara_free(kesara->engine);
    free(kesara);
}

static boolean FcitxKesaraInit(void *arg)
{
    FcitxKesara *kesara = (FcitxKesara *)arg;
    if (!kesara) {
        return false;
    }
    FcitxInstanceSetContext(kesara->owner, CONTEXT_IM_KEYBOARD_LAYOUT, "us");
    return true;
}

static void FcitxKesaraReset(void *arg)
{
    FcitxKesara *kesara = (FcitxKesara *)arg;
    if (!kesara) {
        return;
    }
    kesara_reset(kesara->engine);
    kesara->preedit[0] = '\0';
    kesara->forward = 0;
}

static void apply_result(FcitxKesara *kesara, const KesaraResult *r)
{
    if (r->commit[0]) {
        FcitxInstanceCommitString(kesara->owner,
                                  FcitxInstanceGetCurrentIC(kesara->owner),
                                  r->commit);
    }
    strncpy(kesara->preedit, r->preedit, sizeof(kesara->preedit) - 1);
    kesara->preedit[sizeof(kesara->preedit) - 1] = '\0';
}

static INPUT_RETURN_VALUE FcitxKesaraDoInput(void *arg, FcitxKeySym sym,
                                             unsigned int state)
{
    FcitxKesara *kesara = (FcitxKesara *)arg;
    uint32_t k = 0;
    unsigned mods = 0;
    KesaraResult r;

    kesara->forward = 0;

    if (state & FcitxKeyState_Ctrl) {
        mods |= KESARA_MOD_CTRL;
    }
    if (state & FcitxKeyState_Shift) {
        mods |= KESARA_MOD_SHIFT;
    }
    if (state & FcitxKeyState_Alt) {
        mods |= KESARA_MOD_ALT;
    }

    if (FcitxHotkeyIsHotKey(sym, state, FCITX_ESCAPE)) {
        k = KESARA_KEY_ESCAPE;
    } else if (FcitxHotkeyIsHotKey(sym, state, FCITX_BACKSPACE)) {
        k = KESARA_KEY_BACKSPACE;
    } else if (FcitxHotkeyIsHotKey(sym, state, FCITX_ENTER)) {
        k = KESARA_KEY_RETURN;
    } else if (sym == FcitxKey_space) {
        k = KESARA_KEY_SPACE;
    } else if (sym >= 0x21 && sym <= 0x7e) {
        k = (uint32_t)sym;
    } else if (sym == FcitxKey_Shift_L || sym == FcitxKey_Shift_R ||
               sym == FcitxKey_Control_L || sym == FcitxKey_Control_R ||
               sym == FcitxKey_Alt_L || sym == FcitxKey_Alt_R) {
        return IRV_TO_PROCESS;
    } else {
        kesara_commit_all(kesara->engine, &r);
        apply_result(kesara, &r);
        kesara->forward = 1;
        return IRV_DISPLAY_CANDWORDS;
    }

    kesara_process(kesara->engine, k, mods, &r);
    apply_result(kesara, &r);
    if (!r.consumed) {
        kesara->forward = 1;
    }
    return IRV_DISPLAY_CANDWORDS;
}

static INPUT_RETURN_VALUE FcitxKesaraGetCandWords(void *arg)
{
    FcitxKesara *kesara = (FcitxKesara *)arg;
    FcitxInputState *input;
    FcitxInputContext *ic;
    FcitxProfile *profile;
    size_t l = strlen(kesara->preedit);

    FcitxInstanceCleanInputWindow(kesara->owner);
    if (l) {
        input = FcitxInstanceGetInputState(kesara->owner);
        ic = FcitxInstanceGetCurrentIC(kesara->owner);
        profile = FcitxInstanceGetProfile(kesara->owner);
        if (ic && ((ic->contextCaps & CAPACITY_PREEDIT) == 0 ||
                   !profile->bUsePreedit)) {
            FcitxMessagesAddMessageAtLast(FcitxInputStateGetPreedit(input),
                                          MSG_INPUT, "%s", kesara->preedit);
            FcitxInputStateSetShowCursor(input, true);
            FcitxInputStateSetCursorPos(input, (int)l);
        } else {
            FcitxMessagesAddMessageAtLast(
                FcitxInputStateGetClientPreedit(input), MSG_INPUT, "%s",
                kesara->preedit);
            FcitxInputStateSetClientCursorPos(input, (int)l);
        }
    }

    INPUT_RETURN_VALUE ret = IRV_DISPLAY_CANDWORDS;
    if (kesara->forward) {
        ret |= IRV_DONOT_PROCESS;
        kesara->forward = 0;
    }
    return ret;
}
