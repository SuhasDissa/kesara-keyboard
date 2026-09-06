/*
 * Copyright (C) 2026 Suhas Dissanayake <suhasdissa@gmail.com>
 *
 * Native Fcitx 5 frontend for the shared Kesara engine.
 *
 * This file is part of Kesara.
 *
 * Kesara is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#include "kesara-im.h"

#include <fcitx/event.h>
#include <fcitx/inputcontext.h>
#include <fcitx/inputcontextmanager.h>
#include <fcitx/inputpanel.h>
#include <fcitx/text.h>

#include <string>

#include "kesara.h"

class KesaraICState : public fcitx::InputContextProperty {
public:
    explicit KesaraICState(fcitx::InputContext &ic)
        : ic_(&ic), engine_(kesara_new()) {}

    ~KesaraICState() override { kesara_free(engine_); }

    void reset() {
        kesara_reset(engine_);
        preedit_.clear();
        updateUI();
    }

    void commitAndClear() {
        KesaraResult r{};
        kesara_commit_all(engine_, &r);
        if (r.commit[0]) {
            ic_->commitString(r.commit);
        }
        preedit_.clear();
        updateUI();
    }

    void apply(const KesaraResult &r) {
        if (r.commit[0]) {
            ic_->commitString(r.commit);
        }
        preedit_ = r.preedit;
        updateUI();
    }

    void updateUI() {
        auto &panel = ic_->inputPanel();
        panel.reset();
        if (!preedit_.empty()) {
            fcitx::Text text(preedit_, fcitx::TextFormatFlag::Underline);
            text.setCursor(static_cast<int>(preedit_.size()));
            if (ic_->capabilityFlags().test(fcitx::CapabilityFlag::Preedit)) {
                panel.setClientPreedit(text);
            } else {
                panel.setPreedit(text);
            }
        }
        ic_->updatePreedit();
        ic_->updateUserInterface(fcitx::UserInterfaceComponent::InputPanel);
    }

    ::KesaraState *engine() { return engine_; }

private:
    fcitx::InputContext *ic_;
    ::KesaraState *engine_;
    std::string preedit_;
};

KesaraEngine::KesaraEngine(fcitx::Instance *instance)
    : instance_(instance), factory_([this](fcitx::InputContext &ic) {
          return new KesaraICState(ic);
      }) {
    instance_->inputContextManager().registerProperty("kesaraState", &factory_);
}

KesaraEngine::~KesaraEngine() = default;

void KesaraEngine::activate(const fcitx::InputMethodEntry & /*entry*/,
                            fcitx::InputContextEvent & /*event*/) {}

void KesaraEngine::deactivate(const fcitx::InputMethodEntry & /*entry*/,
                              fcitx::InputContextEvent &event) {
    auto *state = event.inputContext()->propertyFor(&factory_);
    state->commitAndClear();
}

void KesaraEngine::reset(const fcitx::InputMethodEntry & /*entry*/,
                         fcitx::InputContextEvent &event) {
    auto *state = event.inputContext()->propertyFor(&factory_);
    state->reset();
}

void KesaraEngine::keyEvent(const fcitx::InputMethodEntry & /*entry*/,
                            fcitx::KeyEvent &keyEvent) {
    if (keyEvent.isRelease()) {
        return;
    }

    auto *ic = keyEvent.inputContext();
    auto *state = ic->propertyFor(&factory_);
    const auto key = keyEvent.key();

    if (key.states().test(fcitx::KeyState::Super)) {
        state->commitAndClear();
        return;
    }

    unsigned mods = 0;
    if (key.states().test(fcitx::KeyState::Shift)) {
        mods |= KESARA_MOD_SHIFT;
    }
    if (key.states().test(fcitx::KeyState::Alt)) {
        mods |= KESARA_MOD_ALT;
    }
    if (key.states().test(fcitx::KeyState::Ctrl)) {
        mods |= KESARA_MOD_CTRL;
    }

    uint32_t k = 0;
    if (key.sym() == FcitxKey_BackSpace) {
        k = KESARA_KEY_BACKSPACE;
    } else if (key.sym() == FcitxKey_Escape) {
        k = KESARA_KEY_ESCAPE;
    } else if (key.sym() == FcitxKey_Return || key.sym() == FcitxKey_KP_Enter) {
        k = KESARA_KEY_RETURN;
    } else if (key.sym() == FcitxKey_space) {
        k = KESARA_KEY_SPACE;
    } else if (key.sym() >= 0x21 && key.sym() <= 0x7e) {
        k = static_cast<uint32_t>(key.sym());
    } else if (key.sym() == FcitxKey_Shift_L || key.sym() == FcitxKey_Shift_R ||
               key.sym() == FcitxKey_Control_L ||
               key.sym() == FcitxKey_Control_R || key.sym() == FcitxKey_Alt_L ||
               key.sym() == FcitxKey_Alt_R) {
        return;
    } else {
        state->commitAndClear();
        return;
    }

    KesaraResult r{};
    kesara_process(state->engine(), k, mods, &r);
    state->apply(r);
    if (r.consumed) {
        keyEvent.filterAndAccept();
    }
}

#if defined(FCITX_ADDON_FACTORY_V2)
FCITX_ADDON_FACTORY_V2(kesara, KesaraEngineFactory);
#else
FCITX_ADDON_FACTORY(KesaraEngineFactory);
#endif
