#version 450

// Right circular cylinder selection: centered at position, axis along local +Z
// The C++ code applies a pre-rotation so +Z maps to sideways (paint roller orientation)
uniform vec4 selection_cylinder_params; // xyz = center position (world), w = radius
uniform float selection_cylinder_height; // total height along axis
uniform vec4 selection_cylinder_rotation; // quaternion orientation (includes sideways rotation)

/*
//***** begin interface selection_xxxxxx.glsl
bool is_in_selection(in vec4 position);
//***** end interface selection_xxxxxx.glsl *********************************
*/

//***** begin interface of quaternion.glsl ***********************************
vec4 unit_quaternion();
vec3 rotate_vector_with_quaternion(in vec3 preimage, in vec4 q);
vec3 inverse_rotate_vector_with_quaternion(in vec3 v, in vec4 q);
void quaternion_to_axes(in vec4 q, out vec3 x, out vec3 y, out vec3 z);
void quaternion_to_matrix(in vec4 q, out mat3 M);
void rigid_to_matrix(in vec4 q, in vec3 t, out mat4 M);
//***** end interface of quaternion.glsl ***********************************

bool is_in_selection(in vec4 position){
	float radius = selection_cylinder_params.w;
	float half_height = selection_cylinder_height * 0.5;
	// Transform point to cylinder-local space (center at origin, axis along Z)
	vec3 local_pos = inverse_rotate_vector_with_quaternion(position.xyz - selection_cylinder_params.xyz, selection_cylinder_rotation);
	// z must be in [-half_height, +half_height]
	if (local_pos.z < -half_height || local_pos.z > half_height)
		return false;
	// XY distance must be within radius
	float dist_sq = local_pos.x * local_pos.x + local_pos.y * local_pos.y;
	return dist_sq <= radius * radius;
}

bool raycast_selection(in vec3 origin, in vec3 direction, out float t_result){
	return false;
}

bool resolve_collision(in vec4 position, out vec4 new_position, out vec3 collision_normal){
	return false;
}
