#shader vertex
#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal; // <--- NEW: Receive Normal data from Chunk

out vec3 Normal;
out vec3 FragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    // Calculate position in world space
    FragPos = vec3(model * vec4(aPos, 1.0));
    
    // Calculate Normal Matrix to keep normals correct if you rotate/scale the chunk
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
    // --- Basic Lighting ---
    // This makes the cubes look 3D instead of flat
    
    // 1. Define a light direction (like a sun)
    vec3 lightDir = normalize(vec3(0.5, 1.0, 0.7)); 
    
    // 2. Normalize input normal
    vec3 norm = normalize(Normal);
    
    // 3. Calculate Diffuse lighting (how much the face faces the light)
    float diff = max(dot(norm, lightDir), 0.0);
    
    // 4. Ambient lighting (minimum brightness so shadows aren't pitch black)
    float ambient = 0.4;
    
    // Combine
    vec3 lighting = (ambient + diff) * vec3(1.0); // White light
    
    FragColor = vec4(lighting * uColor, 1.0);
}