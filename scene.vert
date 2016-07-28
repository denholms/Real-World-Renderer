in vec3 position;
in vec3 color;
in vec2 texcoord;
in vec3 normal;

out vec3 Color;
out vec2 Texcoord;
out vec3 normalInterp;
out vec3 vertPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
uniform vec3 overrideColor;
uniform sampler2D tex;

void main()
{
	float texVal = texture(tex, texcoord).r;
	mat4 modelView = view * model;
	mat4 normalMatrix = transpose(inverse(modelView));
	vec4 vertPos4 = modelView * vec4(position, 1.0);
	vertPos = vec3(vertPos4) / vertPos4.w;
	normalInterp = vec3(normalMatrix * vec4(normal, 0.0));
	Color = overrideColor * color;
	//Color = overrideColor * vec4(position, 1.0);
	Texcoord = texcoord;
	gl_Position = proj * modelView * vec4(position.x, (position.y + texVal) * 5, position.z , 1.0);
}