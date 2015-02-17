#version 130
// ^ Change this to version 130 if you have compatibility issues

//these are the interpolated values out of the rasterizer, so you can't know
//their specific values without knowing the vertices that contributed to them
in vec4 fs_Normal;
in vec4 fs_LightVector;
in vec4 fs_Color;
in vec4 fs_ViewVector;

out vec4 out_Color;

uniform sampler2D myTexture;

varying vec2 vTexCoord;


const int specular_power = 100;
void main()
{
	vec4 Normal_norm = normalize(fs_Normal);
	vec4 LightVec_norm = normalize(fs_LightVector);
	vec4 ViewVec_norm = normalize(fs_ViewVector);

    // Calculate the diffuse term
    float diffuseTerm = dot(Normal_norm, LightVec_norm); 

    // Avoid negative lighting values
    diffuseTerm = clamp(diffuseTerm, 0, 1);

    // Calculate the specular term: (R dot V) ^ n or (H dot N) ^ n
	//Rr = Ri - 2 N (Ri . N)
	//vec4  R = normalize(fs_LightVector) - 2 * normalize(fs_Normal) * dot(normalize(fs_LightVector), normalize(fs_Normal));
	//has bug
	//vec4 R = LightVec_norm - 2 * Normal_norm * dot(LightVec_norm, Normal_norm);
	//float specularTerm = dot(ViewVec_norm, normalize(R)); 
	
	vec4 H = normalize(fs_LightVector + fs_ViewVector); 
	float specularTerm = dot(normalize(fs_Normal), H); 
	specularTerm = pow(specularTerm,  specular_power);
    specularTerm = clamp(specularTerm, 0, 1);

    float ambientTerm = 0.2;
	float light_refleted = ambientTerm + diffuseTerm + specularTerm; 

    // Compute final shaded color
    out_Color = vec4(fs_Color.rgb * light_refleted, fs_Color.a);

	//texture
	//gl_FragColor = texture2D(myTexture, vTexCoord);
}
