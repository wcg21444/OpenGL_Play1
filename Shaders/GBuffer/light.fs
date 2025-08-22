
#version 330 core
out vec4 LightResult;
in vec2 TexCoord;

/*****************视口大小******************************************************************/
uniform int width = 1600;
uniform int height = 900;

/*****************GBuffer输入*****************************************************************/
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;
uniform samplerCube depthMap; // debug

/*****************SSAO输入******************************************************************/
uniform sampler2D ssaoTex;

/*****************点光源设置******************************************************************/
const int MAX_LIGHTS = 10;  // Maximum number of lights supported
uniform int numPointLights; // actual number of lights used
uniform samplerCube shadowCubeMaps[MAX_LIGHTS];
uniform vec3 pointLightPos[MAX_LIGHTS];
uniform vec3 pointLightIntensity[MAX_LIGHTS];
uniform float pointLightFar;

/*****************定向光源设置******************************************************************/
const int MAX_DIR_LIGHTS = 5; // Maximum number of lights supported
uniform int numDirLights;     // actual number of lights used
uniform sampler2D dirDepthMap[MAX_DIR_LIGHTS];
uniform vec3 dirLightPos[MAX_DIR_LIGHTS];
uniform vec3 dirLightIntensity[MAX_DIR_LIGHTS];
uniform mat4 dirLightSpaceMatrix[MAX_DIR_LIGHTS];
/*****************阴影采样设置******************************************************************/
vec2 noiseScale = vec2(width / 16.0, height / 16.0);
uniform sampler2D shadowNoiseTex;
uniform vec3 shadowSamples[128];
uniform int n_samples;
uniform float blurRadius = 0.1f;

/*****************环境光******************************************************************/
uniform vec3 ambientLight;

/*****************天空盒******************************************************************/
uniform vec3 skyboxSamples[32];
uniform samplerCube skybox;
uniform mat4 view; // 视图矩阵

/*****************TBN******************************************************************/
vec3 randomVec = vec3(texture(shadowNoiseTex, TexCoord *noiseScale).xy / 2, 1.0f);
vec3 normal = texture(gNormal, TexCoord).rgb;
vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal)); // WorldSpace
vec3 bitangent = cross(normal, tangent);
mat3 TBN = mat3(tangent, bitangent, normal);

/*****************Camera设置******************************************************************/
uniform float nearPlane;
uniform float farPlane;
uniform vec3 eyePos;
uniform vec3 eyeFront;
uniform vec3 eyeUp;
uniform float fov;

/*****************toggle设置******************************************************************/
uniform int SSAO;
uniform int DirShadow;
uniform int PointShadow;
uniform int Skybox;

/*******************************RayMarching输入**************************************************/

/*******************************RayMarching******************************************************/
bool isPointInBox(vec3 testPoint, vec3 boxMin, vec3 boxMax)
{
    return testPoint.x < boxMax.x && testPoint.x > boxMin.x &&
           testPoint.z < boxMax.z && testPoint.z > boxMin.z &&
           testPoint.y < boxMax.y && testPoint.y > boxMin.y;
}
vec2 rayBoxDst(vec3 boundsMin, vec3 boundsMax, vec3 rayOrigin, vec3 invRaydir)
{
    vec3 t0 = (boundsMin - rayOrigin) * invRaydir;
    vec3 t1 = (boundsMax - rayOrigin) * invRaydir;
    vec3 tmin = min(t0, t1);
    vec3 tmax = max(t0, t1);

    float dstA = max(max(tmin.x, tmin.y), tmin.z); // 进入点
    float dstB = min(tmax.x, min(tmax.y, tmax.z)); // 出去点

    float dstToBox = max(0, dstA);
    float dstInsideBox = max(0, dstB - dstToBox);
    return vec2(dstToBox, dstInsideBox);
}

vec4 lightMarching(
    vec3 startPoint,
    vec3 direction,
    float depthLimit,
    vec3 boxMin,
    vec3 boxMax)
{
    vec3 testPoint = startPoint;

    float interval = 0.1; // 每次步进间隔
    float intensity = 0.01 * interval;

    int hit = 0;
    float sum = 0.0;
    for (int i = 0; i < 8; ++i)
    {
        // 步进总长度
        if (i * interval > depthLimit)
        {
            break;
        }
        testPoint += direction * interval;

        // 光线在体积内行进
        if (isPointInBox(testPoint, boxMin, boxMax))
        {
            hit = 1;

            sum += intensity / depthLimit;
        }
    }
    return -sum * vec4(dirLightIntensity[0] / length(dirLightIntensity[0]), 1.0f) / 1;
}

