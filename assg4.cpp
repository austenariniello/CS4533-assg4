/************************************************************
 * Assignment 4: Textured Sphere
**************************************************************/
#include "Angel-yjc.h"
#include "texmap.c"
using namespace std;

typedef Angel::vec3  color3;
typedef Angel::vec3  point3;
typedef Angel::vec4  color4;
typedef Angel::vec4  point4;

struct polygon {
    int n; // Number of vertices (should always be three)
    point4 vertices[3]; // Array of vertices, will always be three
    vec3 flat_normal; // Outward Unit-Length Normal Vector for Flat Shading
    vec3 smooth_normal[3]; // Outward Unit-Length Normal Vectors for Smooth Shading
};
int n_polyons; // Num of polygons in the object
vector<polygon> polygons; // Vector of polygons in the object
int sphere_NumVertices;

int r = 1; // Sphere Unit Radius

void fileIn(); // Used to read in a file

GLuint Angel::InitShader(const char* vShaderFile, const char* fShaderFile);

GLuint program;       /* shader program object id */
GLuint floor_buffer;  /* vertex buffer object id for floor */
GLuint sphere_buffer_flat; /* vertex buffer object id for sphere with flat shading*/
GLuint sphere_buffer_smooth; /* vertex buffer object id for sphere with smooth shading */

GLuint SphereProgram;  /* sphere shader program object id */
GLuint FloorProgram;   /* floor shader program object id */
GLuint NoLightProgram; /* no light shader program object id */
GLuint ParticleProgram; /* particle program object id */

GLuint xaxis_buffer; /* vertex buffer object id for x axis */
GLuint yaxis_buffer; /* vertex buffer object id for y axis */
GLuint zaxis_buffer; /* vertex buffer object id for z axis */

GLuint shadow_buffer; /* vertex buffer object id for shadow */

GLuint particle_buffer; /* vertex buffer object id for particles */

/*--- Set Up VPN, VRP, VUP for calculations ---*/
vec4 VPN(-7.0, -3.0, 10.0, 0.0);
vec4 VRP(7.0, 3.0, -10.0, 1.0);
vec4 VUP(0.0, 1.0, 0.0, 0.0);

/*--- Set Up Point A,B,C for Travelling ---*/
vec3 A(3.0, 1.0, 5.0);
vec3 B(-2.0, 1.0, -2.5);
vec3 C(2.0, 1.0, -4.0);

/*--- Set Up Vecs AB, BC, CA for Calculations ---*/
vec3 AB = B - A;
vec3 BC = C - B;
vec3 CA = A - C;

vec3 OY(0.0, 1.0, 0.0); // Normal Vector to Ground
vec3 trans_vec = A; // Translation Vector, Initially Starts at A
int trans_mode = 0; // Translation mode for animation. 0 = AB, 1 = BC, 2 = CA

/*--- Set Up Rotation Vectors for Travelling ---*/
vec3 AB_rotation_axis = cross(OY, AB);
vec3 BC_rotation_axis = cross(OY, BC);
vec3 CA_rotation_axis = cross(OY, CA);

vec3 curr_rotation_axis = AB_rotation_axis; // Initially using AB_rotation_axis

/*--- Set Up M as 4x4 Indentity Matrix ---*/
mat4 M(1.0, 0.0, 0.0, 0.0,
       0.0, 1.0, 0.0, 0.0,
       0.0, 0.0, 1.0, 0.0,
       0.0, 0.0, 0.0, 1.0);

/*--- Set Up L as the fixed light source ---*/
vec4 L(-14.0, 12.0, -3.0, 1.0);

/*--- Set Up N as the matrix to perform shadow projection ---*/
mat4 N(vec4(L.y, -L.x,   0,   0),
       vec4( 0,    0,    0,   0), 
       vec4( 0,  -L.z,  L.y,  0), 
       vec4( 0,   -1,    0,  L.y));

// Projection transformation parameters
GLfloat  fovy = 45.0;  // Field-of-view in Y direction angle (in degrees)
GLfloat  aspect;       // Viewport aspect ratio
GLfloat  zNear = 2.0, zFar = 35.0;

GLfloat angle = 0.0; // rotation angle in degrees
vec4 init_eye = VRP; // initial viewer position (VRP)
vec4 eye = init_eye; // current viewer position

int animationFlag = 0; // 1: animation; 0: non-animation. Toggled by right mouse click
int beginFlag = 0; //  1: animation began; 0: animation has not began. Toggled by key 'b' or 'B'
int solidityFlag = 0; // 1: draws a solid sphere, 0: draws a wireframe sphere
int shadowFlag = 0; // 1: sphere casts a shadow, 0: sphere does not cast a shadow
int lightingFlag = 0; // 2: lighting is enabled with a point source 1: lighting is enabled with a spot light, 0: lighting is disableds
int shadingFlag = 0; // 0: No Shading, 1: Smooth Shading, 2: Flat Shading
int fogFlag = 0; // 0: No Fog, 1: Linear Fog, 2: Exponential Fog, 3: Gaussian Fog
int shadowBlendingFlag = 0; // 0: Shadow Blending Off, 1: Shadow Blending On
int groundTextureFlag = 0; // 0: No ground texture, 1: ground texture enabled
int sphereTextureFlag = 0; // 0: No sphere texture, 1: 1D sphere texture, 2: 2D sphere texture
int sphereTextureDirection = 0; // 0: vertical direction, 1: slanted direction
int sphereTextureFrame = 0; // 0: object frame, 1: eye frame
int latFlag = 0; // 0: lattice effect off, 1: lattice effect upright, 2: lattice effect tilted
int particleFlag = 0; // 0: firework animation off, 1: firework animation on

vector<point4> sphere_points; // positions for all sphere vertices
vector<vec3> sphere_normals_flat; // unit normal vector for all sphere polygons (flat)
vector<vec3> sphere_normals_smooth; // unit normal vector for all sphere vertices (smooth)

vector<point4> shadow_points; // positions for all shadow vertices
vector<vec3> shadow_normals; // normals for all shadow vertices (junk)
color4 shadow_color = color4(0.25, 0.25, 0.25, 0.65); // color for all shadow points

point4 xaxis_points[2]; // positions for x axis vertices
color4 xaxis_color = color4(1.0f, 0.0f, 0.0f, 1.0f); // color for all x axis vertices
vec3 xaxis_normals[2]; // normals for x axis vertices (junk)

point4 yaxis_points[2]; // positions for y axis vertices
color4 yaxis_color = color4(1.0f, 0.0f, 1.0f, 1.0f); // color for all y axis vertices
vec3 yaxis_normals[2]; // normals for y axis vertices (junk)

point4 zaxis_points[2]; // positions for z axis vertices
color4 zaxis_color = color4(0.0f, 0.0f, 1.0f, 1.0f); // color for all y axis vertices
vec3 zaxis_normals[2]; // normals for z axis vertices (junk)

const int floor_NumVertices = 6; //(1 face)*(2 triangles/face)*(3 vertices/triangle)
point4 floor_points[floor_NumVertices]; // positions for all floor vertices
vec3 floor_normals[floor_NumVertices]; // unit normal vector for all floor polygons
vec2 floor_texCoord[floor_NumVertices];

