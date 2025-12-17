#ifndef HELLOQUEST_DEBUG_H
#define HELLOQUEST_DEBUG_H

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>

#include <iostream>
#include <csignal>

#define DEBUG_BREAK raise(SIGTRAP)

inline void OpenXRDebugBreak() {
	std::cerr << "Breakpoint here to debug." << std::endl;
	DEBUG_BREAK;
}

inline const char* GetXRErrorString(XrInstance xrInstance, XrResult result) {
	static char string[XR_MAX_RESULT_STRING_SIZE];
	xrResultToString(xrInstance, result, string);
	return string;
}

#define OPENXR_CHECK(x, y)                                                                                 \
    {                                                                                                      \
        XrResult result = (x);                                                                             \
        if (!XR_SUCCEEDED(result)) {                                                                       \
            std::cerr << "ERROR: OPENXR: " << int(result) << " " << y << std::endl;                         \
            OpenXRDebugBreak();                                                                            \
        }                                                                                                  \
    }

#endif //HELLOQUEST_DEBUG_H
