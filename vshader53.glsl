/* 
File Name: "vshader53.glsl":
Vertex shader:
  - Per vertex shading for a single point light source;
    distance attenuation is Yet To Be Completed.
  - Entire shading computation is done in the Eye Frame.
*/

#version 150  // YJC: Comment/un-comment this line to resolve compilation errors
              //      due to different settings of the default GLSL version

in  vec4 vPosition;
in  vec3 vNormal;
in  vec2 vTexCoord;

out vec4 color;
out float distance;
out vec2 texCoord;
out vec2 latCoord;

uniform vec4 GlobalProduct, DirAmbientProduct, DirDiffuseProduct, DirSpecularProduct, PosAmbientProduct, PosDiffuseProduct, PosSpecularProduct, NoLightColor;
uniform mat4 ModelView;
uniform mat4 Projection;
uniform mat3 Normal_Matrix;
uniform vec4 LightPosition;   // Must be in Eye Frame
uniform vec4 DirLightDirection, PosLightDirection; // In the Eye Coordinate System
uniform float Shininess;

uniform float ConstAtt;  // Constant Attenuation
uniform float LinearAtt; // Linear Attenuation
uniform float QuadAtt;   // Quadratic Attenuation

uniform float CutOffAngle; // Spotlight Cut Off Angle, in Radians
uniform float SpotLightExponent;

uniform int LightingFlag; // Flag for how handle lighting

uniform int SphereTextureFlag, SphereTextureDirection, SphereTextureFrame; // Flags for how to handle sphere texture

uniform int VertexLatFlag;

