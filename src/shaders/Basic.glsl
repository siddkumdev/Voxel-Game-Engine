#shader vertex
#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 Normal;
out vec3 FragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {

    FragPos = vec3(model * vec4(aPos, 1.0));

    Normal = mat3(transpose(inverse(model))) * aNormal;

    gl_Position = projection * view * vec4(FragPos, 1.0);
}

#shader fragment
#version 460 core
out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;

uniform vec3 uColor;

void main() {

    vec3 lightDir = normalize(vec3(0.5, 1.0, 0.7)); 

    vec3 norm = normalize(Normal);

    float diff = max(dot(norm, lightDir), 0.0);

    float ambient = 0.4;

    vec3 lighting = (ambient + diff) * vec3(1.0);

    FragColor = vec4(lighting * uColor, 1.0);
}
