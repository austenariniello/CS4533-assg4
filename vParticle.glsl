/*
File Name: "vParticle.glsl":
           Vertex Shader for Particles
*/

#version 150

in vec4 vColor;
in vec4 vVelocity;

uniform float t_0; // time when animation was started
uniform float t_1; // time of current animation frame

uniform mat4 ModelView;
uniform mat4 Projection;

out vec4 position;
out vec4 color;

void main()
{

    vec4 pos;

    float t_max = 10.0 * 1000.0;

    float t = t_1 - t_0; // time elapsed since animation started

    if (t > t_max) {
        int i = 1;
        while ((t - t_max*i) > 0) {
            i++;
        }
        i--;
        t -= t_max*i;
    }

    float a = -0.00000049; // vertical particle acceleration
    vec4 position_0 = vec4(0.0, 0.1, 0.0, 1.0);

    pos.x = position_0.x + 0.001 * vVelocity.x * t;
    pos.z = position_0.z + 0.001 * vVelocity.z * t;
    pos.y = position_0.y + 0.001 * vVelocity.y * t + 0.5 * a * t * t;
    pos.w = 1.0;

    gl_Position = Projection * ModelView * pos;

    position = pos;

    color = vColor;
}