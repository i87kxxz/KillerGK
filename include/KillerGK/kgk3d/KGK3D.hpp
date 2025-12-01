/**
 * @file KGK3D.hpp
 * @brief 3D graphics module for KillerGK
 */

#pragma once

#include "../core/Types.hpp"
#include <string>
#include <memory>
#include <vector>
#include <array>
#include <functional>
#include <unordered_map>

namespace KGK3D {

using KillerGK::Color;

// Forward declarations
class SceneImpl;
class ModelImpl;
class CameraImpl;
class LightImpl;
class MaterialImpl;
class EntityImpl;
class MeshImpl;
class SkeletonImpl;
class BoneImpl;
class PostProcessorImpl;

/**
 * @brief Handle types
 */
using SceneHandle = std::shared_ptr<SceneImpl>;
using ModelHandle = std::shared_ptr<ModelImpl>;
using CameraHandle = std::shared_ptr<CameraImpl>;
using LightHandle = std::shared_ptr<LightImpl>;
using MaterialHandle = std::shared_ptr<MaterialImpl>;
using EntityHandle = std::shared_ptr<EntityImpl>;
using MeshHandle = std::shared_ptr<MeshImpl>;
using SkeletonHandle = std::shared_ptr<SkeletonImpl>;
using BoneHandle = std::shared_ptr<BoneImpl>;
using PostProcessorHandle = std::shared_ptr<PostProcessorImpl>;

/**
 * @struct Vec3
 * @brief 3D vector
 */
struct Vec3 {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    constexpr Vec3() = default;
    constexpr Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

    Vec3 operator+(const Vec3& other) const { return {x + other.x, y + other.y, z + other.z}; }
    Vec3 operator-(const Vec3& other) const { return {x - other.x, y - other.y, z - other.z}; }
    Vec3 operator*(float s) const { return {x * s, y * s, z * s}; }
    Vec3 operator/(float s) const { return {x / s, y / s, z / s}; }
    
    float dot(const Vec3& other) const { return x * other.x + y * other.y + z * other.z; }
    Vec3 cross(const Vec3& other) const {
        return {y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x};
    }
    float length() const;
    Vec3 normalized() const;
    
    bool operator==(const Vec3& other) const = default;
};

/**
 * @struct Vec4
 * @brief 4D vector
 */
struct Vec4 {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float w = 1.0f;

    constexpr Vec4() = default;
    constexpr Vec4(float x, float y, float z, float w = 1.0f) : x(x), y(y), z(z), w(w) {}
    constexpr Vec4(const Vec3& v, float w = 1.0f) : x(v.x), y(v.y), z(v.z), w(w) {}
    
    Vec3 xyz() const { return {x, y, z}; }
    bool operator==(const Vec4& other) const = default;
};

/**
 * @struct Quaternion
 * @brief Quaternion for rotations
 */
struct Quaternion {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float w = 1.0f;

    constexpr Quaternion() = default;
    constexpr Quaternion(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}

    static Quaternion identity() { return {0, 0, 0, 1}; }
    static Quaternion fromEuler(float pitch, float yaw, float roll);
    static Quaternion fromAxisAngle(const Vec3& axis, float angle);
    
    Quaternion operator*(const Quaternion& other) const;
    Vec3 rotate(const Vec3& v) const;
    Quaternion normalized() const;
    Quaternion conjugate() const { return {-x, -y, -z, w}; }
    
    Vec3 toEuler() const;
    bool operator==(const Quaternion& other) const = default;
};

/**
 * @struct Mat4
 * @brief 4x4 matrix
 */
struct Mat4 {
    std::array<float, 16> m = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};

    Mat4() = default;
    
    static Mat4 identity();
    static Mat4 translation(float x, float y, float z);
    static Mat4 translation(const Vec3& v);
    static Mat4 rotation(const Quaternion& q);
    static Mat4 scale(float x, float y, float z);
    static Mat4 scale(const Vec3& v);
    static Mat4 perspective(float fov, float aspect, float near, float far);
    static Mat4 orthographic(float left, float right, float bottom, float top, float near, float far);
    static Mat4 lookAt(const Vec3& eye, const Vec3& target, const Vec3& up);

    Mat4 operator*(const Mat4& other) const;
    Vec4 operator*(const Vec4& v) const;
    Vec3 transformPoint(const Vec3& p) const;
    Vec3 transformDirection(const Vec3& d) const;
    
    Mat4 inverse() const;
    Mat4 transpose() const;
    
    float& operator()(int row, int col) { return m[row * 4 + col]; }
    float operator()(int row, int col) const { return m[row * 4 + col]; }
};

