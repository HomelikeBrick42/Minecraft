#pragma once

#include "Typedefs.h"
#include "Transform.h"

typedef struct Camera {
    Transform Transform;
    mat4 ProjectionMatrix;
} Camera;
