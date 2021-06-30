#version 330 core

// ____ CONSTANTS ____
const uint MAX_SHADOWS = 15u;


// ____ DATATYPES ____
struct Shadow
{
	bool active;
	mat4 viewProjectionMatrix;
	sampler2D shadowMap;
};


// ____ GLOBAL VARIABLES ____
// Input data from the vertex shader
in vec3 vsPosition;

// Uniform variables
uniform sampler2D uDepthTexture;
uniform mat4 uCameraViewProjectionMatrix;
uniform Shadow uShadows[MAX_SHADOWS];


// ____ FUNCTION DEFINITIONS ____
/*  Returns the location in world space of the vertex located at the given
 * position in the depth buffer */
vec3 decodeLocation(vec2 texCoords)
{
	vec4 clipSpaceLocation;
	clipSpaceLocation.xy = 2.0 * texCoords - 1.0;
	clipSpaceLocation.z = 2.0 * texture(uDepthTexture, texCoords).r - 1.0;
	clipSpaceLocation.w = 1.0;
	vec4 homogenousLocation = uCameraViewProjectionMatrix * clipSpaceLocation;
	return homogenousLocation.xyz / homogenousLocation.w;
}


/* Calculates wether the given point is in shadow or not */
float calculateShadow(uint shadowIndex, vec3 position)
{
	vec4 shadowPosition = uShadows[shadowIndex].viewProjectionMatrix * vec4(position, 1.0);
	vec3 projCoords = shadowPosition.xyz / shadowPosition.w;	// Perspective divide
	projCoords = 0.5 * projCoords + 0.5;						// [0,1] range

    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(uShadows[shadowIndex].shadowMap, projCoords.xy).r;
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // check whether current frag pos is in shadow
    return currentDepth > closestDepth? 1.0 : 0.0;
}


// ____ MAIN PROGRAM ____
void main()
{
	// Extract the data from the depth buffer
	vec2 texCoords = (0.5 * vsPosition + 0.5).xy;
	vec3 position = decodeLocation(texCoords);

	// Calculate the shadows
	float shadow = 0.0;
	for (uint i = 0u; i < MAX_SHADOWS; ++i) {
		if (uShadows[i].active) {
			shadow += calculateShadow(i, position);
		}
	}

	gl_FragDepth = shadow;
}