/**
 * @struct Transform
 * @brief 3D transformation (position, rotation, scale)
 */
struct Transform {
    Vec3 position;
    Quaternion rotation;
    Vec3 scale = {1, 1, 1};

    Transform() = default;
    Transform(const Vec3& pos, const Quaternion& rot = Quaternion::identity(), const Vec3& scl = {1,1,1})
        : position(pos), rotation(rot), scale(scl) {}

    Mat4 toMatrix() const;
    static Transform fromMatrix(const Mat4& m);
    
    Transform operator*(const Transform& child) const;
    Vec3 transformPoint(const Vec3& p) const;
    Vec3 transformDirection(const Vec3& d) const;
    
    bool operator==(const Transform& other) const = default;
};

/**
 * @enum LightType
 * @brief Types of lights
 */
enum class LightType {
    Directional,
    Point,
    Spot
};

/**
 * @enum CameraType
 * @brief Types of cameras
 */
enum class CameraType {
    Perspective,
    Orthographic
};

/**
 * @struct Vec2
 * @brief 2D vector for texture coordinates
 */
struct Vec2 {
    float x = 0.0f;
    float y = 0.0f;
    
    constexpr Vec2() = default;
    constexpr Vec2(float x, float y) : x(x), y(y) {}
    
    Vec2 operator+(const Vec2& other) const { return {x + other.x, y + other.y}; }
    Vec2 operator-(const Vec2& other) const { return {x - other.x, y - other.y}; }
    Vec2 operator*(float s) const { return {x * s, y * s}; }
    Vec2 operator/(float s) const { return {x / s, y / s}; }
    
    bool operator==(const Vec2& other) const = default;
};

/**
 * @struct Vertex
 * @brief 3D vertex with all attributes
 */
struct Vertex {
    Vec3 position;
    Vec3 normal;
    Vec2 texCoord;
    Vec3 tangent;
    Vec3 bitangent;
    std::array<int, 4> boneIds = {-1, -1, -1, -1};
    std::array<float, 4> boneWeights = {0, 0, 0, 0};
};

// Forward declarations for builder classes
class Entity;
class Light;
class Camera;
class Material;
class Model;
class Scene;
class PostProcessor;
class Skeleton;
class Bone;


/**
 * @class EntityImpl
 * @brief Implementation of scene graph entity
 */
class EntityImpl : public std::enable_shared_from_this<EntityImpl> {
public:
    EntityImpl() = default;
    ~EntityImpl() = default;

    // Transform
    Transform localTransform;
    Transform worldTransform;
    
    // Hierarchy
    std::weak_ptr<EntityImpl> parent;
    std::vector<EntityHandle> children;
    
    // Components
    MeshHandle mesh;
    MaterialHandle material;
    SkeletonHandle skeleton;
    
    // Metadata
    std::string name;
    bool visible = true;
    
    void updateWorldTransform();
    void addChild(EntityHandle child);
    void removeChild(EntityHandle child);
    EntityHandle findChild(const std::string& name);
};

/**
 * @class MeshImpl
 * @brief 3D mesh data
 */
class MeshImpl {
public:
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    std::string name;
    
    // Bounding box
    Vec3 boundsMin;
    Vec3 boundsMax;
    
    void calculateBounds();
    void calculateNormals();
    void calculateTangents();
};

/**
 * @class BoneImpl
 * @brief Skeleton bone
 */
class BoneImpl {
public:
    std::string name;
    int id = -1;
    Mat4 offsetMatrix;  // Transforms from mesh space to bone space
    Mat4 localTransform;
    Mat4 worldTransform;
    
    std::weak_ptr<BoneImpl> parent;
    std::vector<BoneHandle> children;
    
    void updateWorldTransform();
};

/**
 * @class SkeletonImpl
 * @brief Skeletal animation data
 */
class SkeletonImpl {
public:
    std::vector<BoneHandle> bones;
    std::unordered_map<std::string, int> boneNameToIndex;
    Mat4 globalInverseTransform;
    
    BoneHandle findBone(const std::string& name);
    void updateBoneTransforms();
    std::vector<Mat4> getFinalBoneMatrices() const;
};

/**
 * @struct AnimationKeyframe
 * @brief Keyframe for skeletal animation
 */
struct AnimationKeyframe {
    float time;
    Vec3 position;
    Quaternion rotation;
    Vec3 scale;
};

/**
 * @struct BoneAnimation
 * @brief Animation data for a single bone
 */
struct BoneAnimation {
    std::string boneName;
    std::vector<AnimationKeyframe> keyframes;
    
