uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;

attribute vec2 vertTexCoord;
attribute vec3 tangent;
attribute vec3 bitangent;

varying vec3 fragVert;
varying vec2 fragTexCoord;
varying vec3 fragNormal;
varying mat3 toTangentSpace;

void main(void) {
    fragTexCoord = vertTexCoord;
    fragNormal = vec3(gl_Normal);
    fragVert = vec3(gl_Vertex);

    gl_Position = projection * view * model * gl_Vertex;

    vec3 norm = normalize((view * model * vec4(fragNormal, 0.0)).xyz);
    vec3 tang = normalize((view * model * vec4(tangent, 0.0)).xyz);
    vec3 bitang = normalize((view * model * vec4(bitangent, 0.0)).xyz);

    toTangentSpace = mat3(
                tang.x, bitang.x, norm.x,
                tang.y, bitang.y, norm.y,
                tang.z, bitang.z, norm.z);
}
