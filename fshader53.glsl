/* 
File Name: "fshader53.glsl":
           Fragment Shader
*/

#version 150  // YJC: Comment/un-comment this line to resolve compilation errors
              //      due to different settings of the default GLSL version

uniform vec4 FogColor;
uniform int FogFlag, GroundTextureFlag, SphereTextureFragFlag, FragLatFlag;
uniform float FogStart, FogEnd, FogDensity;
uniform sampler2D texture_2D;
uniform sampler1D texture_1D;

in  vec4 color;
in  float distance;
in  vec2 texCoord;
in  vec2 latCoord;

out vec4 fColor;

void main() 
{ 
    float f;
    float opacity = color.w;
    vec4 currColor, texColor;

    /*-- Lattice Functions --*/

    if (FragLatFlag > 0) {
        if ((fract(4*latCoord.x) < 0.35) && (fract(4*latCoord.y) < 0.35)) {
            discard;
        }
    }

    /*-- Texture Functions --*/

    currColor = color;

    if (GroundTextureFlag == 1) {
        texColor = texture(texture_2D, texCoord);
    }

    if (SphereTextureFragFlag == 1) {
        texColor = texture(texture_1D, texCoord.x);
    }
    else if (SphereTextureFragFlag == 2) {
        texColor = texture(texture_2D, texCoord);
        if (texColor.y < 0.6) {
            // if green, make red
            texColor = vec4(0.9, 0.1, 0.1, 1.0);
        }
    }

    if ((GroundTextureFlag > 0) || (SphereTextureFragFlag > 0)){
        currColor *= texColor;
    }

    /*-- Fog Functions --*/

    if (FogFlag == 0) {
        // No Fog
        f = 1;
    }
    else if (FogFlag == 1) {
        // Linear Fog
        f = (FogEnd - distance) / (FogEnd - FogStart);
        f = clamp(f, 0, 1);
    }
    else if (FogFlag == 2) {
        // Exponential Fog
        f = exp(-(FogDensity * distance));
        f = clamp(f, 0, 1);

    }
    else if (FogFlag == 3) {
        // Gaussian Fog
        f = exp(-(pow((FogDensity * distance),2)));
        f = clamp(f, 0, 1);
    }

    fColor = mix(FogColor, currColor, f);
    fColor.w = opacity;

}

