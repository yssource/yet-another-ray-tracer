#include <yart/core/device.h>
#include <yart/core/scene.h>
#include <yart/geometry/sphere.h>
#include <yart/material/lambertian.h>
#include <yart/material/metal.h>
#include <yart/material/dielectric.h>
#include <yart/texture/constant.h>
#include <yart/texture/checkerboard.h>
#include <yart/texture/image.h>
#include <yart/camera/perspective.h>
#include <array>
#include <chrono>
#include <iostream>
#include <memory>
#include <random>
#include <string>
#include <common.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

constexpr const unsigned g_width = 1200;
constexpr const unsigned g_height = 800;
constexpr const unsigned g_samples = 100;
constexpr const unsigned g_depth = 50;

class Image
{
public:
    Image(const std::string& filename)
    {
        pixels = stbi_load(filename.c_str(), &width, &height, nullptr, 0);
    }
    ~Image() { stbi_image_free(pixels); }

public:
    unsigned char* pixels;
    int width;
    int height;
};

// ray tracing the next week scene
void gen_scene_rttnw(std::vector<std::unique_ptr<yart::Geometry>>& geometries,
                     std::vector<std::unique_ptr<yart::Material>>& materials,
                     std::vector<std::unique_ptr<yart::Texture>>& textures,
                     std::vector<Image>& images,
                     const yart::Device& device)
{
    geometries.push_back(std::make_unique<yart::Sphere>(
        device, 1000.0f, Eigen::Vector3f(0.0f, -1000.0f, 0.0f)));
    textures.push_back(
        std::make_unique<yart::ConstantTexture>(0.2f, 0.3f, 0.1f));
    textures.push_back(
        std::make_unique<yart::ConstantTexture>(0.9f, 0.9f, 0.9f));
    textures.push_back(std::make_unique<yart::CheckerBoardTexture>(
        textures[0].get(), textures[1].get()));
    materials.push_back(
        std::make_unique<yart::Lambertian>(textures.back().get()));

    geometries.push_back(std::make_unique<yart::Sphere>(
        device, 1.0f, Eigen::Vector3f(0.0f, 1.0f, 0.0f)));
    std::string file(YART_EXAMPLE_IMG_DIR);
    file.append("earthmap1k.jpg");
    images.emplace_back(file);
    textures.push_back(std::make_unique<yart::ImageTexture>(
        images[0].pixels, images[0].width, images[0].height));
    materials.push_back(
        std::make_unique<yart::Lambertian>(textures.back().get()));
    materials.push_back(std::make_unique<yart::Dielectric>(1.5f));
}

int main(int argc, char* argv[])
{
    auto device = yart::Device();

    std::vector<std::unique_ptr<yart::Geometry>> geometries;
    std::vector<std::unique_ptr<yart::Material>> materials;
    std::vector<std::unique_ptr<yart::Texture>> textures;
    std::vector<Image> images;
    gen_scene_rttnw(geometries, materials, textures, images, device);

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

    std::vector<unsigned char> pixels(g_width * g_height * 3);
    auto tstart = std::chrono::high_resolution_clock::now();
    auto num_rays = scene.render(
        camera, pixels.data(), g_width, g_height, g_samples, true, g_depth);
    auto tend = std::chrono::high_resolution_clock::now();
    auto tspent =
        std::chrono::duration_cast<std::chrono::milliseconds>(tend - tstart);
    auto mrays = (num_rays / 1000000) / (tspent.count() / 1000);
    std::cout << "Time used(ms): " << tspent.count() << std::endl;
    std::cout << "Mrays/sec: " << mrays << std::endl;

    std::string out_file(YART_EXAMPLE_OUTPUT_DIR);
    out_file.append("rttnw.png");
    stbi_write_png(out_file.c_str(),
                   g_width,
                   g_height,
                   3,
                   pixels.data(),
                   3 * g_width * sizeof(unsigned char));

    return 0;
}