vec4 computeCloudRayMarching(
    vec3 startPoint,
    vec3 direction,
    float depthLimit,
    vec3 boxMin,
    vec3 boxMax,
    vec4 color)
{
    vec3 testPoint = startPoint;

    float interval = 0.01; // 每次步进间隔
    float intensity = 0.1 * interval;

    float density = 0.2;

    int hit = 0;
    float sum = 0.0;
    for (int i = 0; i < 256; ++i)
    {
        // 步进总长度
        if (i * interval > depthLimit)
        {
            break;
        }
        testPoint += direction * interval;

        // 光线在体积内行进
        if (isPointInBox(testPoint, boxMin, boxMax))
        {
            hit = 1;
            sum += intensity;
            sum *= exp(density * interval);

            // vec3 lightDir = pointLightPos[0]-testPoint;
            vec3 lightDir = normalize(dirLightPos[0]);
            float limit = rayBoxDst(boxMin, boxMax, testPoint, 1 / lightDir).y;

            color += lightMarching(testPoint, lightDir, limit, boxMin, boxMax);
        }
    }
    if (hit == 0)
    {
        return vec4(0.0f);
    }

    return sum * color;
}
vec4 cloudRayMarching(
    vec3 startPoint,
    vec3 direction,
    vec3 fragPos,
    vec3 boxMin,
    vec3 boxMax,
    vec4 color)
{
    vec2 rst = rayBoxDst(boxMin, boxMax, startPoint, 1.0 / direction);
    float boxDepth = rst.x;
    float boxInsideDepth = rst.y;

    float fragDepth = length(fragPos - startPoint);
    // skybox
    if (length(fragPos) == 0)
    {
        fragDepth = 10000000.f;
    }

    if (fragDepth < boxDepth)
    {
        // 物体阻挡Box
        return vec4(0.f);
    }
    float depthLimit;
    if ((fragDepth - boxDepth) < boxInsideDepth)
    {
        // 物体嵌入box,步进深度限制减小
        depthLimit = fragDepth - boxDepth;
    }
    else
    {
        depthLimit = boxInsideDepth;
    }
    return computeCloudRayMarching(startPoint + direction * rst.x, direction, depthLimit, boxMin, boxMax, color);
}
/*********************************LightVolume******************************************************/
vec4 computeLightVolumeRayMarching(
    vec3 startPoint,
    vec3 direction,
    float depthLimit,
    vec3 boxMin,
    vec3 boxMax,
    vec4 color)
{
    vec3 testPoint = startPoint;
    float sum = 0.0;
    float interval = 0.01; // 每次步进间隔
    float intensity = 1.5 * interval;
    float density = 0.1f;
    for (int i = 0; i < 1024; ++i)
    {
        // 步进总长度
        if (i * interval > depthLimit)
        {
            break;
        }
        testPoint += direction * interval;
        if (testPoint.x < boxMax.x && testPoint.x > boxMin.x &&
            testPoint.z < boxMax.z && testPoint.z > boxMin.z &&
            testPoint.y < boxMax.y && testPoint.y > boxMin.y)
            sum += intensity;
        sum *= exp(-density * interval);
    }
    return sum * color;
}
vec4 lightVolumeRayMarching(
    vec3 startPoint,
    vec3 direction,
    vec3 fragPos,
    vec3 boxMin,
    vec3 boxMax,
    vec4 color)
{
    vec2 rst = rayBoxDst(boxMin, boxMax, startPoint, 1.0 / direction);
    float boxDepth = rst.x;
    float boxInsideDepth = rst.y;

    float fragDepth = length(fragPos - startPoint);
    // skybox
    if (length(fragPos) == 0)
    {
        fragDepth = 10000000.f;
    }

    if (fragDepth < boxDepth)
    {
        // 物体阻挡Box
        return vec4(0.f);
    }
    float depthLimit;
    if ((fragDepth - boxDepth) < boxInsideDepth)
    {
        // 物体嵌入box,步进深度限制减小
        depthLimit = fragDepth - boxDepth;
    }
    else
    {
        depthLimit = boxInsideDepth;
    }
    return computeLightVolumeRayMarching(startPoint + direction * rst.x, direction, depthLimit, boxMin, boxMax, color);
}

