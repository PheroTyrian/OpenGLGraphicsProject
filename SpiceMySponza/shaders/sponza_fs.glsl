#version 330

struct Light
{
	vec3 position;
	float range;
	vec3 intensity;
};

uniform vec3 camera_position;
uniform Light light_sources[25];
uniform vec3 ambient_light_intensity;
uniform int numLights;
uniform bool hasDiffuse;
uniform bool hasSpecular;
uniform sampler2D diffuse_texture;
uniform sampler2D specular_texture;
uniform vec3 ambient_colour;
uniform vec3 diffuse_colour;
uniform vec3 specular_colour;
uniform float material_shininess;
uniform bool isShiny;


in vec3 fragment_position;
in vec3 fragment_normal;
in vec2 fragment_texcoord;

out vec4 fragment_colour;

vec3 diffuse(vec3 lightPosition, float lightRange, vec3 lightIntensity, vec3 lightDirection, vec3 objectNormal, vec3 objectPosition, vec3 diffuseColour)
{
	float M = smoothstep(lightRange, lightRange / 2, length(lightPosition - objectPosition));
	if (M == 0) {
		return vec3(0.0, 0.0, 0.0);
	}

	float diffuse = max(0, dot(lightDirection, objectNormal));

	return (lightIntensity * diffuse * M * diffuseColour);
}

vec3 specular(vec3 lightPosition, float lightRange, vec3 lightIntensity, vec3 lightDirection, vec3 objectNormal, vec3 objectPosition, vec3 cameraPosition, vec3 specularColour, float shininess)
{
	float M = smoothstep(lightRange, lightRange / 2, length(lightPosition - objectPosition));
	if (M == 0) {
		return vec3(0.0, 0.0, 0.0);
	}

	vec3 reflected = 2 * objectNormal * dot(objectNormal, lightDirection) - lightDirection;
	vec3 cameraDirection = normalize(cameraPosition - objectPosition);
	vec3 finalIntensity = lightIntensity * specularColour * pow(max(0, dot(cameraDirection, reflected)), shininess);

	return (finalIntensity * M);
}

void main(void)
{
	vec3 normal = normalize(fragment_normal);
	vec3 cameraPosition = camera_position;

	vec3 ambientColour;
	vec3 diffuseColour;
	vec3 specularColour;
	
	if (hasDiffuse){
		diffuseColour = vec3(texture(diffuse_texture, fragment_texcoord));
		ambientColour = diffuseColour * ambient_colour;
		diffuseColour *= diffuse_colour;}
	else{
		ambientColour = ambient_colour;
		diffuseColour = diffuse_colour;}

	if (hasSpecular){
		specularColour = specular_colour * vec3(texture(specular_texture, fragment_texcoord));}
	else{
		specularColour = specular_colour;}

	vec3 sumDiffuse = vec3(0.0, 0.0, 0.0);
	vec3 sumSpecular = vec3(0.0, 0.0, 0.0);

	for (int i = 0; i < numLights; i++)
	{
		vec3 lightDirection = normalize(light_sources[i].position - fragment_position);
		if( dot(lightDirection, normal) < 0.0 )
		{
			continue; 
		}
		
		sumDiffuse += 0.8 * diffuse(light_sources[i].position, light_sources[i].range, light_sources[i].intensity, lightDirection, normal, fragment_position, diffuseColour);

		if (isShiny)
		{
			sumSpecular += specular(light_sources[i].position, light_sources[i].range, light_sources[i].intensity, lightDirection, normal, fragment_position, cameraPosition, specularColour, material_shininess);
		}
	}
	
	vec3 final =  ambient_light_intensity * ambientColour * 0.5 + sumDiffuse + sumSpecular;

    fragment_colour = vec4(final, 1.0);
}
