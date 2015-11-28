#version 120

uniform mat4 model;
uniform mat3 normalMatrix;
uniform sampler2D texture;
uniform sampler2D normalMap;

uniform vec3 lightIntensities;
uniform vec3 lightPosition;
uniform bool normalsEnabled;

varying vec2 fragTexCoord;
varying vec3 fragNormal;
varying vec3 fragVert;
varying mat3 toTangentSpace;

void main(void) {
    float x = fragTexCoord.x;
    float y = 1.0 - fragTexCoord.y;

    vec3 norm = texture2D(normalMap, vec2(x, y)).rgb * 2.0 - 1.0;

    vec3 normal = normalize(normalMatrix * fragNormal);
    vec3 fragPosition = vec3(model * vec4(fragVert, 1));
    vec3 surfaceToLight = lightPosition - fragPosition;

    float brightness = dot(normal, surfaceToLight) / (length(surfaceToLight) * length(normal));
    brightness = clamp(brightness, 0.05, 1);

    float brightness1 = dot(norm, surfaceToLight) / (length(surfaceToLight) * length(norm));
    if (brightness1 < brightness) {
        brightness = brightness1;
    }

    vec4 surfaceColor = texture2D(texture, vec2(x, y));
    gl_FragColor = vec4(brightness * lightIntensities * surfaceColor.rgb, surfaceColor.a);
}