vec4 lightVolume(
    vec3 startPoint,
    vec3 direction,
    vec3 fragPos,
    vec3 boxMin,
    vec3 boxMax,
    vec4 color)
{
    return lightVolumeRayMarching(startPoint, direction, fragPos, boxMin, boxMax, color);
}
/*******************************Lighting******************************************************/

vec3 fragViewSpaceDir(vec2 uv)
{
    vec3 dir = vec3(uv - vec2(0.5f, 0.5f), 0.f);
    dir.y = dir.y * tan(radians(fov / 2)) * 2;
    dir.x = dir.x * tan(radians(fov / 2)) * (float(width) / float(height)) * 2;
    dir.z = -1.0f;
    dir = normalize(inverse(mat3(view)) * dir);
    return dir;
}

vec3 sampleSkybox(vec2 uv, samplerCube _skybox)
{
    // vec3 uv_centered = vec3(uv-vec2(0.5f,0.5f),0.f);
    vec3 dir = fragViewSpaceDir(uv);
    // vec3 dir = normalize(eyeFront+nearPlane*uv_centered*2*tan(radians(fov)/2));
    return vec3(texture(_skybox, dir));
    // return dir;
}

vec3 computeSkyboxAmbient(samplerCube _skybox)
{
    vec3 ambient = vec3(0.f);
    int samplesN = 32;
    for (int i = 0; i < samplesN; ++i)
    {
        vec3 sample_dir = TBN * (skyboxSamples[i] + vec3(0.0, 0.0, 1.f));
        ambient += texture(_skybox, normalize(sample_dir)).rgb;
    }
    return ambient / samplesN;
}

float computeDirLightShadow(vec3 fragPos, mat4 _dirLightSpaceMatrix, sampler2D _dirDepthMap)
{
    // perform perspective divide
    if (DirShadow == 0)
    {
        return 0.f;
    }
    vec4 lightSpaceFragPos = _dirLightSpaceMatrix * vec4(fragPos, 1.0f);

    // 计算遮挡物与接受物的平均距离
    float d = 0.f;
    int occlusion_times = 0;
    for (int k = 0; k < n_samples; ++k)
    {
        vec4 sampleOffset = _dirLightSpaceMatrix * vec4(TBN * shadowSamples[k] * blurRadius * 60, 0.0f);
        vec4 fragPosLightSpace = lightSpaceFragPos + sampleOffset; // Dir Light View Space
        vec3 projCoords = (fragPosLightSpace.xyz) / fragPosLightSpace.w;
        // transform to [0,1] range
        projCoords = projCoords * 0.5 + 0.5;
        // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
        float closestDepth = texture(_dirDepthMap, projCoords.xy).r;
        // get depth of current fragment from light's perspective
        float currentDepth = projCoords.z;
        float bias = 0.000005f;
        if (currentDepth - closestDepth - bias > 0)
        {
            occlusion_times++;
            d += (currentDepth - closestDepth) * 2000; // 深度值换算与光源farplane数值有关
        }
    }
    d = d / occlusion_times;
    float factor = 0.f;
    for (int j = 0; j < n_samples; ++j)
    {
        vec4 sampleOffset = _dirLightSpaceMatrix * vec4(
                                                       TBN *
                                                           shadowSamples[j] *
                                                           blurRadius * 20 * pow(d, 2),
                                                       0.0f);
        vec4 fragPosLightSpace = lightSpaceFragPos + sampleOffset; // Dir Light View Space
        vec3 projCoords = (fragPosLightSpace.xyz) / fragPosLightSpace.w;
        // transform to [0,1] range
        projCoords = projCoords * 0.5 + 0.5;
        // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
        float closestDepth = texture(_dirDepthMap, projCoords.xy).r;
        // get depth of current fragment from light's perspective
        float currentDepth = projCoords.z;
        float bias = 0.000005f;
        // check whether current frag pos is in shadow
        factor += currentDepth - closestDepth - bias > 0 ? 1.0f : 0.0f;
    }

    return factor / n_samples;
}