    AnimationKeyframe interpolate(float time) const;
};

/**
 * @class AnimationClip
 * @brief A complete animation clip
 */
class AnimationClip {
public:
    std::string name;
    float duration = 0.0f;
    float ticksPerSecond = 25.0f;
    std::vector<BoneAnimation> boneAnimations;
    
    void apply(SkeletonHandle skeleton, float time) const;
};

/**
 * @class MaterialImpl
 * @brief PBR material implementation
 */
class MaterialImpl {
public:
    // PBR properties
    Color albedoColor = Color(1.0f, 1.0f, 1.0f, 1.0f);
    float metallic = 0.0f;
    float roughness = 0.5f;
    float ao = 1.0f;
    Color emissiveColor = Color(0.0f, 0.0f, 0.0f, 1.0f);
    float emissiveStrength = 0.0f;
    
    // Texture paths
    std::string albedoMapPath;
    std::string normalMapPath;
    std::string metallicMapPath;
    std::string roughnessMapPath;
    std::string aoMapPath;
    std::string emissiveMapPath;
    
    // Texture IDs (set by renderer)
    uint32_t albedoTexture = 0;
    uint32_t normalTexture = 0;
    uint32_t metallicTexture = 0;
    uint32_t roughnessTexture = 0;
    uint32_t aoTexture = 0;
    uint32_t emissiveTexture = 0;
    
    // Flags
    bool hasAlbedoMap = false;
    bool hasNormalMap = false;
    bool hasMetallicMap = false;
    bool hasRoughnessMap = false;
    bool hasAoMap = false;
    bool hasEmissiveMap = false;
    bool doubleSided = false;
    bool transparent = false;
};

/**
 * @class LightImpl
 * @brief Light source implementation
 */
class LightImpl {
public:
    LightType type = LightType::Directional;
    Color color = Color(1.0f, 1.0f, 1.0f, 1.0f);
    float intensity = 1.0f;
    
    // Position and direction
    Vec3 position;
    Vec3 direction = {0, -1, 0};
    
    // Point/Spot light properties
    float range = 10.0f;
    float innerConeAngle = 30.0f;  // For spot lights
    float outerConeAngle = 45.0f;  // For spot lights
    
    // Shadow properties
    bool castShadow = false;
    int shadowMapSize = 1024;
    float shadowBias = 0.005f;
    float shadowNearPlane = 0.1f;
    float shadowFarPlane = 100.0f;
    
    // Shadow map (set by renderer)
    uint32_t shadowMapTexture = 0;
    Mat4 lightSpaceMatrix;
    
    void calculateLightSpaceMatrix();
};

/**
 * @class CameraImpl
 * @brief Camera implementation
 */
class CameraImpl {
public:
    CameraType type = CameraType::Perspective;
    
    // Perspective properties
    float fov = 60.0f;
    float aspectRatio = 16.0f / 9.0f;
    
    // Orthographic properties
    float orthoSize = 10.0f;
    
    // Common properties
    float nearPlane = 0.1f;
    float farPlane = 1000.0f;
    
    // Transform
    Vec3 position;
    Vec3 target;
    Vec3 up = {0, 1, 0};
    
    // Computed matrices
    Mat4 viewMatrix;
    Mat4 projectionMatrix;
    Mat4 viewProjectionMatrix;
    
    void updateMatrices();
    Vec3 screenToWorld(float x, float y, float depth) const;
    Vec3 worldToScreen(const Vec3& worldPos) const;
};


/**
 * @class PostProcessorImpl
 * @brief Post-processing effects implementation
 */
class PostProcessorImpl {
public:
    // Bloom
    bool bloomEnabled = false;
    float bloomThreshold = 1.0f;
    float bloomIntensity = 1.0f;
    int bloomBlurPasses = 5;
    
    // SSAO
    bool ssaoEnabled = false;
    float ssaoRadius = 0.5f;
    float ssaoBias = 0.025f;
    int ssaoKernelSize = 64;
    
    // Motion blur
    bool motionBlurEnabled = false;
    float motionBlurStrength = 1.0f;
    int motionBlurSamples = 8;
    
    // Color grading
    bool colorGradingEnabled = false;
    float exposure = 1.0f;
    float gamma = 2.2f;
    float contrast = 1.0f;
    float saturation = 1.0f;
    Vec3 colorFilter = {1, 1, 1};
    
    // Tone mapping
    enum class ToneMapping { None, Reinhard, ACES, Filmic };
    ToneMapping toneMapping = ToneMapping::ACES;
    
    // Vignette
    bool vignetteEnabled = false;
    float vignetteIntensity = 0.3f;
    float vignetteSmoothness = 0.5f;
    
