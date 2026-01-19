#include <vector>
#include <iostream>
#include <math.h>

const float EPS = 0.0001;

struct Pixel
{
    u_int8_t r = 0;
    u_int8_t g = 0;
    u_int8_t b = 0;
    u_int8_t a = 0;
};

void setWhite_4(int index, Image &mask)
{
    ((char *)mask.data)[index] = 255;
    ((char *)mask.data)[index + 1] = 255;
    ((char *)mask.data)[index + 2] = 255;
    ((char *)mask.data)[index + 3] = 255;
}

void setBlack_4(int index, Image &mask)
{
    ((char *)mask.data)[index] = 0;
    ((char *)mask.data)[index + 1] = 0;
    ((char *)mask.data)[index + 2] = 0;
    ((char *)mask.data)[index + 3] = 255;
}

float convertToDecimal(int v)
{
    return (float)v / 255;
}

float convertToLinear(float v)
{
    if (abs(v - 0.04045) < EPS)
        return v / 12.95;
    else
        return pow((v + 0.055) / 1.055, 2.4);
}

float getLuminescence(const Pixel p)
{
    float rlin = convertToLinear(convertToDecimal(p.r));
    float glin = convertToLinear(convertToDecimal(p.g));
    float blin = convertToLinear(convertToDecimal(p.b));

    return rlin * 0.2126 + glin * 0.7152 + blin * 0.0722;
}

bool cmp(const Pixel &a, const Pixel &b)
{
    return getLuminescence(a) > getLuminescence(b);
}

std::vector<Pixel> convertToPixRGBA(const char *data, int size)
{
    Pixel curr;
    std::vector<Pixel> res;

    res.reserve(size);

    for (size_t i = 0; i < size * 4; i++)
    {
        curr.r = data[i++];
        curr.g = data[i++];
        curr.b = data[i++];
        curr.a = data[i];

        res.push_back(curr);
    }

    return res;
}

void convertToRGBAPix(const std::vector<Pixel> &pixels, char *data)
{
    for (size_t i = 0; i < pixels.size(); i++)
    {
        data[i * 4 + 0] = pixels[i].r;
        data[i * 4 + 1] = pixels[i].g;
        data[i * 4 + 2] = pixels[i].b;
        data[i * 4 + 3] = pixels[i].a;
    }
}
