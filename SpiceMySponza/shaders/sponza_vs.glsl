#version 330

// TODO: add uniforms to take in the matrix
uniform mat4 projection_xform;
uniform mat4 view_xform;
uniform mat4 proj_view_xform;
uniform mat4 trans_xform;
uniform mat4 combined_xform;

// TODO: add in variables for each of the streamed attributes
in vec3 vertex_position;
in vec3 vertex_normal;
in vec2 vertex_texture;

// TODO: specify out variables to be varied to the FS
out vec3 fragment_position;
out vec3 fragment_normal;
out vec2 fragment_texcoord;

void main(void)
{
	fragment_position = mat4x3(trans_xform) * vec4(vertex_position, 1.0);
	fragment_normal = normalize(mat3(trans_xform) * vertex_normal);
	fragment_texcoord = vertex_texture;

	gl_Position = vec4(combined_xform * vec4(vertex_position, 1.0));
}
