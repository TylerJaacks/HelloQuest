#include "AppEngine.h"

struct AndroidAppState {
    ANativeWindow *NativeWindow = nullptr;
    bool Resumed = false;
};

static void ProcessAndroidCmd(struct android_app *app, int32_t cmd) {
    auto *appState = (AndroidAppState *) app->userData;

    switch (cmd) {
        case APP_CMD_START:
            LOGI ("APP_CMD_START");
            break;
        case APP_CMD_RESUME:
            LOGI ("APP_CMD_RESUME");
            appState->Resumed = true;
            break;
        case APP_CMD_PAUSE:
            LOGI ("APP_CMD_PAUSE");
            appState->Resumed = false;
            break;
        case APP_CMD_STOP:
            LOGI ("APP_CMD_STOP");
            break;
        case APP_CMD_DESTROY:
            LOGI ("APP_CMD_DESTROY");
            appState->NativeWindow = nullptr;
            break;
        case APP_CMD_INIT_WINDOW:
            LOGI ("APP_CMD_INIT_WINDOW");
            appState->NativeWindow = app->window;
            break;
        case APP_CMD_TERM_WINDOW:
            LOGI ("APP_CMD_TERM_WINDOW");
            appState->NativeWindow = nullptr;
            break;
        default:
            break;
    }
}

void android_main(struct android_app *app) {
    AndroidAppState appState = {};
    app->userData = &appState;
    app->onAppCmd = ProcessAndroidCmd;

    AppEngine engine(app);
    engine.InitOpenXR_GLES();

    while (app->destroyRequested == 0) {
        for (;;) {
            int events;
            struct android_poll_source *source;

            int timeout = -1;

            if (appState.Resumed || oxr_is_session_running() || app->destroyRequested)
                timeout = 0;

            if (ALooper_pollAll(timeout, nullptr, &events, (void **) &source) < 0) break;

            if (source != nullptr) source->process(app, source);
        }

        engine.UpdateFrame();
    }
}