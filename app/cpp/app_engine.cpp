#include "common/util_egl.h"
#include "common/util_oxr.h"
#include "app_engine.h"
#include "render_scene.h"

#include <cmath>

AppEngine::AppEngine(android_app *app) : m_app(app) {
}

AppEngine::~AppEngine() = default;

struct android_app *AppEngine::AndroidApp(void) const {
    return m_app;
}

static XrVector3f crossVec(const XrVector3f& a, const XrVector3f& b) {
    XrVector3f result;
    result.x = a.y * b.z - a.z * b.y;
    result.y = a.z * b.x - a.x * b.z;
    result.z = a.x * b.y - a.y * b.x;
    return result;
}

static XrVector3f rotateByQuat(const XrVector3f &v, const XrQuaternionf &q) {
    XrVector3f qv{q.x, q.y, q.z};
    XrVector3f u = crossVec(qv, v);
    u.x += q.w * v.x; u.y += q.w * v.y; u.z += q.w * v.z;
    XrVector3f cross2 = crossVec(qv, u);

    return XrVector3f{
            v.x + 2.0f * cross2.x,
            v.y + 2.0f * cross2.y,
            v.z + 2.0f * cross2.z
    };
}

void AppEngine::SetupInputActions() {
    // Create an action set
    XrActionSetCreateInfo asInfo{XR_TYPE_ACTION_SET_CREATE_INFO};
    strcpy(asInfo.actionSetName, "gameplay");
    strcpy(asInfo.localizedActionSetName, "Gameplay");
    asInfo.priority = 0;
    xrCreateActionSet(m_instance, &asInfo, &m_actionSet);

    // Create a Vector2 action for thumbstick
    XrActionCreateInfo acInfo{XR_TYPE_ACTION_CREATE_INFO};
    acInfo.actionType = XR_ACTION_TYPE_VECTOR2F_INPUT;
    strcpy(acInfo.actionName, "move");
    strcpy(acInfo.localizedActionName, "Move");
    acInfo.countSubactionPaths = 0;
    acInfo.subactionPaths = nullptr;
    xrCreateAction(m_actionSet, &acInfo, &m_moveAction);

    // Suggest bindings for Oculus/Quest touch controller thumbsticks (left+right)
    XrPath profilePath;
    xrStringToPath(m_instance, "/interaction_profiles/oculus/touch_controller", &profilePath);

    xrStringToPath(m_instance, "/user/hand/left/input/thumbstick", &m_pathLeft);
    xrStringToPath(m_instance, "/user/hand/right/input/thumbstick", &m_pathRight);

    XrActionSuggestedBinding bindings[2];
    bindings[0].action = m_moveAction;
    bindings[0].binding = m_pathLeft;
    bindings[1].action = m_moveAction;
    bindings[1].binding = m_pathRight;

    XrInteractionProfileSuggestedBinding suggested{XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING};
    suggested.interactionProfile = profilePath;
    suggested.suggestedBindings = bindings;
    suggested.countSuggestedBindings = 2;
    xrSuggestInteractionProfileBindings(m_instance, &suggested);

    // Attach action set to session
    XrSessionActionSetsAttachInfo attachInfo{XR_TYPE_SESSION_ACTION_SETS_ATTACH_INFO};
    attachInfo.countActionSets = 1;
    attachInfo.actionSets = &m_actionSet;
    xrAttachSessionActionSets(m_session, &attachInfo);
}

void AppEngine::PollAndApplyThumbstick(XrTime dpy_time, std::vector<XrView> &views) {
    // sync actions
    XrActiveActionSet active{};
    active.actionSet = m_actionSet;
    XrActionsSyncInfo syncInfo{XR_TYPE_ACTIONS_SYNC_INFO};
    syncInfo.countActiveActionSets = 1;
    syncInfo.activeActionSets = &active;
    xrSyncActions(m_session, &syncInfo);

    // get vector2 state (aggregate across left+right bindings)
    XrActionStateGetInfo getInfo{XR_TYPE_ACTION_STATE_GET_INFO};
    getInfo.action = m_moveAction;
    getInfo.subactionPath = XR_NULL_PATH;

    XrActionStateVector2f vec{XR_TYPE_ACTION_STATE_VECTOR2F};
    xrGetActionStateVector2f(m_session, &getInfo, &vec);

    // compute dt in seconds using XrTime (nanoseconds)
    float dt = 0.0f;
    if (m_lastFrameTime != 0 && dpy_time > m_lastFrameTime) {
        dt = static_cast<float>(double(dpy_time - m_lastFrameTime) / 1e9);
    }
    m_lastFrameTime = dpy_time;

    if (!vec.isActive) return;

    float sx = vec.currentState.x; // left/right
    float sy = vec.currentState.y; // up/down

    // local movement: x = right, z = forward (note: use -sy for forward if thumbstick up is positive)
    XrVector3f localMove{ sx * m_moveSpeed * dt, 0.0f, -sy * m_moveSpeed * dt };

    // rotate local move by head yaw/pitch as given by primary view orientation
    if (!views.empty()) {
        const XrQuaternionf &headQ = views[0].pose.orientation;
        XrVector3f worldMove = rotateByQuat(localMove, headQ);

        m_playerOffset.x += worldMove.x;
        m_playerOffset.y += worldMove.y;
        m_playerOffset.z += worldMove.z;
    }
}