float computePointLightShadow(vec3 fragPos, vec3 fragNorm, vec3 pointLightPos, samplerCube _depthMap)
{
    if (PointShadow == 0)
    {
        return 0.f;
    }
    vec3 dir = fragPos - pointLightPos;

    float curr_depth = length(dir);
    float omega = -dot(fragNorm, dir) / curr_depth;
    float bias = 0.05f / tan(omega); // idea: bias increase based on center distance
    // shadow test

    // 计算遮挡物与接受物的平均距离
    float d = 0.f;
    int occlusion_times = 0;
    for (int k = 0; k < n_samples; ++k)
    {
        vec3 sampleOffset = TBN * shadowSamples[k] * 4.f;
        vec3 dir_sample = sampleOffset + fragPos - pointLightPos;
        float curr_depth_sample = length(dir_sample);
        float cloest_depth_sample = texture(_depthMap, dir_sample).r;

        cloest_depth_sample *= pointLightFar;
        if (curr_depth_sample - cloest_depth_sample - bias > 0.01f)
        {
            occlusion_times++;
            d += curr_depth_sample - cloest_depth_sample;
        }
    }
    d = d / occlusion_times;
    float factor = 0.f;
    for (int j = 0; j < n_samples; ++j)
    {
        vec3 sampleOffset = TBN * shadowSamples[j] * blurRadius * pow(curr_depth / 12, 2) * d / n_samples * 64;
        vec3 dir_sample = sampleOffset + fragPos - pointLightPos;
        float curr_depth_sample = length(dir_sample);
        float cloest_depth_sample = texture(_depthMap, dir_sample).r;
        cloest_depth_sample *= pointLightFar;
        factor += (curr_depth_sample - cloest_depth_sample - bias > 0.f ? 1.0 : 0.0);
        // factor = curr_depth_sample;
    }
    return (factor) / n_samples; // return shadow
}

vec3 dirLightDiffuse(vec3 fragPos, vec3 n)
{

    vec3 diffuse = vec3(0.0f);
    for (int i = 0; i < numDirLights; ++i)
    {

        vec3 l = normalize(dirLightPos[i]);
        float rr = dot(l, l);
        float shadowFactor = 1 - computeDirLightShadow(fragPos, dirLightSpaceMatrix[i], dirDepthMap[i]);
        diffuse +=
            shadowFactor * dirLightIntensity[i] / rr * max(0.f, dot(n, l));
    }
    return diffuse;
}

vec3 dirLightSpec(vec3 fragPos, vec3 n)
{
    vec3 specular = vec3(0.0f);
    for (int i = 0; i < numDirLights; ++i)
    {
        vec3 l = normalize(dirLightPos[i]);
        float specularStrength = 0.01f;
        vec3 viewDir = normalize(eyePos - fragPos);
        vec3 reflectDir = reflect(-l, n);
        float shadowFactor = 1 - computeDirLightShadow(fragPos, dirLightSpaceMatrix[i], dirDepthMap[i]);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 128);
        specular += shadowFactor * specularStrength * spec * dirLightIntensity[i] * 160;
    }
    return specular;
}

vec3 pointLightDiffuse(vec3 fragPos, vec3 n)
{
    vec3 diffuse = vec3(0.f, 0.f, 0.f);

    for (int i = 0; i < numPointLights; ++i)
    {
        vec3 l = pointLightPos[i] - fragPos;
        float rr = pow(dot(l, l), 0.6) * 10;
        l = normalize(l);

        float shadow_factor = 1 - computePointLightShadow(fragPos, n, pointLightPos[i], shadowCubeMaps[i]);
        diffuse += pointLightIntensity[i] / rr * max(0.f, dot(n, l)) * shadow_factor;
    }
    return diffuse;
}

vec3 pointLightSpec(vec3 fragPos, vec3 n)
{
    vec3 specular = vec3(0.f, 0.f, 0.f);

    for (int i = 0; i < numPointLights; ++i)
    {
        vec3 l = pointLightPos[i] - fragPos;
        float rr = pow(dot(l, l), 0.6) * 10;
        l = normalize(l);
        float spec = 0.f;
        float specularStrength = 0.005f;

        float shadow_factor = 1 - computePointLightShadow(fragPos, n, pointLightPos[i], shadowCubeMaps[i]);

        vec3 viewDir = normalize(eyePos - fragPos);
        vec3 reflectDir = reflect(-l, n);
        spec = pow(max(dot(viewDir, reflectDir), 0.0), 64);
        specular += specularStrength * spec * pointLightIntensity[i] * shadow_factor;
    }
    return specular;
}

