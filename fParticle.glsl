/* 
File Name: "fParticle.glsl":
           Fragment Shader for Particles
*/

#version 150

in vec4 position;
in vec4 color;

out vec4 fColor;

void main() 
{ 
	if (position.y < 0.1) {
		discard;
	}
	
	fColor = color;
}