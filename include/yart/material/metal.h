#pragma once

#include <yart/core/material.h>

namespace yart
{

class YART_API Metal : public Material
{
public:
    Metal(const Eigen::Array3f& albedo, float roughness);
    Metal(float r, float g, float b, float roughness);
    bool scatter(const RTCRayHit& rayhit,
                 Eigen::Vector3f& rayout,
                 Eigen::Array3f& attenuation) const override;

public:
    Eigen::Array3f albedo;
    float roughness;
};

} // namespace yart
