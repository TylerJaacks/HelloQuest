#pragma once

#include "common/util_egl.h"
#include "common/util_oxr.h"

class AppEngine {
public:
    explicit AppEngine(android_app *app);

    ~AppEngine();

    // Interfaces to android application framework
    struct android_app *AndroidApp(void) const;

    void InitOpenXR_GLES();

    void UpdateFrame();
private:
    struct android_app *m_app;

    XrInstance m_instance{};
    XrSession m_session{};
    XrSpace m_appSpace{};
    XrSpace m_stageSpace{};
    XrSystemId m_systemId{};
    std::vector<viewsurface_t> m_viewSurface;

    void RenderFrame();

    bool RenderLayer(XrTime dpy_time,
                     XrTime elapsed_us,
                     std::vector<XrCompositionLayerProjectionView> &layerViews,
                     XrCompositionLayerProjection &layer);

    XrActionSet m_actionSet{};
    XrAction m_moveAction{};
    XrPath  m_pathLeft{XR_NULL_PATH};
    XrPath m_pathRight{XR_NULL_PATH};

    XrVector3f m_playerOffset{0.0f, 0.0f, 0.0f};
    XrTime  m_lastFrameTime{0};
    float m_moveSpeed{1.0f}; // meters per second

    void SetupInputActions();
    void PollAndApplyThumbstick(XrTime dpy_time, std::vector<XrView> &views);

public:
};

AppEngine *GetAppEngine(void);