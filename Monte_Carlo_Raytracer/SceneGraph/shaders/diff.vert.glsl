#version 130
// ^ Change this to version 130 if you have compatibility issues

uniform mat4 u_Model;
uniform mat4 u_ModelInvTr;
uniform mat4 u_ViewProj;
uniform vec4 u_LightPos;
uniform vec4 u_LightColor;
uniform vec4 u_CameraPos;
//uniform vec4 u_Texture;

in vec3 vs_Position;
in vec3 vs_Normal;
in vec3 vs_Color;
//in vec3 vs_Texture;

out vec4 fs_Normal;
out vec4 fs_LightVector;
out vec4 fs_Color;
out vec4 fs_ViewVector;

//vec4 lightPos = vec4(5, 5, 3, 1);

void main()
{
    fs_Color = vec4(vs_Color, 1);
    fs_Color *= u_LightColor;
    fs_Normal = u_ModelInvTr * vec4(vs_Normal, 0);

    vec4 modelposition = u_Model * vec4(vs_Position, 1);

    // Set up our vector for the light
    fs_LightVector = u_LightPos - modelposition;

    // Set up our vector for the view 
    fs_ViewVector = u_CameraPos - modelposition;

    //built-in things to pass down the pipeline
    gl_Position = u_ViewProj * modelposition;
}
