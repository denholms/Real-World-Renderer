precision mediump float;
in vec3 Color;
in vec2 Texcoord;
in vec3 normalInterp;
in vec3 vertPos;
in vec3 Position;

out vec4 outColor;

uniform sampler2D tex;
uniform sampler2D grassTex;
uniform sampler2D rockTex;
uniform sampler2D sandTex;
uniform sampler2D snowTex;
uniform sampler2D waterTex;

const vec3 lightPos = vec3(1.0,1.0,1.0);
const vec3 ambientColor = vec3(0.2,0.0,0.0);
const vec3 diffuseColor = vec3(1.0, 1.0, 1.0);
const vec3 specColor = vec3(2.0, 2.0, 2.0);
const float shininess = 30.0;
const float screenGamma = 2.2;

void main()
{
	vec3 normal = normalize(normalInterp);
	vec3 lightDir = normalize(lightPos - vertPos);

	float lambertian = max(dot(lightDir, normal), 0.0);
	float specular = 0.0;

	if (lambertian > 0.0) {
		vec3 viewDir = normalize(-vertPos);
		vec3 reflectDir = reflect(-lightDir, normal);
		float specAngle = max(dot(reflectDir, viewDir), 0.0);
		specular = pow(specAngle, shininess/4.0);
	}
	vec3 colorLinear = ambientColor + lambertian * diffuseColor + specular * specColor + Color;
	if (Position.y > 7.5) {
		outColor = texture(snowTex, Texcoord * 5);
	} else if (Position.y > 4) {
		outColor = texture(rockTex, Texcoord * 5);		 // * vec4(colorLinear, 1.0);
	} else if (Position.y > 2.5) {
		outColor = texture(grassTex, Texcoord * 5);
	} else if (Position.y > 1.5) {
		outColor = texture(sandTex, Texcoord * 5);
	} else {
		outColor = texture(waterTex, Texcoord * 5);
	}
}