    // Depth of field
    bool dofEnabled = false;
    float dofFocusDistance = 10.0f;
    float dofFocusRange = 5.0f;
    float dofBokehSize = 4.0f;
};

/**
 * @class SceneImpl
 * @brief Scene graph implementation
 */
class SceneImpl {
public:
    // Root entity
    EntityHandle root;
    
    // Scene properties
    Color backgroundColor = Color(0.1f, 0.1f, 0.1f, 1.0f);
    Color ambientColor = Color(1.0f, 1.0f, 1.0f, 1.0f);
    float ambientIntensity = 0.1f;
    
    // Environment
    std::string skyboxPath;
    std::string environmentMapPath;
    float environmentIntensity = 1.0f;
    
    // Lights
    std::vector<LightHandle> lights;
    
    // Camera
    CameraHandle activeCamera;
    
    // Post-processing
    PostProcessorHandle postProcessor;
    
    // All entities (for fast lookup)
    std::vector<EntityHandle> allEntities;
    std::unordered_map<std::string, EntityHandle> entityByName;
    
    // Methods
    void addEntity(EntityHandle entity);
    void removeEntity(EntityHandle entity);
    EntityHandle findEntity(const std::string& name);
    void updateTransforms();
    std::vector<EntityHandle> getVisibleEntities(const CameraImpl& camera) const;
};

/**
 * @class ModelImpl
 * @brief Loaded 3D model implementation
 */
class ModelImpl {
public:
    std::string path;
    std::vector<MeshHandle> meshes;
    std::vector<MaterialHandle> materials;
    SkeletonHandle skeleton;
    std::vector<AnimationClip> animations;
    
    // Root entity for the model
    EntityHandle rootEntity;
    
    // Bounding box
    Vec3 boundsMin;
    Vec3 boundsMax;
    
    bool load(const std::string& path);
    void calculateBounds();
};

// ============================================================================
// Builder Classes
// ============================================================================

/**
 * @class Entity
 * @brief Entity builder
 */
class Entity {
public:
    static Entity create();
    
    Entity& name(const std::string& name);
    Entity& position(float x, float y, float z);
    Entity& position(const Vec3& pos);
    Entity& rotation(float pitch, float yaw, float roll);
    Entity& rotation(const Quaternion& rot);
    Entity& scale(float x, float y, float z);
    Entity& scale(const Vec3& scl);
    Entity& scale(float uniform);
    Entity& mesh(MeshHandle mesh);
    Entity& material(MaterialHandle mat);
    Entity& visible(bool vis);
    Entity& addChild(Entity& child);
    
    EntityHandle build();
    
private:
    Entity();
    std::shared_ptr<EntityImpl> m_impl;
};

/**
 * @class Scene
 * @brief Scene builder
 */
class Scene {
public:
    static Scene create();

    Scene& backgroundColor(const Color& color);
    Scene& ambientLight(const Color& color, float intensity);
    Scene& skybox(const std::string& path);
    Scene& environmentMap(const std::string& path, float intensity = 1.0f);

    Scene& add(Entity& entity);
    Scene& add(Light& light);
    Scene& camera(Camera& camera);
    Scene& postProcessing(PostProcessor& pp);

    SceneHandle build();

private:
    Scene();
    std::shared_ptr<SceneImpl> m_impl;
};

/**
 * @class Model
 * @brief Model loader builder
 */
class Model {
public:
    static Model load(const std::string& path);

    Model& position(float x, float y, float z);
    Model& rotation(float pitch, float yaw, float roll);
    Model& scale(float sx, float sy, float sz);
    Model& scale(float uniform);
    Model& material(Material& mat);

    ModelHandle build();
    EntityHandle toEntity();

private:
    Model();
    std::shared_ptr<ModelImpl> m_impl;
    Transform m_transform;
    MaterialHandle m_overrideMaterial;
};

/**
 * @class Camera
 * @brief Camera builder
 */
class Camera {
public:
    static Camera perspective(float fov, float near, float far);
    static Camera orthographic(float size, float near, float far);

    Camera& position(float x, float y, float z);
    Camera& position(const Vec3& pos);
    Camera& lookAt(float x, float y, float z);
    Camera& lookAt(const Vec3& target);
    Camera& up(float x, float y, float z);
    Camera& up(const Vec3& upVec);
    Camera& aspectRatio(float ratio);

    CameraHandle build();

private:
    Camera();
    std::shared_ptr<CameraImpl> m_impl;
};

/**
 * @class Light
 * @brief Light builder
 */
