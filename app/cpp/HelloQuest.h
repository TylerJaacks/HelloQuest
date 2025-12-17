#ifndef HELLOQUEST_HELLOQUEST_H
#define HELLOQUEST_HELLOQUEST_H
#include <android_native_app_glue.h>

#include "debug.h"

class HelloQuest {
public:
	HelloQuest() = default;
	~HelloQuest() = default;

	void Run() {
		while (m_applicationRunning) {
			PollSystemEvents();
		}
	}

public:
	static android_app* androidApp;

	struct AndroidAppState {
		ANativeWindow* nativeWindow = nullptr;
		bool resumed = false;
	};

	static AndroidAppState androidAppState;

	static void AndroidAppHandleCmd(struct android_app* app, int32_t cmd) {
		auto* appState = reinterpret_cast<AndroidAppState*>(app->userData);

		switch (cmd) {
			case APP_CMD_RESUME:
				appState->resumed = true;
				break;
			case APP_CMD_PAUSE:
				appState->resumed = false;
				break;
			case APP_CMD_INIT_WINDOW:
				appState->nativeWindow = app->window;
				break;
			case APP_CMD_TERM_WINDOW:
				appState->nativeWindow = nullptr;
				break;
			case APP_CMD_DESTROY:
				appState->nativeWindow = nullptr;
				break;
			default:
				break;
		}
	}

private:
	void PollSystemEvents() {
		if (androidApp->destroyRequested != 0) {
			m_applicationRunning = false;
			return;
		}

		while (true) {
			android_poll_source* source = nullptr;
			int events = 0;

			const int timeoutMilliseconds =
					(!androidAppState.resumed && !m_sessionRunning && androidApp->destroyRequested == 0)
					? -1
					: 0;

			if (ALooper_pollOnce(timeoutMilliseconds, nullptr, &events,
								 reinterpret_cast<void**>(&source)) >= 0) {
				if (source) {
					source->process(androidApp, source);
				}
			} else {
				break;
			}
		}
	}

private:
	bool m_applicationRunning = true;
	bool m_sessionRunning = false;
};

#endif //HELLOQUEST_HELLOQUEST_H