void AppEngine::InitOpenXR_GLES() {
    void *vm = m_app->activity->vm;
    void *clazz = m_app->activity->clazz;

    oxr_initialize_loader(vm, clazz);

    m_instance = oxr_create_instance(vm, clazz);
    m_systemId = oxr_get_system(m_instance);

    egl_init_with_pbuffer_surface(3, 24, 0, 0, 16, 16);
    oxr_confirm_gfx_requirements(m_instance, m_systemId);

    init_gles_scene();

    m_session = oxr_create_session(m_instance, m_systemId);
    m_appSpace = oxr_create_ref_space(m_session, XR_REFERENCE_SPACE_TYPE_LOCAL);
    m_stageSpace = oxr_create_ref_space(m_session, XR_REFERENCE_SPACE_TYPE_STAGE);

    SetupInputActions();

    m_viewSurface = oxr_create_viewsurface(m_instance, m_systemId, m_session);
}

void AppEngine::UpdateFrame() {
    bool exit_loop, req_restart;
    oxr_poll_events(m_instance, m_session, &exit_loop, &req_restart);

    if (!oxr_is_session_running()) return;

    RenderFrame();
}

void AppEngine::RenderFrame() {
    std::vector<XrCompositionLayerBaseHeader*> all_layers;

    XrTime dpy_time, elapsed_us;
    oxr_begin_frame(m_session, &dpy_time);

    static XrTime init_time = -1;

    if (init_time < 0)
        init_time = dpy_time;

    elapsed_us = (dpy_time - init_time) / 1000;

    std::vector<XrCompositionLayerProjectionView> projLayerViews;
    XrCompositionLayerProjection projLayer;
    RenderLayer(dpy_time, elapsed_us, projLayerViews, projLayer);

    all_layers.push_back(reinterpret_cast<XrCompositionLayerBaseHeader *>(&projLayer));

    /* Compose all layers */
    oxr_end_frame(m_session, dpy_time, all_layers);
}

bool AppEngine::RenderLayer(XrTime dpy_time,
                            XrTime elapsed_us,
                            std::vector<XrCompositionLayerProjectionView> &layerViews,
                            XrCompositionLayerProjection &layer) {
    auto viewCount = (uint32_t) m_viewSurface.size();

    std::vector<XrView> views(viewCount, {XR_TYPE_VIEW});
    oxr_locate_views(m_session, dpy_time, m_appSpace, &viewCount, views.data());

    PollAndApplyThumbstick(dpy_time, views);

    layerViews.resize(viewCount);

    /* Acquire Stage Location (relative to the View Location) */
    XrSpaceLocation stageLoc{XR_TYPE_SPACE_LOCATION};
    xrLocateSpace(m_stageSpace, m_appSpace, dpy_time, &stageLoc);

    /* Render each view */
    for (uint32_t i = 0; i < viewCount; i++) {
        XrSwapchainSubImage subImg;
        render_target_t rTarget;

        oxr_acquire_viewsurface(m_viewSurface[i], rTarget, subImg);

        layerViews[i] = {XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW};
        layerViews[i].pose = views[i].pose;
        layerViews[i].fov = views[i].fov;
        layerViews[i].subImage = subImg;
        layerViews[i].pose.position.x += m_playerOffset.x;
        layerViews[i].pose.position.y += m_playerOffset.y;
        layerViews[i].pose.position.z += m_playerOffset.z;

        render_gles_scene(layerViews[i], rTarget, stageLoc.pose, elapsed_us, i);

        oxr_release_viewsurface(m_viewSurface[i]);
    }

    layer = {XR_TYPE_COMPOSITION_LAYER_PROJECTION};
    layer.space = m_appSpace;
    layer.viewCount = (uint32_t) layerViews.size();
    layer.views = layerViews.data();

    return true;
}