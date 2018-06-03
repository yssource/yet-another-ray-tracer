#pragma once

#include "../core/material.h"

namespace yart
{

class Dielectric : public Material
{
public:
    explicit Dielectric(float rindex) : rindex(rindex) {}
    bool scatter(const Eigen::Vector3f& rayin,
                 const Eigen::Vector3f& hitpt,
                 const Eigen::Vector3f& normal,
                 Eigen::Vector3f& rayout,
                 Eigen::Array3f& attenuation) const override;

public:
    float rindex;
};

} // namespace yart