void main()
{

    vec3 pos, L, E, H, N;
    vec4 ambient, diffuse, specular, texPos;
    float attenuation, d, s, dist;

    /*-- Lattice Functions --*/

    if (VertexLatFlag == 1) {
        // if latice mode is upright, s = 0.5(x+1), t = 0.5(y+1)
        latCoord.x = 0.5*(vPosition.x + 1.0);
        latCoord.y = 0.5*(vPosition.y + 1.0);
    }
    else if (VertexLatFlag == 2) {
        // if latice mode is tilted, s = 0.3(x+y+z), t = 0.3(x-y+z)
        latCoord.x = 0.3*(vPosition.x + vPosition.y + vPosition.z);
        latCoord.y = 0.3*(vPosition.x - vPosition.y + vPosition.z);
    }

    /*-- Fog Functions --*/

    // converts position to eye coordinates and finds the length of the vector
    // becayse the vector is in eye frame, this is the distance from the eye to the frag
    distance = length(ModelView * vPosition);

    /*-- Texture Functions --*/

    // Define the position of the sphere point to be used for the texture
    if (SphereTextureFrame == 0) {
        texPos = vPosition;
    } 
    else {
        texPos = ModelView * vPosition;
    }

    // Handling sphere texture
    if (SphereTextureFlag == 0) {
        texCoord = vTexCoord;
    }
    else if (SphereTextureFlag == 1) {
        if (SphereTextureDirection == 0) {
            // if sphere texture is vertical use s = 2.5x
            texCoord.x = 2.5 * texPos.x;
        }
        else {
            // if sphere texture is slanted use s = 1.5(x + y + z)
            texCoord.x = 1.5 * (texPos.x + texPos.y + texPos.z);
        }

        // in both cases t = NULL
    }

    else {
        if (SphereTextureDirection == 0) {
            // if sphere texture is vertical use s = 0.5(x+1), t = 0.5(y+1)
            texCoord.x = 0.5 * (texPos.x + 1); 
            texCoord.y = 0.5 * (texPos.y + 1);
        }
        else {
            // if sphere texture is slanted use s = 0.3(x+y+z), t = 0.3(x-y+z)
            texCoord.x = 0.3 * (texPos.x + texPos.y + texPos.z); 
            texCoord.y = 0.3 * (texPos.x - texPos.y + texPos.z);
        }
    }
    

    /*-- Lighting Functions --*/

    switch (LightingFlag) {

        case 0 :

            gl_Position = Projection * ModelView * vPosition;

            color = NoLightColor;

            break;

        case 1 :
            // Positional Spotlight Source

            /*---  Set up Directional Light Source ---*/

            // Transform vertex position into eye coordinates
            pos = (ModelView * vPosition).xyz; // eye position
            
            L = -DirLightDirection.xyz; // vector from point to light source = -distant light direction
            E = normalize(-pos); // vector from the point to the eye
            H = normalize(L + E); // halfway vector

            // Transform vertex normal into eye coordinates
            N = normalize(ModelView * vec4(vNormal, 0.0)).xyz;

            // YJC Note: N must use the one pointing *toward* the viewer
            //     ==> If (N dot E) < 0 then N must be changed to -N
            //
            if (dot(N, E) < 0) N = -N;

            // Attenuation for a Directional Light Source is always 1
            attenuation = 1.0; 

            // Compute terms in the illumination equation
            ambient = DirAmbientProduct;

            d = max(dot(L, N), 0.0);
            diffuse = d * DirDiffuseProduct;

            s = pow(max(dot(N, H), 0.0), Shininess);
            specular = s * DirSpecularProduct;
            
            if( dot(L, N) < 0.0 ) {
                specular = vec4(0.0, 0.0, 0.0, 1.0);
            } 

            // Phong Lighting Model
            color = attenuation * (ambient + diffuse + specular);

            /*---  Set up Spot Light Source ---*/
            // pos, E, N are same as for directional light source

            // distance from point to the light source position
            dist = length(pos - LightPosition.xyz);

            L = normalize(LightPosition.xyz - pos); // light source vector = normalize(eye light position - eye position)
            H = normalize(L + E);
            vec3 Lf = normalize(PosLightDirection.xyz);

            attenuation = 1/(ConstAtt + (LinearAtt*dist) + (QuadAtt*(pow(dist,2))));

            // Compute terms in the illumination equation
            ambient = PosAmbientProduct;

            d = max(dot(L, N), 0.0);
            diffuse = d * PosDiffuseProduct;

            s = pow(max(dot(N, H), 0.0), Shininess);
            specular = s * PosSpecularProduct;
            
            if( dot(L, N) < 0.0 ) {
                specular = vec4(0.0, 0.0, 0.0, 1.0);
            }

            float spotlight_attenuation;
            if (dot(Lf, -L) >= cos(CutOffAngle)) {
                spotlight_attenuation = pow(dot(Lf, -L), SpotLightExponent);
            }
            else {
                spotlight_attenuation = 0.0f;
            }

            attenuation *= spotlight_attenuation;

            color += attenuation * (ambient + diffuse + specular);

            // Add Global Ambient Light
            color += GlobalProduct;

            gl_Position = Projection * ModelView * vPosition;

            break;

        case 2 :

            // Positional Point Light Source

            /*---  Set up Directional Light Source ---*/

            // Transform vertex position into eye coordinates
            pos = (ModelView * vPosition).xyz; // eye position
            
            L = -DirLightDirection.xyz; // vector from point to light source = -distant light direction
            E = normalize(-pos); // vector from the point to the eye
            H = normalize(L + E); // halfway vector

            // Transform vertex normal into eye coordinates
            N = normalize(ModelView * vec4(vNormal, 0.0)).xyz;

            // YJC Note: N must use the one pointing *toward* the viewer
            //     ==> If (N dot E) < 0 then N must be changed to -N
            //
            if (dot(N, E) < 0) N = -N;

            // Attenuation for a Directional Light Source is always 1
            attenuation = 1.0; 

            // Compute terms in the illumination equation
            ambient = DirAmbientProduct;

            d = max(dot(L, N), 0.0);
            diffuse = d * DirDiffuseProduct;

            s = pow(max(dot(N, H), 0.0), Shininess);
            specular = s * DirSpecularProduct;
            
            if( dot(L, N) < 0.0 ) {
                specular = vec4(0.0, 0.0, 0.0, 1.0);
            } 

            // Phong Lighting Model
            color = attenuation * (ambient + diffuse + specular);

            /*---  Set up Point Light Source ---*/
            // pos, E, N are same as for directional light source

            // distance from point to the light source position
            dist = length(pos - LightPosition.xyz);

            L = normalize(LightPosition.xyz - pos); // light source vector = normalize(eye light position - eye position)
            H = normalize(L + E);

            attenuation = 1/(ConstAtt + (LinearAtt*dist) + (QuadAtt*(pow(dist,2))));

            // Compute terms in the illumination equation
            ambient = PosAmbientProduct;

            d = max(dot(L, N), 0.0);
            diffuse = d * PosDiffuseProduct;

            s = pow(max(dot(N, H), 0.0), Shininess);
            specular = s * PosSpecularProduct;
            
            if( dot(L, N) < 0.0 ) {
                specular = vec4(0.0, 0.0, 0.0, 1.0);
            }

            color += attenuation * (ambient + diffuse + specular);

            // Add Global Ambient Light
            color += GlobalProduct;

            gl_Position = Projection * ModelView * vPosition;

            break;


    }


    
}
