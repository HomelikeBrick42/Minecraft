#pragma once

#include "Typedefs.h"
#include <cglm/cglm.h>

typedef struct Transform {
    vec3 Position;
    vec3 Rotation;
    vec3 Scale;
} Transform;

void Transform_ToMatrix(Transform* transform, mat4 outMatrix);
