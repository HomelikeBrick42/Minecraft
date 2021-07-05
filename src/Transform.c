#include "Transform.h"

void Transform_ToMatrix(Transform* transform, mat4 outMatrix) {
    glm_mat4_identity(outMatrix);
    glm_translate(outMatrix, transform->Position);
    glm_rotate(outMatrix, transform->Rotation[1] * cast(f32) (M_PI / 180.0), (vec3){ 0.0f, 1.0f, 0.0f });
    glm_rotate(outMatrix, transform->Rotation[0] * cast(f32) (M_PI / 180.0), (vec3){ 1.0f, 0.0f, 0.0f });
    glm_rotate(outMatrix, transform->Rotation[2] * cast(f32) (M_PI / 180.0), (vec3){ 0.0f, 0.0f, 1.0f });
    glm_scale(outMatrix, transform->Scale);
}