class Light {
public:
    static Light directional(const Color& color, float intensity);
    static Light point(const Color& color, float intensity, float range);
    static Light spot(const Color& color, float intensity, float range, float angle);

    Light& position(float x, float y, float z);
    Light& position(const Vec3& pos);
    Light& direction(float x, float y, float z);
    Light& direction(const Vec3& dir);
    Light& castShadow(bool enabled);
    Light& shadowMapSize(int size);
    Light& shadowBias(float bias);
    Light& innerConeAngle(float angle);  // For spot lights

    LightHandle build();

private:
    Light();
    std::shared_ptr<LightImpl> m_impl;
};

/**
 * @class Material
 * @brief PBR material builder
 */
class Material {
public:
    static Material pbr();

    Material& albedo(const Color& color);
    Material& albedoMap(const std::string& path);
    Material& normalMap(const std::string& path);
    Material& metallicMap(const std::string& path);
    Material& roughnessMap(const std::string& path);
    Material& aoMap(const std::string& path);
    Material& emissiveMap(const std::string& path);
    Material& metallic(float value);
    Material& roughness(float value);
    Material& ao(float value);
    Material& emissive(const Color& color);
    Material& emissiveStrength(float strength);
    Material& doubleSided(bool enabled);
    Material& transparent(bool enabled);

    MaterialHandle build();

private:
    Material();
    std::shared_ptr<MaterialImpl> m_impl;
};

/**
 * @class PostProcessor
 * @brief Post-processing effects builder
 */
class PostProcessor {
public:
    static PostProcessor create();
    
    // Bloom
    PostProcessor& bloom(bool enabled);
    PostProcessor& bloomThreshold(float threshold);
    PostProcessor& bloomIntensity(float intensity);
    PostProcessor& bloomBlurPasses(int passes);
    
    // SSAO
    PostProcessor& ssao(bool enabled);
    PostProcessor& ssaoRadius(float radius);
    PostProcessor& ssaoBias(float bias);
    PostProcessor& ssaoKernelSize(int size);
    
    // Motion blur
    PostProcessor& motionBlur(bool enabled);
    PostProcessor& motionBlurStrength(float strength);
    PostProcessor& motionBlurSamples(int samples);
    
    // Color grading
    PostProcessor& colorGrading(bool enabled);
    PostProcessor& exposure(float exp);
    PostProcessor& gamma(float g);
    PostProcessor& contrast(float c);
    PostProcessor& saturation(float s);
    PostProcessor& colorFilter(const Color& filter);
    PostProcessor& toneMapping(PostProcessorImpl::ToneMapping tm);
    
    // Vignette
    PostProcessor& vignette(bool enabled);
    PostProcessor& vignetteIntensity(float intensity);
    PostProcessor& vignetteSmoothness(float smoothness);
    
    // Depth of field
    PostProcessor& depthOfField(bool enabled);
    PostProcessor& dofFocusDistance(float distance);
    PostProcessor& dofFocusRange(float range);
    PostProcessor& dofBokehSize(float size);
    
    PostProcessorHandle build();

private:
    PostProcessor();
    std::shared_ptr<PostProcessorImpl> m_impl;
};

/**
 * @class Skeleton
 * @brief Skeleton builder for skeletal animation
 */
class Skeleton {
public:
    static Skeleton create();
    
    Skeleton& addBone(Bone& bone);
    Skeleton& globalInverseTransform(const Mat4& mat);
    
    SkeletonHandle build();

private:
    Skeleton();
    std::shared_ptr<SkeletonImpl> m_impl;
};

/**
 * @class Bone
 * @brief Bone builder
 */
class Bone {
public:
    static Bone create(const std::string& name, int id);
    
    Bone& offsetMatrix(const Mat4& mat);
    Bone& localTransform(const Mat4& mat);
    Bone& parent(BoneHandle parentBone);
    
    BoneHandle build();

private:
    Bone();
    std::shared_ptr<BoneImpl> m_impl;
};

// ============================================================================
// Model Loader (Assimp integration)
// ============================================================================

/**
 * @class ModelLoader
 * @brief Loads 3D models using Assimp
 */
class ModelLoader {
public:
    struct LoadOptions {
        bool loadMaterials = true;
        bool loadAnimations = true;
        bool calculateTangents = true;
        bool flipUVs = false;
        bool triangulate = true;
        float scaleFactor = 1.0f;
    };
    
    static ModelHandle loadFromFile(const std::string& path, const LoadOptions& options = {});
    static bool isFormatSupported(const std::string& extension);
    static std::vector<std::string> getSupportedFormats();
};

} // namespace KGK3D

