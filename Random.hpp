#include <random>
#include <glm/glm.hpp>
#include <vector>

namespace Random
{
    std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // random floats between [0.0, 1.0]
    std::default_random_engine generator;
    inline std::vector<glm::vec3> GenerateSSAONoise()
    {
        std::vector<glm::vec3> ssaoNoise;
        for (unsigned int i = 0; i < 64; i++)
        {
            glm::vec3 noise(
                randomFloats(generator) * 2.0 - 1.0,
                randomFloats(generator) * 2.0 - 1.0,
                0.0f);
            ssaoNoise.push_back(noise);
        }
        return ssaoNoise;
    }
}