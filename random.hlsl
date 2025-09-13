#ifndef RANDOM_HLSL
#define RANDOM_HLSL

uint Hash(uint x)
{
    x += (x << 10);
    x ^= (x >> 6);
    x += (x << 3);
    x ^= (x >> 11);
    x += (x << 15);
    return x;
}

uint Hash(uint2 v)
{
    return Hash(v.x ^ Hash(v.y));
}

uint Hash(uint3 v)
{
    return Hash(v.x ^ Hash(v.y) ^ Hash(v.z));
}

uint Hash(uint4 v)
{
    return Hash(v.x ^ Hash(v.y) ^ Hash(v.z) ^ Hash(v.w));
}

float FloatConstruct(uint m)
{
    const uint ieeeMantissa = 0x007FFFFF;
    const uint ieeeOne = 0x3F800000;
    m &= ieeeMantissa;
    m |= ieeeOne;
    float f = asfloat(m);
    return f - 1.0f;
}

float Random(float x)
{
    return FloatConstruct(Hash(asuint(x)));
}

float Random(float2 v)
{
    return FloatConstruct(Hash(asuint(v)));
}

float Random(float3 v)
{
    return FloatConstruct(Hash(asuint(v)));
}

float Random(float4 v)
{
    return FloatConstruct(Hash(asuint(v)));
}

float Random(float v, float a, float b)
{
    return a + (b - a) * Random(v);
}

float Random(float2 v, float a, float b)
{
    return a + (b - a) * Random(v);
}

float Random(float3 v, float a, float b)
{
    return a + (b - a) * Random(v);
}

float Random(float4 v, float a, float b)
{
    return a + (b - a) * Random(v);
}

float4 Random(uint2 id, uint count, uint depth)
{
    return float4(
        FloatConstruct(Hash(uint4(id.x, id.y, depth, (count + 1) * 17u))),
        FloatConstruct(Hash(uint4(id.x, id.y, depth, (count + 1) * 73u))),
        FloatConstruct(Hash(uint4(id.x, id.y, depth, (count + 1) * 97u))),
        FloatConstruct(Hash(uint4(id.x, id.y, depth, (count + 1) * 57u)))
    ) * 2.0f - 1.0f;
}

#endif