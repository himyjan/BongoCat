#include "model_load_progress.h"
#include "runtime.h"
#include "bongo_cat/preferences.h"

#include <SDL3/SDL.h>
#include <stdio.h>

void bongo_cat_model_request_frame(BongoCatApp *app, bool reveal) {
    if (!app) return;
    if (app->window) {
        SDL_WindowFlags flags = SDL_GetWindowFlags(app->window);
        bool hidden = (flags & (SDL_WINDOW_HIDDEN | SDL_WINDOW_MINIMIZED)) != 0;
        if (reveal && (!app->session.window.visible || hidden ||
            app->hover_hidden))
            bongo_cat_window_set_visible(app, true);
        else if (app->session.window.visible)
            bongo_cat_window_clamp_to_display(app);
    }
    bongo_cat_window_mark_hit_dirty(app);
    app->dirty = true;
}

void bongo_cat_model_runtime_stage(BongoCatApp *app, const char *stage,
    const BongoCatModelEntry *entry) {
    char state[BONGO_CAT_ID_CAP + 32];
    const char *id = entry ? entry->id : "unknown";
    snprintf(state, sizeof(state), "model-load:%s:%s", stage, id);
    bongo_cat_runtime_stage(app, state);
    SDL_Log("[runtime] Model load %s: id=%s", stage, id);
}

static void model_progress_runtime_stage(BongoCatApp *app, float progress) {
    static const char *names[] = {"", "model-core", "texture-loading",
        "finalizing"};
    unsigned stage = progress >= .95f ? 3 : progress >= .50f ? 2 : 1;
    if (!app || stage == app->model_load_runtime_stage) return;
    app->model_load_runtime_stage = stage;
    char state[BONGO_CAT_ID_CAP + 40];
    snprintf(state, sizeof(state), "model-load:%s:%s", names[stage],
        app->loading_model[0] ? app->loading_model : "unknown");
    bongo_cat_runtime_stage(app, state);
}

void bongo_cat_model_load_progress(void *userdata, float progress) {
    BongoCatApp *app = userdata;
    model_progress_runtime_stage(app, progress);
    /* Keep native window procedures responsive while the main loop is
       synchronously loading Cubism and OpenGL resources. */
    SDL_PumpEvents();
    if (app && app->preferences)
        bongo_cat_preferences_model_load_progress(app->preferences, progress);
    /* The main loop is blocked during loading, so advance the visible model. */
    if (!app || !app->loaded_model[0] || !app->model_load_last_frame_ns ||
        !app->live2d)
        return;
    uint64_t now = SDL_GetTicksNS();
    if (progress < .98f && now - app->model_load_last_frame_ns < 16000000ull)
        return;
    uint64_t previous = app->model_load_last_frame_ns;
    app->model_load_last_frame_ns = now;
    float elapsed = (float)((now - previous) / 1000000000.0);
    bongo_cat_app_drain_input(app, false);
    bongo_cat_app_update_hover(app, now);
    if (!app->smoke_freeze_model && elapsed > 0.0f)
        bongo_cat_app_step_live2d(app, elapsed);
    app->last_frame_ns = now;
    bongo_cat_app_render_now(app);
}
