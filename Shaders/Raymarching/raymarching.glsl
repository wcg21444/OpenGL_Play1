bool isPointInBox(vec3 testPoint, vec3 boxMin, vec3 boxMax) {
    return testPoint.x < boxMax.x && testPoint.x > boxMin.x &&
    testPoint.z < boxMax.z && testPoint.z > boxMin.z &&
    testPoint.y < boxMax.y && testPoint.y > boxMin.y;
}
vec2 rayBoxDst(vec3 boundsMin, vec3 boundsMax, vec3 rayOrigin, vec3 invRaydir) {
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
    vec3 boxMax) {
    vec3 testPoint = startPoint;

    float interval = 0.1; // 每次步进间隔
    float intensity = 0.01 * interval;

    int hit = 0;
    float sum = 0.0;
    for (int i = 0; i < 8; ++i) {
        // 步进总长度
        if (i * interval > depthLimit) {
            break;
        }
        testPoint += direction * interval;

        // 光线在体积内行进
        if (isPointInBox(testPoint, boxMin, boxMax)) {
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
    vec4 color) {
    vec3 testPoint = startPoint;

    float interval = 0.01; // 每次步进间隔
    float intensity = 0.1 * interval;

    float density = 0.2;

    int hit = 0;
    float sum = 0.0;
    for (int i = 0; i < 256; ++i) {
        // 步进总长度
        if (i * interval > depthLimit) {
            break;
        }
        testPoint += direction * interval;

        // 光线在体积内行进
        if (isPointInBox(testPoint, boxMin, boxMax)) {
            hit = 1;
            sum += intensity;
            sum *= exp(density * interval);

            // vec3 lightDir = pointLightPos[0]-testPoint;
            vec3 lightDir = normalize(dirLightPos[0]);
            float limit = rayBoxDst(boxMin, boxMax, testPoint, 1 / lightDir).y;

            color += lightMarching(testPoint, lightDir, limit, boxMin, boxMax);
        }
    }
    if (hit == 0) {
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
    vec4 color) {
    vec2 rst = rayBoxDst(boxMin, boxMax, startPoint, 1.0 / direction);
    float boxDepth = rst.x;
    float boxInsideDepth = rst.y;

    float fragDepth = length(fragPos - startPoint);
    // skybox
    if (length(fragPos) == 0) {
        fragDepth = 10000000.f;
    }

    if (fragDepth < boxDepth) {
        // 物体阻挡Box
        return vec4(0.f);
    }
    float depthLimit;
    if ((fragDepth - boxDepth) < boxInsideDepth) {
        // 物体嵌入box,步进深度限制减小
        depthLimit = fragDepth - boxDepth;
    }
    else {
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
    vec4 color) {
    vec3 testPoint = startPoint;
    float sum = 0.0;
    float interval = 0.01; // 每次步进间隔
    float intensity = 1.5 * interval;
    float density = 0.1f;
    for (int i = 0; i < 1024; ++i) {
        // 步进总长度
        if (i * interval > depthLimit) {
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
    vec4 color) {
    vec2 rst = rayBoxDst(boxMin, boxMax, startPoint, 1.0 / direction);
    float boxDepth = rst.x;
    float boxInsideDepth = rst.y;

    float fragDepth = length(fragPos - startPoint);
    // skybox
    if (length(fragPos) == 0) {
        fragDepth = 10000000.f;
    }

    if (fragDepth < boxDepth) {
        // 物体阻挡Box
        return vec4(0.f);
    }
    float depthLimit;
    if ((fragDepth - boxDepth) < boxInsideDepth) {
        // 物体嵌入box,步进深度限制减小
        depthLimit = fragDepth - boxDepth;
    }
    else {
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
    vec4 color) {
    return lightVolumeRayMarching(startPoint, direction, fragPos, boxMin, boxMax, color);
}
