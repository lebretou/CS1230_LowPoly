#version 330 core

in vec3 position_world;
in vec3 normal_world;

out vec4 fragColor;

uniform float ka;
uniform float ks;
uniform float kd;

uniform vec4 cAmbient;
uniform vec4 cDiffuse;
uniform vec4 cSpecular;
uniform float shininess;

// light directions and colors
uniform vec3 light_directions[8];
uniform vec3 light_colors[8];
uniform vec3 light_positions[8];
uniform vec3 light_atts[8];
uniform int light_types[8];
uniform float light_angles[8];
uniform float light_penus[8];


// Area light
vec3 areaLightPosition = vec3(0, 3, 2);
vec3 areaLightColor = vec3(0.8, 0.6, 0.4); // Brown light
float areaLightIntensity = 2.0;
float areaLightWidth = 5.0;
float areaLightHeight = 5.0;
vec3 areaLightNormal = normalize(-areaLightPosition); // light faces the origin
vec3 areaLightRight = normalize(cross(vec3(0, 1, 0), areaLightNormal));
vec3 areaLightUp = cross(areaLightNormal, areaLightRight);

uniform int num_lights;

uniform vec3 camera_pos;

vec4 calculateAreaLightContribution() {
    vec4 color = vec4(0.0);
    int samples = 4; // Increase for better quality at the cost of performance
    for (int i = 0; i < samples; i++) {
        for (int j = 0; j < samples; j++) {
            float s = (float(i) + 0.5) / float(samples);
            float t = (float(j) + 0.5) / float(samples);
            vec3 samplePos = areaLightPosition + (s - 0.5) * areaLightWidth * areaLightRight + (t - 0.5) * areaLightHeight * areaLightUp;
            vec3 toSample = samplePos - position_world;
            float distanceToSample = length(toSample);
            toSample = normalize(toSample);

            // Calculate the contribution from this sample
            // Diffuse
            float ndotl = max(dot(normal_world, toSample), 0.0);
            color += kd * ndotl * vec4(areaLightColor, 1.0) * areaLightIntensity / (distanceToSample * distanceToSample);

            // Specular
            vec3 reflected = reflect(-toSample, normal_world);
            float specAngle = max(dot(reflected, normalize(camera_pos - position_world)), 0.0);
            color += ks * pow(specAngle, shininess) * vec4(areaLightColor, 1.0) * areaLightIntensity / (distanceToSample * distanceToSample);
        }
    }
    return color / float(samples * samples);
}





void main() {

    fragColor = vec4(ka * cAmbient[0],
                     ka * cAmbient[1],
                     ka * cAmbient[2],
                     1);


    // Area light contribution
    fragColor += calculateAreaLightContribution();


}