/*----- Shader Lighting Parameters -----*/

    // Global Ambient Light
    color4 gl_light_ambient( 1.0f, 1.0f, 1.0f, 1.0f );

    // Directional Light Parameters
    color4 dir_light_ambient( 0.0f, 0.0f, 0.0f, 1.0f );
    color4 dir_light_diffuse( 0.8f, 0.8f, 0.8f, 1.0f );
    color4 dir_light_specular( 0.2f, 0.2f, 0.2f, 1.0f );

    vec4 dir_light_direction(0.1f, 0.0f, -1.0f, 0.0f ); // In Eye Frame.

    // Positional Light Parameters
    color4 pos_light_ambient( 0.0f, 0.0f, 0.0f, 1.0f );
    color4 pos_light_diffuse( 1.0f, 1.0f, 1.0f, 1.0f );
    color4 pos_light_specular( 1.0f, 1.0f, 1.0f, 1.0f );

    vec4 pos_light_position(-14.0f, 12.0f, -3.0f, 1.0f); // In World Coordinate

    // Additional Spot Light Parameters
    vec4 spot_light_direction = vec4(-6.0f, 0.0f, -4.5f, 1.0f) - pos_light_position; // In World Coordinate
    float cut_off_angle = DegreesToRadians*20.0; // Converting from Degrees to Radians
    float spot_light_exponent = 15.0;

    float const_att = 2.0f;
    float linear_att = 0.01f;
    float quad_att = 0.001f;

    // Floor material Parameters
    color4 floor_no_light_color( 0.0f, 1.0f, 0.0f, 1.0f);
    color4 floor_material_ambient( 0.2f, 0.2f, 0.2f, 1.0f );
    color4 floor_material_diffuse( 0.0f, 1.0f, 0.0f, 1.0f );
    color4 floor_material_specular( 0.0f, 0.0f, 0.0f, 1.0f );
    float  floor_material_shininess = 0.0f;

    // Sphere material Parameters
    color4 sphere_no_light_color( 1.0f, 0.84f, 0.0f, 1.0f);
    color4 sphere_material_ambient( 0.2f, 0.2f, 0.2f, 1.0f );
    color4 sphere_material_diffuse( 1.0f, 0.84f, 0.0f, 1.0f );
    color4 sphere_material_specular( 1.0f, 0.84f, 0.0f, 1.0f );
    float  sphere_material_shininess = 125.0f;

/*----- Calculating Object Lighting Parameters -----*/

    /*
    color4 ambient_product = light_ambient * material_ambient;
    color4 diffuse_product = light_diffuse * material_diffuse;
    color4 specular_product = light_specular * material_specular;
    */

    // Global Ambient Light Values
    color4 floor_global_res = gl_light_ambient * floor_material_ambient;
    color4 sphere_global_res = gl_light_ambient * sphere_material_ambient;

    // Direction Ambient Light Values
    color4 dir_floor_ambient_res = dir_light_ambient * floor_material_ambient; 
    color4 dir_floor_diffuse_res = dir_light_diffuse * floor_material_diffuse;
    color4 dir_floor_specular_res = dir_light_specular * floor_material_specular;

    color4 dir_sphere_ambient_res = dir_light_ambient * sphere_material_ambient;
    color4 dir_sphere_diffuse_res = dir_light_diffuse * sphere_material_diffuse;
    color4 dir_sphere_specular_res = dir_light_specular * sphere_material_specular;

    // Positional Ambient Light Values
    color4 pos_floor_ambient_res = pos_light_ambient * floor_material_ambient; 
    color4 pos_floor_diffuse_res = pos_light_diffuse * floor_material_diffuse;
    color4 pos_floor_specular_res = pos_light_specular * floor_material_specular;

    color4 pos_sphere_ambient_res = pos_light_ambient * sphere_material_ambient;
    color4 pos_sphere_diffuse_res = pos_light_diffuse * sphere_material_diffuse;
    color4 pos_sphere_specular_res = pos_light_specular * sphere_material_specular;

/*----- Setting Up Fog Parameters -----*/

    float fog_start = 0.0;
    float fog_end = 18.0;
    float fog_density = 0.09;

    color4 fog_color(0.7, 0.7, 0.7, 0.5);

/*----- Setting Up Texture Parameters -----*/

    static GLuint texName;

/*----- Setting Up Particle Parameters -----*/

    // Number of Particles
    const int ParticlesN = 300;

    vec4 particleVelocity[ParticlesN];
    color4 particleColor[ParticlesN];

    // Time When Animation Started
    float t_0, t_1;


//-------------------------------
// generate 2 triangles: 6 vertices and 6 colors
void floor()
{

    point4 a = point4(5.0, 0.0, 8.0, 1.0);
    point4 b = point4(-5.0, 0.0, 8.0, 1.0);
    point4 c = point4(-5.0, 0.0, -4.0, 1.0);
    point4 d = point4(5.0, 0.0, -4.0, 1.0);

    vec4 u = b - a;
    vec4 v = d - a;

    vec3 normal = normalize(cross(u, v));

    // floor texture s:[0.0,1.5] t:[0.0,1.25]
    // t: ab, cd; s: bc, da

    floor_normals[0] = normal; floor_points[0] = a; floor_texCoord[0] = vec2(0.0, 0.0);
    floor_normals[1] = normal; floor_points[1] = b; floor_texCoord[1] = vec2(0.0, 1.25);
    floor_normals[2] = normal; floor_points[2] = c; floor_texCoord[2] = vec2(1.5, 1.25);

    floor_normals[3] = normal; floor_points[3] = a; floor_texCoord[3] = vec2(0.0, 0.0);
    floor_normals[4] = normal; floor_points[4] = c; floor_texCoord[4] = vec2(1.5, 1.25);
    floor_normals[5] = normal; floor_points[5] = d; floor_texCoord[5] = vec2(1.5, 0.0);


}
//----------------------------------------------------------------------------
// generate sphere: n_polygons triangles: 3 * n_polygons vertices
void sphere() {
    for ( polygon p : polygons ) {

        sphere_normals_flat.push_back(p.flat_normal); sphere_points.push_back(p.vertices[0]); sphere_normals_smooth.push_back(p.smooth_normal[0]);
        sphere_normals_flat.push_back(p.flat_normal); sphere_points.push_back(p.vertices[1]); sphere_normals_smooth.push_back(p.smooth_normal[1]);
        sphere_normals_flat.push_back(p.flat_normal); sphere_points.push_back(p.vertices[2]); sphere_normals_smooth.push_back(p.smooth_normal[2]);
    }
}

//----------------------------------------------------------------------------
// generate axes: 6 vertices
void axes() {
    xaxis_points[0] = point4(100.0, 0.0, 0.0, 1.0);
    xaxis_points[1] = point4(0.0, 0.0, 0.0, 1.0);

    xaxis_normals[0] = vec3(0.0f, 0.0f, 0.0f);
    xaxis_normals[1] = vec3(0.0f, 0.0f, 0.0f);

    yaxis_points[0] = point4(0.0, 100.0, 0.0, 1.0);
    yaxis_points[1] = point4(0.0, 0.0, 0.0, 1.0);

    yaxis_normals[0] = vec3(0.0f, 0.0f, 0.0f);
    yaxis_normals[1] = vec3(0.0f, 0.0f, 0.0f);

    zaxis_points[0] = point4(0.0, 0.0, 100.0, 1.0);
    zaxis_points[1] = point4(0.0, 0.0, 0.0, 1.0);

    zaxis_normals[0] = vec3(0.0f, 0.0f, 0.0f);
    zaxis_normals[1] = vec3(0.0f, 0.0f, 0.0f);
}

//----------------------------------------------------------------------------
// generate shadow: n_polygons triangles: 3 * n_polygons vertices
void shadow() {
    for ( point4 p : sphere_points ) {

        shadow_normals.push_back(vec3(0.0f, 0.0f, 0.0f)); shadow_points.push_back(p);

    }
}

//----------------------------------------------------------------------------
// generate particles: ParticlesN vertices
void particles() {
    for ( int i = 0; i < ParticlesN; i++) {

        // assigning random x,y,z velocity
        particleVelocity[i].x = 2.0*((rand()%256)/256.0-0.5);
        particleVelocity[i].y = 1.2*2.0*((rand()%256)/256.0);
        particleVelocity[i].z = 2.0*((rand()%256)/256.0-0.5);
        particleVelocity[i].w = 0.0;

        // assigning random r,g,b color
        particleColor[i].x = (rand()%256)/256.0;
        particleColor[i].y = (rand()%256)/256.0;
        particleColor[i].z = (rand()%256)/256.0;
        particleColor[i].w = 1.0;
    }
}


