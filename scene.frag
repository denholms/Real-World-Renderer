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
	if (Position.y > 0.8) {
		outColor = texture(rockTex, Texcoord);		 // * vec4(colorLinear, 1.0);
	} else {
		outColor = texture(grassTex, Texcoord);
	}
}