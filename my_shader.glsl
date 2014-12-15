#version 330

#ifdef VERTEX_SHADER

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 3) in vec3 texcoord;

uniform mat4 mvpMatrix;

out vec3 vertex_position;
out vec3 vertex_normal;
out vec2 vertex_texcoord;

void main()
{
  gl_Position = mvpMatrix * vec4(position, 1.0);

  vertex_position = position;
  vertex_normal = normal;
  vertex_texcoord = normalize(texcoord).st;
}

#endif

#ifdef FRAGMENT_SHADER

in vec3 vertex_position;
in vec3 vertex_normal;
in vec2 vertex_texcoord;

uniform sampler2D image;

out vec4 fragment_color;

void main( )
{
  vec3 n = normalize(vertex_normal);

  fragment_color.rgb = texture(image, vertex_texcoord).rgb * abs(n.z);
}

#endif
