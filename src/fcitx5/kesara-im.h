#ifndef _FCITX5_KESARA_KESARA_H_
#define _FCITX5_KESARA_KESARA_H_

#include <fcitx/addonfactory.h>
#include <fcitx/addonmanager.h>
#include <fcitx/inputcontextproperty.h>
#include <fcitx/inputmethodengine.h>
#include <fcitx/instance.h>

class KesaraEngine : public fcitx::InputMethodEngineV2 {
public:
    explicit KesaraEngine(fcitx::Instance *instance);
    ~KesaraEngine() override;

    void activate(const fcitx::InputMethodEntry &entry,
                  fcitx::InputContextEvent &event) override;
    void deactivate(const fcitx::InputMethodEntry &entry,
                    fcitx::InputContextEvent &event) override;
    void keyEvent(const fcitx::InputMethodEntry &entry,
                  fcitx::KeyEvent &keyEvent) override;
    void reset(const fcitx::InputMethodEntry &entry,
               fcitx::InputContextEvent &event) override;

private:
    fcitx::Instance *instance_;
    fcitx::FactoryFor<class KesaraICState> factory_;
};

class KesaraEngineFactory : public fcitx::AddonFactory {
    fcitx::AddonInstance *create(fcitx::AddonManager *manager) override {
        return new KesaraEngine(manager->instance());
    }
};

#endif
