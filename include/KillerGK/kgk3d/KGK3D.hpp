/**
 * @file KGK3D.hpp
 * @brief 3D graphics module for KillerGK
 */

#pragma once

#include "../core/Types.hpp"
#include <string>
#include <memory>

namespace KGK3D {

using KillerGK::Color;

/**
 * @brief Handle to a scene
 */
using SceneHandle = std::shared_ptr<class SceneImpl>;

/**
 * @brief Handle to a model
 */
using ModelHandle = std::shared_ptr<class ModelImpl>;

/**
 * @brief Handle to a camera
 */
using CameraHandle = std::shared_ptr<class CameraImpl>;

/**
 * @brief Handle to a light
 */
using LightHandle = std::shared_ptr<class LightImpl>;

/**
 * @brief Handle to a material
 */
using MaterialHandle = std::shared_ptr<class MaterialImpl>;

// Forward declarations
class Entity;
class Light;
class Camera;
class Material;

/**
 * @class Scene
 * @brief 3D scene container
 */
class Scene {
public:
    static Scene create();

    Scene& backgroundColor(const Color& color);
    Scene& ambientLight(const Color& color, float intensity);

    Scene& add(Entity& entity);
    Scene& add(Light& light);
    Scene& camera(Camera& camera);

    SceneHandle build();

private:
    Scene();
    struct Impl;
    std::shared_ptr<Impl> m_impl;
};

/**
 * @class Model
 * @brief 3D model loader
 */
class Model {
public:
    static Model load(const std::string& path);

    Model& position(float x, float y, float z);
    Model& rotation(float pitch, float yaw, float roll);
    Model& scale(float sx, float sy, float sz);
    Model& material(Material& mat);

    ModelHandle build();

private:
    Model();
    struct Impl;
    std::shared_ptr<Impl> m_impl;
};

/**
 * @class Camera
 * @brief 3D camera
 */
class Camera {
public:
    static Camera perspective(float fov, float near, float far);
    static Camera orthographic(float size, float near, float far);

    Camera& position(float x, float y, float z);
    Camera& lookAt(float x, float y, float z);

    CameraHandle build();

private:
    Camera();
    struct Impl;
    std::shared_ptr<Impl> m_impl;
};

/**
 * @class Light
 * @brief 3D light source
 */
class Light {
public:
    static Light directional(const Color& color, float intensity);
    static Light point(const Color& color, float intensity, float range);
    static Light spot(const Color& color, float intensity, float range, float angle);

    Light& position(float x, float y, float z);
    Light& direction(float x, float y, float z);
    Light& castShadow(bool enabled);

    LightHandle build();

private:
    Light();
    struct Impl;
    std::shared_ptr<Impl> m_impl;
};

/**
 * @class Material
 * @brief PBR material
 */
class Material {
public:
    static Material pbr();

    Material& albedo(const Color& color);
    Material& albedoMap(const std::string& path);
    Material& normalMap(const std::string& path);
    Material& metallicMap(const std::string& path);
    Material& roughnessMap(const std::string& path);
    Material& metallic(float value);
    Material& roughness(float value);
    Material& emissive(const Color& color);

    MaterialHandle build();

private:
    Material();
    struct Impl;
    std::shared_ptr<Impl> m_impl;
};

} // namespace KGK3D
