#include "HelloQuest.h"

android_app* HelloQuest::androidApp = nullptr;
HelloQuest::AndroidAppState HelloQuest::androidAppState{};

void android_main(struct android_app* app) {
	JNIEnv* env = nullptr;
	app->activity->vm->AttachCurrentThread(&env, nullptr);

	PFN_xrInitializeLoaderKHR xrInitializeLoaderKHR = nullptr;
	OPENXR_CHECK(
			xrGetInstanceProcAddr(XR_NULL_HANDLE, "xrInitializeLoaderKHR",
								  reinterpret_cast<PFN_xrVoidFunction*>(&xrInitializeLoaderKHR)),
			"Failed to get xrInitializeLoaderKHR");

	if (!xrInitializeLoaderKHR)
		return;

	XrLoaderInitInfoAndroidKHR loaderInitInfo{
			XR_TYPE_LOADER_INIT_INFO_ANDROID_KHR};
	loaderInitInfo.applicationVM = app->activity->vm;
	loaderInitInfo.applicationContext = app->activity->clazz;

	OPENXR_CHECK(xrInitializeLoaderKHR(
			reinterpret_cast<XrLoaderInitInfoBaseHeaderKHR*>(&loaderInitInfo)),
			"Failed to initialize OpenXR loader");

	app->userData = &HelloQuest::androidAppState;
	app->onAppCmd = HelloQuest::AndroidAppHandleCmd;

	HelloQuest::androidApp = app;

	HelloQuest helloQuest;
	helloQuest.Run();
}