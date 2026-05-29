#version 450

// Pointy cone selection: base (circle) at position, narrows along -Z to apex (outward from hand)
// base_radius = height / 4 (base diameter = half of height)
uniform vec4 selection_cone_sphere; // xyz = base center position (world), w = height
uniform vec4 selection_cone_rotation; // quaternion orientation

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
	float height = selection_cone_sphere.w;
	// Transform point to cone-local space (base at origin, apex at -Z*height)
	vec3 local_pos = inverse_rotate_vector_with_quaternion(position.xyz - selection_cone_sphere.xyz, selection_cone_rotation);
	// z must be in [-height, 0] (cone extends along -Z direction, outward from hand)
	if (local_pos.z > 0.0 || local_pos.z < -height)
		return false;
	// At depth z, the allowed radius shrinks linearly from base_radius (at z=0) to 0 (at z=-height)
	// base_radius = height / 4 (pointy cone: diameter = half of height)
	float base_radius = height * 0.25;
	float allowed_radius = base_radius * (1.0 + local_pos.z / height);
	float dist_sq = local_pos.x * local_pos.x + local_pos.y * local_pos.y;
	return dist_sq <= allowed_radius * allowed_radius;
}

bool raycast_selection(in vec3 origin, in vec3 direction, out float t_result){
	return false;
}

bool resolve_collision(in vec4 position, out vec4 new_position, out vec3 collision_normal){
	return false;
}