//----------------------------------------------------------------------------
// OpenGL initialization
void init()
{

    glEnable(GL_DEPTH_TEST); // Enable Z-Buffer Testing
    glClearColor( 0.529, 0.807, 0.92, 0.0 ); // set background to sky blue color
    glLineWidth(2.0);
    glPointSize(3.0);

    // Generate Particle Parameters
    particles();

    glGenBuffers(1, &particle_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, particle_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(particleVelocity) + sizeof(particleColor),
         NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(particleVelocity), particleVelocity);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(particleVelocity), sizeof(particleColor), 
                    particleColor);

    // Generate Texture Images
    image_set_up();

    /*--- Create and Initialize a texture object for checkerboard ---*/
    glGenTextures(1, &texName);      // Generate texture obj name(s)

    glActiveTexture( GL_TEXTURE0 );  // Set the active texture unit to be 0 
    glBindTexture(GL_TEXTURE_2D, texName); // Bind the texture xto this texture unit

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    // ImageWidth, ImageHeight, Image defined in texmap.c
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ImageWidth, ImageHeight, 
                0, GL_RGBA, GL_UNSIGNED_BYTE, Image);

    /*--- Create and Initialize a texture object for contour lines ---*/
    glActiveTexture( GL_TEXTURE1 );  // Set the active texture unit to be 1 
    glBindTexture(GL_TEXTURE_1D, texName); // Bind the texture xto this texture unit

    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    // stripeImageWidth, stripeImage defined in texmap.c
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, stripeImageWidth, 
                0, GL_RGBA, GL_UNSIGNED_BYTE, stripeImage);

    sphere();

    // temp array for storing sphere points to be used in creating a buffer
    point4 sphere_points_arr[sphere_points.size()];
    copy(sphere_points.begin(), sphere_points.end(), sphere_points_arr);

    // temp array for storing flat sphere normals to be used in creating a buffer
    vec3 sphere_normals_flat_arr[sphere_normals_flat.size()];
    copy(sphere_normals_flat.begin(), sphere_normals_flat.end(), sphere_normals_flat_arr);

    // Create and initialize a vertex buffer object for sphere with flat shading, to be used in display()
    glGenBuffers(1, &sphere_buffer_flat);
    glBindBuffer(GL_ARRAY_BUFFER, sphere_buffer_flat);

    glBufferData(GL_ARRAY_BUFFER, 
                 sizeof(point4)*sphere_NumVertices + sizeof(vec3)*sphere_NumVertices,
         NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, 
                    sizeof(point4) * sphere_NumVertices, sphere_points_arr);
    glBufferSubData(GL_ARRAY_BUFFER, 
                    sizeof(point4) * sphere_NumVertices, 
                    sizeof(vec3) * sphere_NumVertices,
                    sphere_normals_flat_arr);

    // temp array for storing smooth sphere normals to be used in creating a buffer
    vec3 sphere_normals_smooth_arr[sphere_normals_smooth.size()];
    copy(sphere_normals_smooth.begin(), sphere_normals_smooth.end(), sphere_normals_smooth_arr);

    // Create and initialize a vertex buffer object for sphere with smooth, to be used in display()
    glGenBuffers(1, &sphere_buffer_smooth);
    glBindBuffer(GL_ARRAY_BUFFER, sphere_buffer_smooth);

    glBufferData(GL_ARRAY_BUFFER, 
                 sizeof(point4)*sphere_NumVertices + sizeof(vec3)*sphere_NumVertices,
         NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, 
                    sizeof(point4) * sphere_NumVertices, sphere_points_arr);
    glBufferSubData(GL_ARRAY_BUFFER, 
                    sizeof(point4) * sphere_NumVertices, 
                    sizeof(vec3) * sphere_NumVertices,
                    sphere_normals_smooth_arr);

    shadow();

    // temp array for storing shadow points to be used in creating a buffer
    point4 shadow_points_arr[shadow_points.size()];
    copy(shadow_points.begin(), shadow_points.end(), shadow_points_arr);

    // temp array for storing shadow normals to be used in creating a buffer (junk)
    vec3 shadow_normals_arr[shadow_normals.size()];
    copy(shadow_normals.begin(), shadow_normals.end(), shadow_normals_arr);

   // Create and initialize a vertex buffer object for shadow, to be used in display()
    glGenBuffers(1, &shadow_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, shadow_buffer);

    glBufferData(GL_ARRAY_BUFFER, 
                 sizeof(point4)*sphere_NumVertices + sizeof(vec3)*sphere_NumVertices,
         NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, 
                    sizeof(point4) * sphere_NumVertices, shadow_points_arr);
    glBufferSubData(GL_ARRAY_BUFFER, 
                    sizeof(point4) * sphere_NumVertices, 
                    sizeof(vec3) * sphere_NumVertices,
                    shadow_normals_arr);

    floor();     
 // Create and initialize a vertex buffer object for floor, to be used in display()
    glGenBuffers(1, &floor_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, floor_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(floor_points) + sizeof(floor_normals) + sizeof(floor_texCoord),
		 NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(floor_points), floor_points);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(floor_points), sizeof(floor_normals),
                    floor_normals);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(floor_points) + sizeof(floor_normals),
                    sizeof(floor_texCoord), floor_texCoord);

    axes();     
 // Create and initialize a vertex buffer object for x axis, to be used in display()

    glGenBuffers(1, &xaxis_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, xaxis_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(xaxis_points) + sizeof(xaxis_normals),
         NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(xaxis_points), xaxis_points);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(xaxis_points), sizeof(xaxis_normals),
                    xaxis_normals);

 // Create and initialize a vertex buffer object for y axis, to be used in display()
    glGenBuffers(1, &yaxis_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, yaxis_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(yaxis_points) + sizeof(yaxis_normals),
         NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(yaxis_points), yaxis_points);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(yaxis_points), sizeof(yaxis_normals),
                    yaxis_normals);

 // Create and initialize a vertex buffer object for z axis, to be used in display()
    glGenBuffers(1, &zaxis_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, zaxis_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(zaxis_points) + sizeof(zaxis_normals),
         NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(zaxis_points), zaxis_points);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(zaxis_points), sizeof(zaxis_normals),
                    zaxis_normals);

 // Load shaders and create a shader program (to be used in display())
 // Shaders for individual objects
    SphereProgram = InitShader( "./../vshader53.glsl", "./../fshader53.glsl" );
    FloorProgram = InitShader( "./../vshader53.glsl", "./../fshader53.glsl" );
    NoLightProgram = InitShader( "./../vshader53.glsl", "./../fshader53.glsl" );
    ParticleProgram = InitShader( "./../vParticle.glsl", "./../fParticle.glsl");
    
}
//----------------------------------------------------------------------------
// SetUp_Lighting_Uniform_Vars(mat4 mv, GLuint shader):
//   assign the lighting properties of the current object to
//   to the given shader program
//
void SetUp_Lighting_Uniform_Vars(mat4 mv, GLuint shader, color4 GlobalProduct, 
    color4 DirAmbientProduct, color4 DirDiffuseProduct, color4 DirSpecularProduct,
    color4 PosAmbientProduct, color4 PosDiffuseProduct, color4 PosSpecularProduct,
    color4 NoLightColor, float material_shininess,
    int LightingFlag, int FloorTextureFlag, int SphereTexFlag, int LaticeFlag)
{
    glUniform4fv( glGetUniformLocation(shader, "GlobalProduct"),
          1, GlobalProduct );
    glUniform4fv( glGetUniformLocation(shader, "DirAmbientProduct"),
          1, DirAmbientProduct );
    glUniform4fv( glGetUniformLocation(shader, "DirDiffuseProduct"),
          1, DirDiffuseProduct);
    glUniform4fv( glGetUniformLocation(shader, "DirSpecularProduct"),
          1, DirSpecularProduct );
    glUniform4fv( glGetUniformLocation(shader, "PosAmbientProduct"),
          1, PosAmbientProduct );
    glUniform4fv( glGetUniformLocation(shader, "PosDiffuseProduct"),
          1, PosDiffuseProduct);
    glUniform4fv( glGetUniformLocation(shader, "PosSpecularProduct"),
          1, PosSpecularProduct );
    glUniform4fv( glGetUniformLocation(shader, "NoLightColor"),
          1, NoLightColor );

    // Directional Light Direction is in EyeFrame
    glUniform4fv( glGetUniformLocation(shader, "DirLightDirection"),
          1, dir_light_direction );

    // Positional Light Direction is in World Frame
    vec4 pos_light_direction_eyeFrame = mv * spot_light_direction;
    glUniform4fv( glGetUniformLocation(shader, "PosLightDirection"),
          1, pos_light_direction_eyeFrame );

    // The Light Position is in World Frame
    vec4 pos_light_position_eyeFrame = mv * pos_light_position;
    glUniform4fv( glGetUniformLocation(shader, "LightPosition"),
          1, pos_light_position_eyeFrame);

    // Attenuation Values are only used for calulations of a positional light
    glUniform1f(glGetUniformLocation(shader, "ConstAtt"),
                const_att);
    glUniform1f(glGetUniformLocation(shader, "LinearAtt"),
                linear_att);
    glUniform1f(glGetUniformLocation(shader, "QuadAtt"),
                quad_att);

    // Parameters for a Spot Light
    glUniform1f(glGetUniformLocation(shader, "CutOffAngle"),
                cut_off_angle);
    glUniform1f(glGetUniformLocation(shader, "SpotLightExponent"),
                spot_light_exponent);

    // Shininess coefficient of the material
    glUniform1f(glGetUniformLocation(shader, "Shininess"),
                material_shininess );

    // Flag for how to handle lighting in the shader
    glUniform1i(glGetUniformLocation(shader, "LightingFlag"),
                LightingFlag );

    // Fog Parameters
    glUniform4fv(glGetUniformLocation(shader, "FogColor"),
                1, fog_color);
    glUniform1f(glGetUniformLocation(shader, "FogDensity"),
                fog_density);
    glUniform1f(glGetUniformLocation(shader, "FogStart"),
                fog_start);
    glUniform1f(glGetUniformLocation(shader, "FogEnd"),
                fog_end);
    glUniform1i(glGetUniformLocation(shader, "FogFlag"),
                fogFlag );

    // Texture ID Parameters

    // texUnit - 0:checkerboard texture, 1:linear texture
    glUniform1i(glGetUniformLocation(shader, "texture_2D"), 
                0);
    glUniform1i(glGetUniformLocation(shader, "texture_1D"), 
                1);
    glUniform1i( glGetUniformLocation(shader, "GroundTextureFlag"), 
                FloorTextureFlag);
    glUniform1i( glGetUniformLocation(shader, "SphereTextureFlag"), 
                SphereTexFlag);
    glUniform1i( glGetUniformLocation(shader, "SphereTextureFragFlag"), 
                SphereTexFlag);
    glUniform1i( glGetUniformLocation(shader, "SphereTextureDirection"), 
                sphereTextureDirection);
    glUniform1i( glGetUniformLocation(shader, "SphereTextureFrame"), 
                sphereTextureFrame);

    // Latice Parameters
    glUniform1i( glGetUniformLocation(shader, "VertexLatFlag"), 
                LaticeFlag);
    glUniform1i( glGetUniformLocation(shader, "FragLatFlag"), 
                LaticeFlag);
}
//----------------------------------------------------------------------------
// SetUp_Particle_Uniform_Vars(mat4 mv, mat4 p):
//   assign the particles properties of the current object to
//   to the given particle program
//
void SetUp_Particle_Uniform_Vars(mat4 mv, mat4 p) 
{
    glUniform1f(glGetUniformLocation(ParticleProgram, "t_0"),
                t_0);
    glUniform1f(glGetUniformLocation(ParticleProgram, "t_1"),
                glutGet(GLUT_ELAPSED_TIME));

    glUniformMatrix4fv(glGetUniformLocation(ParticleProgram, "ModelView"), 1, GL_TRUE, 
                mv);
    glUniformMatrix4fv(glGetUniformLocation(ParticleProgram, "Projection"), 1, GL_TRUE, 
                p);

}
//----------------------------------------------------------------------------
// drawObjShading(buffer, num_vertices, GLuint shader):
//   draw the object that is associated with the vertex buffer object "buffer"
//   and has "num_vertices" vertices.
//
void drawObjShading(GLuint buffer, int num_vertices, GLuint shader)
{
    //--- Activate the vertex buffer object to be drawn ---//
    glBindBuffer(GL_ARRAY_BUFFER, buffer);

    /*----- Set up vertex attribute arrays for each vertex attribute -----*/
    GLuint vPosition = glGetAttribLocation( shader, "vPosition" );
    glEnableVertexAttribArray( vPosition );
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0,
               BUFFER_OFFSET(0) );

    GLuint vNormal = glGetAttribLocation( shader, "vNormal" ); 
    glEnableVertexAttribArray( vNormal );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0,
               BUFFER_OFFSET(sizeof(point4) * num_vertices) ); 
    // the offset is the (total) size of the previous vertex attribute array(s)

    /* Draw a sequence of geometric objs (triangles) from the vertex buffer
       (using the attributes specified in each enabled vertex attribute array) */
    if (num_vertices > 2) {
        glDrawArrays(GL_TRIANGLES, 0, num_vertices);
    }
    else {
        glDrawArrays(GL_LINES, 0, 2);
    }

    /*--- Disable each vertex attribute array being enabled ---*/
    glDisableVertexAttribArray(vPosition);
    glDisableVertexAttribArray(vNormal);
}
//----------------------------------------------------------------------------
// drawObjTextured(buffer, num_vertices, GLuint shader):
//   draw the object that is associated with the vertex buffer object "buffer"
//   and has "num_vertices" vertices. includes a buffer for texture coordinates
//
void drawObjTextured(GLuint buffer, int num_vertices, GLuint shader)
{
    //--- Activate the vertex buffer object to be drawn ---//
    glBindBuffer(GL_ARRAY_BUFFER, buffer);

    /*----- Set up vertex attribute arrays for each vertex attribute -----*/
    GLuint vPosition = glGetAttribLocation( shader, "vPosition" );
    glEnableVertexAttribArray( vPosition );
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0,
               BUFFER_OFFSET(0) );

    GLuint vNormal = glGetAttribLocation( shader, "vNormal" ); 
    glEnableVertexAttribArray( vNormal );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0,
               BUFFER_OFFSET(sizeof(point4) * num_vertices) ); 
    // the offset is the (total) size of the previous vertex attribute array(s)

    GLuint vTexCoord = glGetAttribLocation( shader, "vTexCoord" ); 
    glEnableVertexAttribArray( vTexCoord );
    glVertexAttribPointer( vTexCoord, 2, GL_FLOAT, GL_FALSE, 0,
               BUFFER_OFFSET((sizeof(point4) * num_vertices) + (sizeof(vec3) * num_vertices)) ); 
    // the offset is the (total) size of the previous vertex attribute array(s)

    /* Draw a sequence of geometric objs (triangles) from the vertex buffer
       (using the attributes specified in each enabled vertex attribute array) */
    if (num_vertices > 2) {
        glDrawArrays(GL_TRIANGLES, 0, num_vertices);
    }
    else {
        glDrawArrays(GL_LINES, 0, 2);
    }

    /*--- Disable each vertex attribute array being enabled ---*/
    glDisableVertexAttribArray(vPosition);
    glDisableVertexAttribArray(vNormal);
    glDisableVertexAttribArray(vTexCoord);
}
//----------------------------------------------------------------------------
// drawParticles():
//   draw the system's particles
//
void drawParticles()
{
    //--- Activate the vertex buffer object to be drawn ---//
    glBindBuffer(GL_ARRAY_BUFFER, particle_buffer);

    /*----- Set up vertex attribute arrays for each vertex attribute -----*/

    GLuint vVelocity= glGetAttribLocation( ParticleProgram, "vVelocity" ); 
    glEnableVertexAttribArray( vVelocity );
    glVertexAttribPointer( vVelocity, 4, GL_FLOAT, GL_FALSE, 0,
               BUFFER_OFFSET(0) ); 

    GLuint vColor = glGetAttribLocation( ParticleProgram, "vColor" ); 
    glEnableVertexAttribArray( vColor );
    glVertexAttribPointer( vColor, 4, GL_FLOAT, GL_FALSE, 0,
               BUFFER_OFFSET(sizeof(particleColor)) ); 
    // the offset is the (total) size of the previous vertex attribute array(s)

    glDrawArrays(GL_POINTS, 0, ParticlesN);

    /*--- Disable each vertex attribute array being enabled ---*/
    glDisableVertexAttribArray(vColor);
    glDisableVertexAttribArray(vVelocity);
}
//----------------------------------------------------------------------------
void display( void )
{
  GLuint  model_view;  // model-view matrix uniform shader variable location
  GLuint  projection;  // projection matrix uniform shader variable location
  mat4    p;
  mat3    normal_matrix;
  GLuint  sphere_buffer = sphere_buffer_flat; // curr sphere buffer based on shading mode

    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    /*---  Set up Projection matrix for the shaders ---*/
    p = Perspective(fovy, aspect, zNear, zFar);

    /*---  Set up variables for Model-View matrix to the shader ---*/
    // eye is a global variable of vec4 set to init_eye and updated by keyboard()
    vec4 at = VPN + VRP; // VPN = at - eye (eye = VRP) should be (0, 0, 0, 1)
    vec4 up = VUP; // up = VUP

    mat4 mv = LookAt(eye, at, up);

    if (solidityFlag) {

        // If Sphere is Solid, attempt to light it

        // If Sphere is Solid, shadingFlag will always be 1 or 2
        if (shadingFlag == 2) {
            // Smooth shading enabled
            sphere_buffer = sphere_buffer_smooth;
        } // Else: flat shading enabled, sphere_buffer = sphere_buffer_flat by default

        // Switch to the shader program for the sphere
        glUseProgram(SphereProgram); // Use the shader program

        model_view = glGetUniformLocation(SphereProgram, "ModelView" );
        projection = glGetUniformLocation(SphereProgram, "Projection" );

        /*---  Pass on Projection matrix to the shader ---*/
        glUniformMatrix4fv(projection, 1, GL_TRUE, p); // GL_TRUE: matrix is row-major

        // Set up the correct ambient, global, diffuse, specular, and shiniess properties for the calculations
        // If light is not enabled, lightingFlag == 0 and the program does not light it
        SetUp_Lighting_Uniform_Vars(mv, SphereProgram, sphere_global_res, 
        dir_sphere_ambient_res, dir_sphere_diffuse_res, dir_sphere_specular_res,
        pos_sphere_ambient_res, pos_sphere_diffuse_res, pos_sphere_specular_res, 
        sphere_no_light_color, sphere_material_shininess, lightingFlag, 0, sphereTextureFlag, latFlag);

        /*----- Set up transformed Model-View matrix for the sphere -----*/
        mv = mv * Translate(trans_vec) * Rotate(angle, curr_rotation_axis.x, curr_rotation_axis.y, curr_rotation_axis.z) * M;

        glUniformMatrix4fv(model_view, 1, GL_TRUE, mv); // GL_TRUE: matrix is row-major

        normal_matrix = NormalMatrix(model_view, 1);

        glUniformMatrix3fv(glGetUniformLocation(SphereProgram, "Normal_Matrix"), 
                   1, GL_TRUE, normal_matrix );

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Filled sphere

        drawObjShading(sphere_buffer, sphere_NumVertices, SphereProgram); // draw the sphere

    }
    else {

        // If Sphere is not Solid, do not attempt to light it

        // If Sphere is not Solid, either shading buffer can be used because the normals are ignored

        // If Sphere is not Solid, no texture will be mapped to it

        // If Sphere is not Solid, no latice effect will be applied to it

        // Switch to the shader program for no light
        glUseProgram(NoLightProgram); // Use the shader program

        model_view = glGetUniformLocation(NoLightProgram, "ModelView" );
        projection = glGetUniformLocation(NoLightProgram, "Projection" );

        /*---  Pass on Projection matrix to the shader ---*/
        glUniformMatrix4fv(projection, 1, GL_TRUE, p); // GL_TRUE: matrix is row-major

        // Set up the correct ambient, global, diffuse, specular, and shiniess properties for the calculations
        // If light is not enabled, lightingFlag == 0 and the program does not light it
        SetUp_Lighting_Uniform_Vars(mv, NoLightProgram, NULL, NULL, NULL, NULL, NULL, NULL, NULL, sphere_no_light_color, 0.0f, 0, 0, 0, 0);

        /*----- Set up transformed Model-View matrix for the sphere -----*/
        mv = mv * Translate(trans_vec) * Rotate(angle, curr_rotation_axis.x, curr_rotation_axis.y, curr_rotation_axis.z) * M;

        glUniformMatrix4fv(model_view, 1, GL_TRUE, mv); // GL_TRUE: matrix is row-major

        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Wireframe sphere

        drawObjShading(sphere_buffer, sphere_NumVertices, NoLightProgram); // draw the sphere

    }

    

/*----- Determining whether to draw the Shadow -----*/
    if (shadowFlag && (eye.y >= 0)) {
        // Drawing Shadow as a Decal

        // Use the FloorProgram Shader Object
        glUseProgram(FloorProgram);

        model_view = glGetUniformLocation(FloorProgram, "ModelView" );
        projection = glGetUniformLocation(FloorProgram, "Projection" );

        /*---  Pass on Projection matrix to the shader ---*/
        glUniformMatrix4fv(projection, 1, GL_TRUE, p); // GL_TRUE: matrix is row-major

        glDepthMask(GL_FALSE); // Disable writing to the Z-Buffer

        /*----- Set up the Model-View matrix for the floor -----*/
        mv = LookAt(eye, at, up);

        // Set up the correct ambient, global, diffuse, specular, and shiniess properties for the calculations
        SetUp_Lighting_Uniform_Vars(mv, FloorProgram, floor_global_res, 
            dir_floor_ambient_res, dir_floor_diffuse_res, dir_floor_specular_res,
            pos_floor_ambient_res, pos_floor_diffuse_res, pos_floor_specular_res, 
            floor_no_light_color, floor_material_shininess, lightingFlag, groundTextureFlag, 0, 0);

        // Draw Floor to Only Frame Buffer
        glUniformMatrix4fv(model_view, 1, GL_TRUE, mv); // GL_TRUE: matrix is row-major
        normal_matrix = NormalMatrix(model_view, 1);
        glUniformMatrix3fv(glGetUniformLocation(FloorProgram, "Normal_Matrix"), 
               1, GL_TRUE, normal_matrix );
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        drawObjTextured(floor_buffer, floor_NumVertices, FloorProgram);  // draw the floor to frame buffer

        // Switch to the NoLightProgram Shader Object
        glUseProgram(NoLightProgram);

        model_view = glGetUniformLocation(NoLightProgram, "ModelView" );
        projection = glGetUniformLocation(NoLightProgram, "Projection" );

        /*---  Pass on Projection matrix to the shader ---*/
        glUniformMatrix4fv(projection, 1, GL_TRUE, p); // GL_TRUE: matrix is row-major

        if (shadowBlendingFlag) {
            // if we are blending the shadow, enable blending
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }
        else {
            // if we aren't blending the shadow, enable the z-buffer
            glDepthMask(GL_TRUE); // Enable writing to the Z-Buffer
        }

        /*----- Set up initial Model-View matrix for the shadow -----*/
        mv = LookAt(eye, at, up); 

        // All Lighting Properties are NULL because there are no light calculations for this shader object
        // LightingFlag is 0 such that the vertext shader program will handle it as if there is no lighting
        SetUp_Lighting_Uniform_Vars(mv, NoLightProgram, NULL, NULL, NULL, NULL, NULL, NULL, NULL, shadow_color, 0.0f, 0, 0, 0, latFlag);

        /*----- Set up the Model-View matrix for the shadow -----*/
        mv = mv * N * Translate(trans_vec) * Rotate(angle, curr_rotation_axis.x, curr_rotation_axis.y, curr_rotation_axis.z) * M;

        // Draw Shadow to both Frame and Z-Buffer
        glUniformMatrix4fv(model_view, 1, GL_TRUE, mv); // GL_TRUE: matrix is row-major
        if (solidityFlag) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Filled sphere shadow
        }
        else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Wireframe sphere shadow
        }
        drawObjShading(shadow_buffer, sphere_NumVertices, NoLightProgram); // draw the shadow to both buffers, same number of vertices as sphere

        // Switch to the FloorProgram Shader Object
        glUseProgram(FloorProgram);

        model_view = glGetUniformLocation(FloorProgram, "ModelView" );
        projection = glGetUniformLocation(FloorProgram, "Projection" );

        /*---  Set up and pass on Projection matrix to the shader ---*/
        glUniformMatrix4fv(projection, 1, GL_TRUE, p); // GL_TRUE: matrix is row-major

        if (shadowBlendingFlag) {
            // if we are blending the shadow, diable blending and enable the z-buffer
            glDisable(GL_BLEND);
            glDepthMask(GL_TRUE); // Enable writing to the Z-Buffer
        }

        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); // Disable writing to the Frame Buffer

        /*----- Set up the Model-View matrix for the floor -----*/
        mv = LookAt(eye, at, up);

        // Set up the correct ambient, global, diffuse, specular, and shiniess properties for the calculations
        SetUp_Lighting_Uniform_Vars(mv, FloorProgram, floor_global_res, 
            dir_floor_ambient_res, dir_floor_diffuse_res, dir_floor_specular_res,
            pos_floor_ambient_res, pos_floor_diffuse_res, pos_floor_specular_res, 
            floor_no_light_color, floor_material_shininess, lightingFlag, groundTextureFlag, 0, 0);

        // Draw Floor to Only Z-Buffer
        glUniformMatrix4fv(model_view, 1, GL_TRUE, mv); // GL_TRUE: matrix is row-major
        normal_matrix = NormalMatrix(model_view, 1);
        glUniformMatrix3fv(glGetUniformLocation(FloorProgram, "Normal_Matrix"), 
               1, GL_TRUE, normal_matrix );
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        drawObjTextured(floor_buffer, floor_NumVertices, FloorProgram);  // draw the floor to z-buffer

        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE); // Enable writing to the frame buffer.

    }

    else {
        // Shadow is not Drawn

        // Use the FloorProgram Shader Object
        glUseProgram(FloorProgram);

        model_view = glGetUniformLocation(FloorProgram, "ModelView" );
        projection = glGetUniformLocation(FloorProgram, "Projection" );

        /*---  Pass on Projection matrix to the shader ---*/
        glUniformMatrix4fv(projection, 1, GL_TRUE, p); // GL_TRUE: matrix is row-major

        /*----- Set up the Model-View matrix for the floor -----*/
        mv = LookAt(eye, at, up);

        // Set up the correct ambient, global, diffuse, specular, and shiniess properties for the calculations
        SetUp_Lighting_Uniform_Vars(mv, FloorProgram, floor_global_res, 
            dir_floor_ambient_res, dir_floor_diffuse_res, dir_floor_specular_res,
            pos_floor_ambient_res, pos_floor_diffuse_res, pos_floor_specular_res, 
            floor_no_light_color, floor_material_shininess, lightingFlag, groundTextureFlag, 0, 0);

        glUniformMatrix4fv(model_view, 1, GL_TRUE, mv); // GL_TRUE: matrix is row-major
        normal_matrix = NormalMatrix(model_view, 1);
        glUniformMatrix3fv(glGetUniformLocation(FloorProgram, "Normal_Matrix"), 
               1, GL_TRUE, normal_matrix );
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        drawObjTextured(floor_buffer, floor_NumVertices, FloorProgram);  // draw the floor
    }

    // Switch to the NoLightProgram Shader Object
    glUseProgram(NoLightProgram);

    model_view = glGetUniformLocation(NoLightProgram, "ModelView" );
    projection = glGetUniformLocation(NoLightProgram, "Projection" );

    /*---  Pass on Projection matrix to the shader ---*/
    glUniformMatrix4fv(projection, 1, GL_TRUE, p); // GL_TRUE: matrix is row-major

    /*----- Set up the Model-View matrix for the axes -----*/
    mv = LookAt(eye, at, up);

    // All Lighting Properties are NULL because there are no light calculations for this shader object
    // LightingFlag is 0 such that the vertext shader program will handle it as if there is no lighting
    SetUp_Lighting_Uniform_Vars(mv, NoLightProgram, NULL, NULL, NULL, NULL, NULL, NULL, NULL, xaxis_color, 0.0f, 0, 0, 0, 0);

    glUniformMatrix4fv(model_view, 1, GL_TRUE, mv); // GL_TRUE: matrix is row-major
    drawObjShading(xaxis_buffer, 2, NoLightProgram);  // draw the x axis

    mv = LookAt(eye, at, up);

    // All Lighting Properties are NULL because there are no light calculations for this shader object
    // LightingFlag is 0 such that the vertext shader program will handle it as if there is no lighting
    SetUp_Lighting_Uniform_Vars(mv, NoLightProgram, NULL, NULL, NULL, NULL, NULL, NULL, NULL, yaxis_color, 0.0f, 0, 0, 0, 0);

    glUniformMatrix4fv(model_view, 1, GL_TRUE, mv); // GL_TRUE: matrix is row-major
    drawObjShading(yaxis_buffer, 2, NoLightProgram);  // draw the y axis

    mv = LookAt(eye, at, up);

    // All Lighting Properties are NULL because there are no light calculations for this shader object
    // LightingFlag is 0 such that the vertext shader program will handle it as if there is no lighting
    SetUp_Lighting_Uniform_Vars(mv, NoLightProgram, NULL, NULL, NULL, NULL, NULL, NULL, NULL, zaxis_color, 0.0f, 0, 0, 0, 0);

    glUniformMatrix4fv(model_view, 1, GL_TRUE, mv); // GL_TRUE: matrix is row-major
    drawObjShading(zaxis_buffer, 2, NoLightProgram);  // draw the z axis

    /*-- Particle Display --*/

    if (particleFlag) {

        // Switch to the NoLightProgram Shader Object
        glUseProgram(ParticleProgram);

        /*----- Set up the Model-View matrix for the particles -----*/
        mv = LookAt(eye, at, up);

        SetUp_Particle_Uniform_Vars(mv, p);

        drawParticles();
    }

    glutSwapBuffers();

    glutPostRedisplay();
    
}
//---------------------------------------------------------------------------
void idle (void)
{
     // angle += 0.03f;
    angle += 1.0f;

    float d;

    /*---  Set up Translation Vector ---*/
    if (trans_mode == 0) {

        d = angle * (2.0 * M_PI * r) / 360.0;
        trans_vec = A + d * normalize(AB);

        /*
        cout << "In AB Mode" << endl;
        cout << "Translation Vector: " << trans_vec << endl;
        */

        if (trans_vec.x < B.x || trans_vec.z < B.z) {
            // If translation vector is past point B, change to BC Mode
            // Reset Angle and Change Rotation Axis Vector

            // Update Accumulation Matrix
            M = Rotate(angle, curr_rotation_axis.x, curr_rotation_axis.y, curr_rotation_axis.z) * M ;

            curr_rotation_axis = BC_rotation_axis;
            trans_mode = 1;
            angle = 0;

        }
    }
    else if (trans_mode == 1) {

        d = angle * (2.0 * M_PI * r) / 360.0;
        trans_vec = B + d * normalize(BC);

        /*
        cout << "In BC Mode" << endl;
        cout << "Translation Vector: " << trans_vec << endl;
        */

        if (trans_vec.x > C.x || trans_vec.z < C.z) {
            // If translation vector is past point C, change to CA Mode
            // Reset Angle

            // Update Accumulation Matrix
            M = Rotate(angle, curr_rotation_axis.x, curr_rotation_axis.y, curr_rotation_axis.z) * M ;

            curr_rotation_axis = CA_rotation_axis;
            trans_mode = 2;
            angle = 0;


        }
    }
    else {

        d = angle * (2.0 * M_PI * r) / 360.0;
        trans_vec = C + d * normalize(CA);

        /*
        cout << "In CA Mode" << endl;
        cout << "Translation Vector: " << trans_vec << endl;
        */

        if (trans_vec.x > A.x || trans_vec.z > A.z) {
            // If translation vector is past point A, change to AB Mode
            // Reset Angle

            // Update Accumulation Matrix
            M = Rotate(angle, curr_rotation_axis.x, curr_rotation_axis.y, curr_rotation_axis.z) * M ;

            curr_rotation_axis = AB_rotation_axis;
            trans_mode = 0;
            angle = 0;

        }
    }

    glutPostRedisplay();
}
//----------------------------------------------------------------------------
void keyboard(unsigned char key, int x, int y)
{
    switch(key) {

        case 'X': eye[0] += 1.0; break;
	case 'x': eye[0] -= 1.0; break;
        case 'Y': eye[1] += 1.0; break;
	case 'y': eye[1] -= 1.0; break;
        case 'Z': eye[2] += 1.0; break;
	case 'z': eye[2] -= 1.0; break;

        case 'b': case 'B': // Begins Animation
            if (not beginFlag) {
                glutIdleFunc(idle);
                beginFlag = 1;
                animationFlag = 1;
            }
            break;

        case 'v': case 'V': // vertical sphere texture map
            if (sphereTextureFlag > 0) {
                sphereTextureDirection = 0;
            }
            break;

        case 's': case 'S': // slanted sphere texture map
            if (sphereTextureFlag > 0) {
                sphereTextureDirection = 1;
            }
            break;

        case 'o': case 'O': // sphere texture map in object space
            if (sphereTextureFlag > 0) {
                sphereTextureFrame = 0;
            }
            break;

        case 'e': case 'E': // sphere texture map in eye frame
            if (sphereTextureFlag > 0) {
                sphereTextureFrame = 1;
            }
            break;

        case 'u': case 'U': // lattice effect upright
            if (latFlag > 0) {
                latFlag = 1;
            }
            break;

        case 't': case 'T': // lattice effect tilted
            if (latFlag > 0) {
                latFlag = 2;
            }
            break;

        case 'l': case 'L': // toggle lattice effect on/off
            if (latFlag > 0) {
                latFlag = 0;
            }
            else {
                latFlag = 1;
                solidityFlag = 1;
            }
            break;

    }

    glutPostRedisplay();
}
//----------------------------------------------------------------------------
void mouse(int button, int state, int x, int y) {
    if(button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
        // Toggle between animation and non-animation
        // Only if the animation began by hitting b
        if (beginFlag) {
                animationFlag = 1 -  animationFlag;
                if (animationFlag == 1) glutIdleFunc(idle);
                else                    glutIdleFunc(NULL);
            }
    }

    glutPostRedisplay();
}
//----------------------------------------------------------------------------
void menu(int id) {
    switch(id) {
        case 1:
            // reset to initial viewer/eye position
            eye = init_eye;

        break;

        case 2:
            // quits the program
            exit(EXIT_SUCCESS);
        break;

        case 3:
            // draws a wireframe sphere
            // turns off shading
            // can also be toggled by toggling either shading option
            solidityFlag = 0;
            shadingFlag = 0;
            sphereTextureFlag = 0;
        break;
            
    }

    glutPostRedisplay();
}
//----------------------------------------------------------------------------
void shadow_menu(int id) {
    switch(id) {
        case 1:
            // sphere does not produce a shadow
            shadowFlag = 0;
        break;

        case 2:
            // sphere does produce a shadow
            shadowFlag = 1;
            sphereTextureFlag = 0;
        break;
            
    }

    glutPostRedisplay();
}
//----------------------------------------------------------------------------
void lighting_menu(int id) {
    switch(id) {
        case 1:
            // lighting is disabled
            lightingFlag = 0;
        break;

        case 2:
            // lighting is enabled
            lightingFlag = 1;
        break;
            
    }

    glutPostRedisplay();
}
//----------------------------------------------------------------------------
void shading_menu(int id) {
    switch(id) {
        case 1:
            // flat shading
            shadingFlag = 1;
            solidityFlag = 1;
        break;

        case 2:
            // smooth shading
            shadingFlag = 2;
            solidityFlag = 1;
        break;
            
    }

    glutPostRedisplay();
}
//----------------------------------------------------------------------------
void light_source_menu(int id) {
    switch(id) {
        // does nothing unless lightening is enabled
        case 1:
            // spot light
            if (lightingFlag != 0) {
                lightingFlag = 1;
            }
        break;

        case 2:
            // point source
            if (lightingFlag != 0) {
                lightingFlag = 2;
            }
        break;
            
    }

    glutPostRedisplay();
}
//----------------------------------------------------------------------------
void fog_menu(int id) {
    switch(id) {
        case 1:
            // no fog
            fogFlag = 0;
        break;

        case 2:
            // linear fog
            fogFlag = 1;
        break;

        case 3:
            // exponential fog
            fogFlag = 2;
        break;

        case 4:
            // gaussian fog
            fogFlag = 3;
        break;
            
    }

    glutPostRedisplay();
}
//----------------------------------------------------------------------------
void blending_shadow_menu(int id) {
    switch(id) {
        case 1:
            // shadow blending off
            shadowBlendingFlag = 0;
        break;

        case 2:
            // shadow blending on
            shadowBlendingFlag = 1;
        break;
            
    }

    glutPostRedisplay();
}
//----------------------------------------------------------------------------
void ground_texture_menu(int id) {
    switch(id) {
        case 1:
            // ground texture off
            groundTextureFlag = 0;
        break;

        case 2:
            // ground texture on
            groundTextureFlag = 1;
        break;
            
    }

    glutPostRedisplay();
}
//----------------------------------------------------------------------------
void sphere_texture_menu(int id) {
    switch(id) {
        case 1:
            // sphere texture off
            sphereTextureFlag = 0;
        break;

        case 2:
            // 1D sphere texture
            // turn off wireframe and shadow
            shadowFlag = 0;
            solidityFlag = 1;
            sphereTextureFlag = 1;
        break;

        case 3:
            // 2D sphere texture
            // turn off wireframe and shadow
            shadowFlag = 0;
            solidityFlag = 1;
            sphereTextureFlag = 2;
        break;
            
    }

    glutPostRedisplay();
}
//----------------------------------------------------------------------------
void particle_menu(int id) {
    switch(id) {
        case 1:
            // particles off
            particleFlag = 0;
        break;

        case 2:
            // particles on
            particleFlag = 1;
            t_0 =  glutGet(GLUT_ELAPSED_TIME);
        break;
            
    }

    glutPostRedisplay();
}
//----------------------------------------------------------------------------
void reshape(int width, int height)
{
    glViewport(0, 0, width, height);
    aspect = (GLfloat) width  / (GLfloat) height;
    glutPostRedisplay();
}
//----------------------------------------------------------------------------
int main( int argc, char **argv )
{
    glutInit(&argc, argv);
#ifdef __APPLE__ // Enable core profile of OpenGL 3.2 on macOS.
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_3_2_CORE_PROFILE);
#else
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
#endif
    glutInitWindowSize(512, 512);
    glutCreateWindow("Rotating Sphere With Shadow, Lighting, and Shading");

