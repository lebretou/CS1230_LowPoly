#version 330 core
layout (location = 0) in vec3 position_object;
layout (location = 1) in vec3 normal_object;

uniform mat4 model_matrix;
uniform mat4 model_matrix_inverse;
uniform mat3 model_matrix_inv_trans;

uniform mat4 model_view;
uniform mat4 model_proj;

out vec3 position_world;
out vec3 normal_world;



void main() {
//    position_world = mat3(model_matrix) * position_object;
    position_world = vec3(model_matrix * vec4(position_object, 1));
    normal_world = normalize(model_matrix_inv_trans * normalize(normal_object));


    gl_Position = model_proj * model_view * model_matrix * vec4(position_object, 1.0);
//    gl_Position = model_matrix * model_view * model_proj * vec4(position_object, 1.0);

}
