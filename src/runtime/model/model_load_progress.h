#ifndef BONGO_CAT_MODEL_LOAD_PROGRESS_H
#define BONGO_CAT_MODEL_LOAD_PROGRESS_H

#include "bongo_cat/app.h"

void bongo_cat_model_request_frame(BongoCatApp *app, bool reveal);
void bongo_cat_model_runtime_stage(BongoCatApp *app, const char *stage,
    const BongoCatModelEntry *entry);
void bongo_cat_model_load_progress(void *userdata, float progress);

#endif
