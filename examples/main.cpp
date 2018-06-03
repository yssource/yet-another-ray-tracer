#include <yart/core/device.h>
#include <yart/core/scene.h>
#include <yart/geometry/sphere.h>
#include <yart/material/lambertian.h>
#include <yart/material/metal.h>
#include <yart/material/dielectric.h>
#include <yart/camera/perspective.h>
#include <array>
#include <iostream>
#include <memory>
#include <random>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

constexpr const unsigned g_width = 1200;
constexpr const unsigned g_height = 800;
constexpr const unsigned g_samples = 100;
constexpr const unsigned g_depth = 50;

void gen_rand_scene(std::vector<std::unique_ptr<yart::Geometry>>& geometries,
                    std::vector<std::unique_ptr<yart::Material>>& materials,
                    const yart::Device& device)
{
    std::random_device rd;
    std::minstd_rand rd_gen(rd());
    std::uniform_real_distribution<> rd_number(0.0f, 1.0f);

    geometries.push_back(std::make_unique<yart::Sphere>(
        device, 1000.0f, Eigen::Vector3f(0.0f, -1000.0f, 0.0f)));
    materials.push_back(std::make_unique<yart::Lambertian>(0.5f, 0.5f, 0.5f));
    for (int a = -11; a < 11; ++a) {
        for (int b = -11; b < 11; ++b) {
            float rd_mat = rd_number(rd_gen);
            Eigen::Vector3f center(a + 0.9f * rd_number(rd_gen),
                                   0.2f,
                                   b + 0.9f * rd_number(rd_gen));
            if ((center - Eigen::Vector3f(4.0f, 2.0f, 0.0f)).norm() > 0.9f) {
                geometries.push_back(
                    std::make_unique<yart::Sphere>(device, 0.2f, center));
                if (rd_mat < 0.8f) { // diffuse
                    materials.push_back(std::make_unique<yart::Lambertian>(
                        rd_number(rd_gen) * rd_number(rd_gen),
                        rd_number(rd_gen) * rd_number(rd_gen),
                        rd_number(rd_gen) * rd_number(rd_gen)));
                }
                else if (rd_mat < 0.95f) { // metal
                    materials.push_back(std::make_unique<yart::Metal>(
                        0.5 * (1.0f + rd_number(rd_gen)),
                        0.5 * (1.0f + rd_number(rd_gen)),
                        0.5 * (1.0f + rd_number(rd_gen)),
                        0.5 * rd_number(rd_gen)));
                }
                else { // glass
                    materials.push_back(
                        std::make_unique<yart::Dielectric>(1.5f));
                }
            }
        }
    }

    geometries.push_back(std::make_unique<yart::Sphere>(
        device, 1.0f, Eigen::Vector3f(0.0f, 1.0f, 0.0f)));
    materials.push_back(std::make_unique<yart::Dielectric>(1.5f));
    geometries.push_back(std::make_unique<yart::Sphere>(
        device, 1.0f, Eigen::Vector3f(-4.0f, 1.0f, 0.0f)));
    materials.push_back(std::make_unique<yart::Lambertian>(0.4f, 0.2f, 0.1f));
    geometries.push_back(std::make_unique<yart::Sphere>(
        device, 1.0f, Eigen::Vector3f(4.0f, 1.0f, 0.0f)));
    materials.push_back(std::make_unique<yart::Metal>(0.7f, 0.6f, 0.5f, 0.0f));
}

int main(int argc, char* argv[])
{
    auto device = yart::Device();

    std::vector<std::unique_ptr<yart::Geometry>> geometries;
    std::vector<std::unique_ptr<yart::Material>> materials;
    gen_rand_scene(geometries, materials, device);
    auto scene = yart::Scene(device);
    for (size_t i = 0; i < geometries.size(); ++i) {
        scene.add(*geometries[i], *materials[i]);
    }

    yart::PerspectiveCamera camera(Eigen::Vector3f(13.0f, 2.0f, 3.0f),
                                   Eigen::Vector3f(0.0f, 0.0f, 0.0f),
                                   Eigen::Vector3f(0.0f, 1.0f, 0.0f),
                                   0.01f,
                                   20.0f,
                                   g_width,
                                   g_height);
    camera.zoom(10.0f);

    std::array<unsigned char, g_width * g_height * 3> pixels;
    scene.render(
        camera, pixels.data(), g_width, g_height, g_samples, true, g_depth);

    stbi_write_png("cnm.png",
                   g_width,
                   g_height,
                   3,
                   pixels.data(),
                   3 * g_width * sizeof(unsigned char));

    return 0;
}