#pragma once

#include <random>
#include <glm/glm.hpp>
#include <vector>

namespace Random
{
    inline std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // random floats between [0.0, 1.0]
    inline std::default_random_engine generator;
    inline std::vector<glm::vec3> GenerateNoise()
    {
        std::vector<glm::vec3> Noises;
        for (unsigned int i = 0; i < 64; i++)
        {
            glm::vec3 noise(
                randomFloats(generator) * 2.0 - 1.0,
                randomFloats(generator) * 2.0 - 1.0,
                0.0f);
            Noises.push_back(noise);
        }
        return Noises;
    }
    inline std::vector<glm::vec3> GenerateSSAOKernel()
    {
        std::vector<glm::vec3> ssaoKernel;
        for (unsigned int i = 0; i < 64; ++i)
        {
            glm::vec3 sample(
                randomFloats(generator) * 2.0 - 1.0,
                randomFloats(generator) * 2.0 - 1.0,
                randomFloats(generator));
            float scale = (float)i / 64.0;
            scale = std::lerp(0.1f, 1.0f, scale * scale);
            sample *= scale;
            ssaoKernel.push_back(sample);
        }
        return ssaoKernel;
    }
    inline std::vector<glm::vec3> GenerateShadowKernel(unsigned int n_samples)
    {
        std::vector<glm::vec3> shadowKernel;
        for (unsigned int i = 0; i < n_samples; ++i)
        {
            glm::vec3 sample(
                randomFloats(generator) * 2.0 - 1.0,
                randomFloats(generator) * 2.0 - 1.0,
                0.f); // 平面分布
            float scale = (float)i / (float)n_samples;
            scale = std::lerp(0.1f, 1.0f, scale * scale);
            sample *= scale;
            shadowKernel.push_back(sample);
        }
        return shadowKernel;
    }
    inline std::vector<glm::vec3> GenerateSemiSphereKernel(unsigned int n_samples)
    {
        std::vector<glm::vec3> kernel;
        for (unsigned int i = 0; i < n_samples; ++i)
        {
            glm::vec3 sample(
                randomFloats(generator) * 2.0 - 1.0,
                randomFloats(generator) * 2.0 - 1.0,
                randomFloats(generator)); // 半球均匀
            float scale = (float)i / (float)n_samples;
            scale = std::lerp(0.1f, 1.0f, scale * scale);
            sample *= scale;
            kernel.push_back(sample);
        }
        return kernel;
    }
}