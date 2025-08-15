
#version 330 core
out vec4 FragColor;
in vec2 TexCoord;

/*****************Screen输入*****************************************************************/
uniform sampler2D screenTex;

/*****************SSAO输入******************************************************************/
uniform sampler2D ssaoTex;

/*****************视口大小******************************************************************/
uniform int width = 1600;
uniform int height = 900;

/*****************toggle设置******************************************************************/
uniform int SSAO;
uniform int HDR;
uniform int Vignetting;
uniform int GammaCorrection;

void main() {
    FragColor = texture(screenTex,TexCoord);
    if(SSAO == 1) {
        vec3 AO = texture(ssaoTex,TexCoord).rgb;
        FragColor *= vec4(AO,1.0f);
    }
    //HDR
    if(HDR==1) {
        const float exposure = 1.1;
        FragColor *= exposure;
    }

    //Gamma 矫正
    if(GammaCorrection==1) {
        float gamma = 1.5;
        FragColor.rgb = pow(FragColor.rgb, vec3(1.0/gamma));
    }

    // 暗角
    if(Vignetting==1) {
        const float strength = 4.0;
        const float power = 0.1;
        vec2 tuv = TexCoord * (vec2(1.0) - TexCoord.yx);
        float vign = tuv.x*tuv.y * strength;
        vign = pow(vign, power);
        FragColor *= vign;
    }
}