/*****************************天空大气计算********************************************************** */
const float PI = 3.1415926535;
//全局变量
vec3 camDir = vec3(0.f);
vec3 camPos = vec3(0.f);
vec3 sunDir = vec3(0.f);
vec3 earthCenter;
const vec4 betaRayleigh = vec4(5.8e-6, 1.35e-5, 3.31e-5,1.0f);
float itvl;
uniform int maxStep;
uniform float skyHeight;
uniform float earthRadius;
uniform float skyIntensity;
uniform float H;

vec3 intersectSky(vec3 ori, vec3 dir) {
    // 假设这些常量已经在全局或函数外定义
    float kAtmosphereRadius = earthRadius + skyHeight;
    float kEarthRadius = earthRadius;

    vec3 relativeOrigin = ori - earthCenter;

    // 计算二次方程的系数 A, B, C
    float a = dot(dir, dir);
    float b = 2.0f * dot(relativeOrigin, dir);
    float cS = dot(relativeOrigin, relativeOrigin) - kAtmosphereRadius * kAtmosphereRadius;
    float cE = dot(relativeOrigin, relativeOrigin) - kEarthRadius * kEarthRadius;

    // --- 检查光线与大气层的交点 ---
    float discrS = b * b - 4.0f * a * cS;
    if (discrS < 0.0f) {
        // 判别式小于0，光线没有与大气层相交
        return vec3(0.0f);
    }
    
    // 计算大气层的两个交点参数 t
    float tS1 = (-b - sqrt(discrS)) / (2.0f * a);
    float tS2 = (-b + sqrt(discrS)) / (2.0f * a);

    // --- 确定进入大气的第一个交点 tS ---
    float tS;
    if (tS1 > 0.0f) {
        tS = tS1;
    } else if (tS2 > 0.0f) {
        tS = tS2;
    } else {
        // 两个交点都在光线起点之后（tS < 0），光线朝远离大气的方向传播
        return vec3(0.0f);
    }

    // --- 检查光线是否被地球遮挡 ---
    float discrE = b * b - 4.0f * a * cE;
    if (discrE >= 0.0f) {
        // 光线与地球相交
        float tE1 = (-b - sqrt(discrE)) / (2.0f * a);
        float tE2 = (-b + sqrt(discrE)) / (2.0f * a);

        // 检查最靠近的地球交点是否在进入大气层之前
        // tE1 和 tE2 都是光线与地球的交点参数
        // 如果 tE1 大于 0 且小于 tS，说明光线在进入大气之前就被地球遮挡了
        // 这里我们只需要检查 tE1，因为它总是更靠近的交点
        if (tE1 > 0.0f && tE1 < tS) {
            return vec3(0.0f); // 光线被地球遮挡
        }
    }

    // 返回进入大气层的交点坐标
    return ori + dir * tS;
}

// 计算一个点p相对于地球表面的高度
float heightToGround(vec3 p) {

    // return (length(p-earthCenter) - earthRadius);
    return p.y;
}

float phase(vec3 _camDir,vec3 _sunDir){
    float cosine = dot(_camDir,_sunDir)/length(_camDir)/length(_sunDir);
    return 3.0f/16.0f*PI*(1+cosine*cosine);
}

float rho(float h){
    return exp(-h/H);//大气密度近似
}

vec4 transmittance(vec3 ori, vec3 end)
{
    // if(heightToGround(ori)<0.0f||heightToGround(end)<0.0f){
    //     return vec4(0.0f);
    // }
    vec4 t; // 透射率

    // int tMaxStep = 32; //透射率步进步数
    float dist = length(end - ori);
    float tItvl = dist / float(maxStep);

    float opticalDepth = 0.f;
    for(int i = 0; i < maxStep; ++i)
    {
        vec3 p =  i  * (end - ori);
        float h = heightToGround(p);
        opticalDepth+= tItvl*rho(h);
    }
    
    t = vec4(
    exp(-betaRayleigh.r*opticalDepth),
    exp(-betaRayleigh.g*opticalDepth),
    exp(-betaRayleigh.b*opticalDepth),
    1.0f    
    );
    return t;
}