#ifdef __APPLE__ // on macOS
    // Core profile requires to create a Vertex Array Object (VAO).
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
#else           // on Linux or Windows, we still need glew
    /* Call glewInit() and error checking */
    int err = glewInit();
    if (GLEW_OK != err)
    { 
        printf("Error: glewInit failed: %s\n", (char*) glewGetErrorString(err)); 
        exit(1);
    }
#endif

    // Get info of GPU and supported OpenGL version
    printf("Renderer: %s\n", glGetString(GL_RENDERER));
    printf("OpenGL version supported %s\n", glGetString(GL_VERSION));

    fileIn();

    /* //Print File Information
    int i = 0;
    for (polygon p : polygons) {

        cout << "Polygon " << i << ":" << endl;

        for (int j = 1; j < 3; j++) {
            cout << "x = " << p.vertices[j].x << endl;
            cout << "y = " << p.vertices[j].y << endl;
            cout << "z = " << p.vertices[j].z << endl;
        }
        
        i++;
    } */

    int menuID, shadow_menuID, lighting_menuID, shading_menuID, 
    light_source_menuID, fog_menuID, blending_shadow_menuID, 
    ground_texture_menuID, sphere_texture_menuID, particle_menuID;
    sphere_NumVertices = n_polyons * 3;

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutIdleFunc(NULL);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);

    shadow_menuID = glutCreateMenu(shadow_menu);

    #ifndef __APPLE__ 
        // freeglut is not working on macos
        // this runs on windows such that font is updated
        glutSetMenuFont(shadow_menuID, GLUT_BITMAP_HELVETICA_18);
    #endif

    glutAddMenuEntry(" No ", 1);
    glutAddMenuEntry(" Yes ", 2);

    lighting_menuID = glutCreateMenu(lighting_menu);

    #ifndef __APPLE__ 
        // freeglut is not working on macos
        // this runs on windows such that font is updated
        glutSetMenuFont(lighting_menuID, GLUT_BITMAP_HELVETICA_18);
    #endif

    glutAddMenuEntry(" No ", 1);
    glutAddMenuEntry(" Yes ", 2);

    shading_menuID = glutCreateMenu(shading_menu);

    #ifndef __APPLE__ 
        // freeglut is not working on macos
        // this runs on windows such that font is updated
        glutSetMenuFont(shading_menuID, GLUT_BITMAP_HELVETICA_18);
    #endif

    glutAddMenuEntry(" Flat Shading ", 1);
    glutAddMenuEntry(" Smooth Shading ", 2);

    light_source_menuID = glutCreateMenu(light_source_menu);

    #ifndef __APPLE__ 
        // freeglut is not working on macos
        // this runs on windows such that font is updated
        glutSetMenuFont(light_source_menuID, GLUT_BITMAP_HELVETICA_18);
    #endif

    glutAddMenuEntry(" Spot Light ", 1);
    glutAddMenuEntry(" Point Source ", 2);

    fog_menuID = glutCreateMenu(fog_menu);

    #ifndef __APPLE__ 
        // freeglut is not working on macos
        // this runs on windows such that font is updated
        glutSetMenuFont(fog_menuID, GLUT_BITMAP_HELVETICA_18);
    #endif

    glutAddMenuEntry(" No Fog ", 1);
    glutAddMenuEntry(" Linear Fog ", 2);
    glutAddMenuEntry(" Exponential Fog ", 3);
    glutAddMenuEntry(" Exponential Square Fog ", 4);

    blending_shadow_menuID = glutCreateMenu(blending_shadow_menu);

    #ifndef __APPLE__ 
        // freeglut is not working on macos
        // this runs on windows such that font is updated
        glutSetMenuFont(blending_shadow_menuID, GLUT_BITMAP_HELVETICA_18);
    #endif

    glutAddMenuEntry(" No ", 1);
    glutAddMenuEntry(" Yes ", 2);

    ground_texture_menuID = glutCreateMenu(ground_texture_menu);

    #ifndef __APPLE__ 
        // freeglut is not working on macos
        // this runs on windows such that font is updated
        glutSetMenuFont(ground_texture_menuID, GLUT_BITMAP_HELVETICA_18);
    #endif

    glutAddMenuEntry(" No ", 1);
    glutAddMenuEntry(" Yes ", 2);

    sphere_texture_menuID = glutCreateMenu(sphere_texture_menu);

    #ifndef __APPLE__ 
        // freeglut is not working on macos
        // this runs on windows such that font is updated
        glutSetMenuFont(sphere_texture_menuID, GLUT_BITMAP_HELVETICA_18);
    #endif

    glutAddMenuEntry(" No ", 1);
    glutAddMenuEntry(" Yes - Contour Lines ", 2);
    glutAddMenuEntry(" Yes - Checkerboard ", 3);

    particle_menuID = glutCreateMenu(particle_menu);

    #ifndef __APPLE__ 
        // freeglut is not working on macos
        // this runs on windows such that font is updated
        glutSetMenuFont(particle_menuID, GLUT_BITMAP_HELVETICA_18);
    #endif

    glutAddMenuEntry(" No ", 1);
    glutAddMenuEntry(" Yes ", 2);

    menuID = glutCreateMenu(menu);

    #ifndef __APPLE__ 
        // freeglut is not working on macos
        // this runs on windows such that font is updated
        glutSetMenuFont(menuID, GLUT_BITMAP_HELVETICA_18);
    #endif

    glutAddMenuEntry(" Default View Point ", 1);
    glutAddMenuEntry(" Quit ", 2);
    glutAttachMenu(GLUT_LEFT_BUTTON);
    glutAddSubMenu(" Shadow ", shadow_menuID);
    glutAddSubMenu(" Enable Lighting", lighting_menuID);
    glutAddMenuEntry(" Wireframe Sphere ", 3);
    glutAddSubMenu(" Shading ", shading_menuID);
    glutAddSubMenu(" Light Source ", light_source_menuID);
    glutAddSubMenu(" Fog Options ", fog_menuID);
    glutAddSubMenu(" Blending Shadow ", blending_shadow_menuID);
    glutAddSubMenu(" Texture Mapped Ground ", ground_texture_menuID);
    glutAddSubMenu(" Texture Mapped Sphere ", sphere_texture_menuID);
    glutAddSubMenu(" Firework ", particle_menuID);


    init();
    glutMainLoop();
    return 0;
}

