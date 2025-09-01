#define NO_INTERSECTION vec3(1.0f / 0.0f)

vec3 intersectSky(vec3 ori, vec3 dir) {
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
        return NO_INTERSECTION;
    }

    // 计算大气层的两个交点参数 t
    float tS1 = (-b - sqrt(discrS)) / (2.0f * a);
    float tS2 = (-b + sqrt(discrS)) / (2.0f * a);

    // --- 确定进入大气的第一个交点 tS ---
    float tS;
    if (tS1 > 0.0f) {
        tS = tS1;
    }
    else if (tS2 > 0.0f) {
        tS = tS2;
    }
    else {
        // 两个交点都在光线起点之后（tS < 0），光线朝远离大气的方向传播
        // return vec3(0.0f);
    }

    // 返回进入大气层的交点坐标
    return ori + dir * tS;
}
vec3 intersectEarth(vec3 ori, vec3 dir) {
    float kEarthRadius = earthRadius;

    vec3 relativeOrigin = ori - earthCenter;

    // 计算二次方程的系数 A, B, C
    float a = dot(dir, dir);
    float b = 2.0f * dot(relativeOrigin, dir);
    float c = dot(relativeOrigin, relativeOrigin) - kEarthRadius * kEarthRadius;

    float discr = b * b - 4.0f * a * c;

    // 如果判别式小于0，说明没有交点
    if (discr < 0.0f) {
        // 返回一个特殊值来表示没有交点，例如一个“无效”向量
        return vec3(1.f / 0.0f);
    }

    // 计算两个交点参数t
    float t1 = (-b - sqrt(discr)) / (2.0f * a);
    float t2 = (-b + sqrt(discr)) / (2.0f * a);

    // 通常我们只关心“最近”的那个交点（t值最小且大于0）
    float t_intersect;

    if (t1 > 0.0f) {
        t_intersect = t1;
    }
    else if (t2 > 0.0f) {
        t_intersect = t2;
    }
    else {
        // 两个t值都小于等于0，表示光线从球体内部射出，或球体在光线背后
        // 此时可以认为没有有效交点
        return vec3(1.f / 0.0f); // 同样，返回一个特殊值
    }

    // 根据找到的有效t值计算交点坐标
    vec3 intersectionPoint = ori + t_intersect * dir;
    return intersectionPoint;
}

float heightToGround(vec3 p) {
    return (length(p - earthCenter) - earthRadius);
    // return p.y;
}

/// @brief 透射率lut uv映射
/// @param earthRadius 地面半径
/// @param skyRadius 天空半径
/// @param mu 天顶角余弦值
/// @param r 起始点半径
/// @return uv in LUT
vec2 transmittanceUVMapping(float earthRadius, float skyRadius, float mu, float r) {
    float H = sqrt(skyRadius * skyRadius - earthRadius * earthRadius); // 天顶与地面切线长度.
    float rho = sqrt(r * r - earthRadius * earthRadius);               // 起始点与地面相切的长度

    float discriminant = r * r * (mu * mu - 1.0f) + skyRadius * skyRadius;
    float d = max(0.0f, (-r * mu + sqrt(discriminant)));

    float d_min = skyRadius - r;
    float d_max = rho + H; // 起始点为r时,地平线路径长度最大值

    float x_mu = (d - d_min) / (d_max - d_min);
    float x_r = rho / H;

    return vec2(x_mu, x_r);
}

/// @brief 透射率lut uv反向映射
/// @param earthRadius 地面半径
/// @param skyRadius 天空半径
/// @param uv uv in LUT
/// @return mu 和 r
vec2 transmittanceUVInverseMapping(float earthRadius, float skyRadius, vec2 uv) {
    float x_mu = uv.x;
    float x_r = uv.y;

    float H = sqrt(skyRadius * skyRadius - earthRadius * earthRadius);
    float rho = x_r * H;
    float r = sqrt(rho * rho + earthRadius * earthRadius);

    float d_min = skyRadius - r;
    float d_max = rho + H;
    float d = x_mu * (d_max - d_min) + d_min;

    float mu = 0.0f;
    if (2.0f * d * r > 0.0f) {
        mu = (skyRadius * skyRadius - r * r - d * d) / (2.0f * d * r);
    }

    return vec2(mu, r);
}
vec2 rayToMuR(float earthRadius, float skyRadius, vec3 ori, vec3 end) {
    vec3 center = vec3(0.0, -earthRadius, 0.0);
    float R = length(ori - center);

    vec3 n = normalize(ori - center);

    vec3 dir = normalize(end - ori);

    float mu = dot(dir, n);
    return vec2(mu, R);
}

void MuRToRay(vec2 MuR, float earthRadius, out vec3 ori, out vec3 dir) {
    ori = vec3(0.0, MuR.y - earthRadius, 0.0);

    float dir_x = sqrt(max(0.0, 1.0 - MuR.x * MuR.x));
    float dir_y = MuR.x;
    float dir_z = 0.0;

    dir = vec3(dir_x, dir_y, dir_z);
}

// 从LUT获取两点透射率
vec4 getTransmittanceFromLUT(sampler2D LUT, float earthRadius, float skyRadius, vec3 ori, vec3 end) {
    vec3 dir = normalize(ori - end);
    vec3 n = normalize(ori - vec3(0.0f, -earthRadius, 0.0f));
    vec3 earthIntersection = intersectEarth(ori, -dir) ;
    int hitEarth = 0;

    if (earthIntersection != NO_INTERSECTION) {
        hitEarth = 1;
    }
    vec2 MuR_ori;
    vec2 MuR_end;
    if (hitEarth == 1) {
        MuR_ori = rayToMuR(earthRadius, skyRadius, ori, 2 * ori - end);
        MuR_end = rayToMuR(earthRadius, skyRadius, end, ori);
        float bias = 0.3f;//加上这个magic bias 近地面透射率的artifcat减少了
        MuR_end.y+=bias;
    }
    else {
        MuR_ori = rayToMuR(earthRadius, skyRadius, ori, end);//高空地平线走样严重
        MuR_end = rayToMuR(earthRadius, skyRadius, end, 2 * end - ori);
    }
    vec4 tori = texture(LUT, transmittanceUVMapping(earthRadius, skyRadius, MuR_ori.x, MuR_ori.y));
    vec4 tend = texture(LUT, transmittanceUVMapping(earthRadius, skyRadius, MuR_end.x, MuR_end.y));

    if (hitEarth == 1) {
        return tend / tori;
    }
    return tori / tend;
}
// 从LUT获取起点到天空透射率
vec4 getTransmittanceFromLUTSky(sampler2D LUT, float earthRadius, float skyRadius, vec3 ori, vec3 skyIntersection) {
    vec3 dir = normalize(ori - skyIntersection);
    vec3 n = normalize(ori - vec3(0.0f, -earthRadius, 0.0f));

    vec2 MuR_ori = rayToMuR(earthRadius, skyRadius, ori, skyIntersection);
    vec4 tori = texture(LUT, transmittanceUVMapping(earthRadius, skyRadius, MuR_ori.x, MuR_ori.y));

    return tori;
}