#version 330 core
out vec4 Result;
in vec2 TexCoord;
uniform sampler2D InputTexture;
uniform int RowSummary;

void main()
{
    ivec2 textureSize = textureSize(InputTexture, 0);
    if (RowSummary == 1)
    {
        float sum = 0.0f;
        float sumSquare = 0.0f;
        int x_coord = int(TexCoord.x * textureSize.x);

        // 从左侧开始累加
        const float bias = 0.5f;
        for (int i = 0; i <= x_coord; ++i)
        {
            float sample = texelFetch(InputTexture, ivec2(i, TexCoord.y * textureSize.y), 0).r;
            sum += sample - 0.5f;
            sumSquare += sample * sample - 0.5f;
        }
        Result = vec4(sum, sumSquare, 0.0f, 1.0f);
    }
    if (RowSummary == 0)
    {
        Result = texture(InputTexture, TexCoord);
        float sum = 0.0f;
        float sumSquare = 0.0f;
        int y_coord = int(TexCoord.y * textureSize.y);

        for (int j = 0; j <= y_coord; ++j)
        {
            vec2 sample = texelFetch(InputTexture, ivec2(TexCoord.x * textureSize.x, j), 0).rg;
            sum += sample.r;
            sumSquare += sample.g;
        }
        Result = vec4(sum, sumSquare, 0.0f, 1.0f);
    }
}