void fileIn() {
    // Asks user for the filename and reads the file
    // Stores information in a global vector of polygon structs

    string filename;
    ifstream file;

    cout << "Input Filename: ";
    cin >> filename;

    // Print Identity Matrix
    // cout << "M: " << M << endl;

    file.open(filename);

    if (file.is_open()) {

        cout << "File Found" << endl;

        file >> n_polyons;

        // cout << "Number of Polygons: " << n_polyons << endl;

        for (int i = 0; i < n_polyons; i++) {

            polygon temp_poly;

            file >> temp_poly.n;

            // cout << "Number of vertices in Polygon " << i << ": " << temp_poly.n << endl;

            for (int j = 0; j < temp_poly.n; j++) {

                point4 temp_point;

                file >> temp_point.x;
                file >> temp_point.y;
                file >> temp_point.z;
                temp_point.w = 1.0;

                temp_poly.vertices[j] = temp_point;

                vec4 temp_smooth = normalize(temp_point - point4(0.0, 0.0, 0.0, 1.0));

                temp_poly.smooth_normal[j].x = temp_smooth.x;
                temp_poly.smooth_normal[j].y = temp_smooth.y;
                temp_poly.smooth_normal[j].z = temp_smooth.z;

            }

            vec4 u = temp_poly.vertices[2] - temp_poly.vertices[0];
            vec4 v = temp_poly.vertices[1] - temp_poly.vertices[0];

            temp_poly.flat_normal = normalize(cross(u, v));

            polygons.push_back(temp_poly);
        }
    }
    else {
        cout << "File Not Found" << endl;
    }
}



















