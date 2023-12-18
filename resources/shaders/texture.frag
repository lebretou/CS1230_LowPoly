#version 330 core

// Task 16: Create a UV coordinate in variable
in vec2 uvs;

// Task 8: Add a sampler2D uniform
uniform sampler2D my_texture;

// Task 29: Add a bool on whether or not to filter the texture
uniform bool post_pro;
uniform bool blur;

uniform float width;
uniform float height;

uniform float qLevel = 30.0;

out vec4 fragColor;

void main()
{
//    fragColor = vec4(1);
    // Task 17: Set fragColor using the sampler2D at the UV coordinate
    fragColor = texture(my_texture, uvs);

    // Task 33: Invert fragColor's r, g, and b color channels if your bool is true
    if (post_pro) {
//        fragColor = vec4(1.0 - fragColor.r, 1.0 - fragColor.g, 1.0 - fragColor.b, 1.0 - fragColor.a);
        fragColor = floor(fragColor* qLevel) / qLevel;
    }

    if (blur) {
        float uOffset = 1.0 / width;
        float vOffset = 1.0 / height;

        vec4 blurColor = vec4(0.0);
        for (int i = -2; i <= 2; ++i) {
            for (int j = -2; j <= 2; ++j) {
                vec2 offset = vec2(i * uOffset, j * vOffset);
                blurColor += texture(my_texture, uvs + offset) * (1.0 / 25.0); // Kernel weight for box blur is 1/25
            }
        }
        fragColor = blurColor;
    }

}
