#pragma once

int init_gles_scene ();
int render_gles_scene (XrCompositionLayerProjectionView &layerView,
                       render_target_t &rTarget, XrPosef &stagePose,
                       XrTime elapsed_us, uint32_t viewID);