vec4 scatterCoefficient(vec3 p){
    vec3 intersection = intersectSky(camPos,camDir);
    // if(intersection==vec3(0.0f)){
    //     return vec4(0.0f);
    // }
    float h =heightToGround(p);//散射点高度
    return betaRayleigh* rho(h);
}

vec4 scatterColor(vec3 p){
    vec3 skyInsertion = intersectSky(p,sunDir);
    // if(skyInsertion == vec3(0.0f)){
    //     return vec4(0.000f);//散射点阳光被地面阻挡   
    // }
    vec4 t1 = transmittance(camPos, p);//散射点到摄像机的透射率

    vec4 t2 = transmittance(p, skyInsertion);//边界到散射点的透射率
    if(skyInsertion == vec3(0.0f)){
        t2 =  vec4(0.000f);//散射点阳光被地面阻挡   
    }
    // return dirLightIntensity[0]*decay(p,border)*scatterCoefficient(p)
    // return vec4(dirLightIntensity[0],1.0f)*decay1*scatterCoefficient(p)*decay2;
    // return t1;
    return t2*t1*scatterCoefficient(p);
    // return t2*t1;
    // return t2;//T2决定日出日落颜色变化
    // return normalize(vec4(border(p,-sunDir),1.0f))*1e5;
}

vec4 computeSkyColor(vec2 TexCoord)
{
    vec4 skyColor;
    itvl = skyHeight / float(maxStep);
    camDir = fragViewSpaceDir(TexCoord);
    camPos = eyePos;

    sunDir = dirLightPos[0];
    earthCenter = vec3(0.0f,-earthRadius, 0.0f); // 地球球心，位于地面原点正下方
    
    for(int i = 0; i<maxStep;++i){

        skyColor += scatterColor(camPos+i*itvl*camDir);
    }

    //从循环中提出常数,减少计算量
    return vec4(dirLightIntensity[0],1.0f)*phase(camDir,sunDir)*skyColor*skyIntensity;
}

void main()
{
    // Diffuse Caculation

    LightResult = vec4(0.f, 0.f, 0.f, 1.f); // Initialize LightResult

    vec3 FragPos = texture(gPosition, TexCoord).rgb;
    vec3 n = normalize(texture(gNormal, TexCoord).rgb);
    vec3 diffuse;
    vec3 specular;
    vec3 ambient;
    // 天空计算
    if (length(FragPos) == 0)
    {
        if (Skybox == 1)
        {
            LightResult = vec4(sampleSkybox(TexCoord, skybox), 1.0f); // 采样天空盒
        }
        else
{        LightResult = computeSkyColor(TexCoord);}
    }
    else
    {
        diffuse =
            pointLightDiffuse(FragPos, n) +
            dirLightDiffuse(FragPos, n);

        specular =
            pointLightSpec(FragPos, n) +
            dirLightSpec(FragPos, n);

        vec3 ambient = ambientLight;
        if (Skybox == 1)
        {
            ambient += computeSkyboxAmbient(skybox);
        }
        // ambient = (ambientLight +
        //            computeSkyboxAmbient(skybox));

        LightResult += vec4(diffuse * texture(gAlbedoSpec, TexCoord).rgb, 1.f);
        LightResult += vec4(specular * texture(gAlbedoSpec, TexCoord).a, 1.f);
        LightResult += vec4(ambient * texture(gAlbedoSpec, TexCoord).rgb, 1.f);
    }

    const vec3 BoxMin = vec3(2.0f, -2.0f, -2.0f);
    const vec3 BoxMax = vec3(6.0f, 2.0f, 2.0f);
    vec3 dir = fragViewSpaceDir(TexCoord);

    // vec4 cloud = cloudRayMarching(eyePos.xyz, dir, FragPos, BoxMin, BoxMax, vec4(0.5f, 0.5f, 0.5f, 1.0f));
    // LightResult -= cloud; // 散射吸收,减色

    const vec3 LightVolueBoxMin = vec3(-0.5f, -0.5f, -0.5f);
    const vec3 LightVolueBoxMax = vec3(0.5f, 0.5f, 0.5f);
    for (int i = 0; i < numPointLights; ++i)
    {
        LightResult += lightVolume(
            eyePos.xyz,
            dir,
            FragPos,
            LightVolueBoxMin + pointLightPos[i],
            LightVolueBoxMax + pointLightPos[i],
            vec4(pointLightIntensity[i], 1.0f) / length(pointLightIntensity[i]));
    }
}
