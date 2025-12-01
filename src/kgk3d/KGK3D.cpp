/**
 * @file KGK3D.cpp
 * @brief 3D graphics module implementation
 */

#include "KillerGK/kgk3d/KGK3D.hpp"
#include <cmath>
#include <algorithm>
#include <fstream>
#include <sstream>

namespace KGK3D {

// ============================================================================
// Math Implementations
// ============================================================================

float Vec3::length() const {
    return std::sqrt(x * x + y * y + z * z);
}

Vec3 Vec3::normalized() const {
    float len = length();
    if (len > 0.0001f) {
        return *this / len;
    }
    return {0, 0, 0};
}

Quaternion Quaternion::fromEuler(float pitch, float yaw, float roll) {
    // Convert degrees to radians
    float p = pitch * 0.5f * 3.14159265f / 180.0f;
    float y = yaw * 0.5f * 3.14159265f / 180.0f;
    float r = roll * 0.5f * 3.14159265f / 180.0f;
    
    float cp = std::cos(p), sp = std::sin(p);
    float cy = std::cos(y), sy = std::sin(y);
    float cr = std::cos(r), sr = std::sin(r);
    
    Quaternion q;
    q.w = cr * cp * cy + sr * sp * sy;
    q.x = sr * cp * cy - cr * sp * sy;
    q.y = cr * sp * cy + sr * cp * sy;
    q.z = cr * cp * sy - sr * sp * cy;
    return q;
}

Quaternion Quaternion::fromAxisAngle(const Vec3& axis, float angle) {
    float rad = angle * 0.5f * 3.14159265f / 180.0f;
    float s = std::sin(rad);
    Vec3 n = axis.normalized();
    return {n.x * s, n.y * s, n.z * s, std::cos(rad)};
}

Quaternion Quaternion::operator*(const Quaternion& other) const {
    return {
        w * other.x + x * other.w + y * other.z - z * other.y,
        w * other.y - x * other.z + y * other.w + z * other.x,
        w * other.z + x * other.y - y * other.x + z * other.w,
        w * other.w - x * other.x - y * other.y - z * other.z
    };
}

Vec3 Quaternion::rotate(const Vec3& v) const {
    Vec3 qv{x, y, z};
    Vec3 uv = qv.cross(v);
    Vec3 uuv = qv.cross(uv);
    return v + (uv * w + uuv) * 2.0f;
}

Quaternion Quaternion::normalized() const {
    float len = std::sqrt(x*x + y*y + z*z + w*w);
    if (len > 0.0001f) {
        return {x/len, y/len, z/len, w/len};
    }
    return identity();
}

Vec3 Quaternion::toEuler() const {
    Vec3 euler;
    
    // Roll (x-axis rotation)
    float sinr_cosp = 2.0f * (w * x + y * z);
    float cosr_cosp = 1.0f - 2.0f * (x * x + y * y);
    euler.z = std::atan2(sinr_cosp, cosr_cosp) * 180.0f / 3.14159265f;
    
    // Pitch (y-axis rotation)
    float sinp = 2.0f * (w * y - z * x);
    if (std::abs(sinp) >= 1.0f) {
        euler.x = std::copysign(90.0f, sinp);
    } else {
        euler.x = std::asin(sinp) * 180.0f / 3.14159265f;
    }
    
    // Yaw (z-axis rotation)
    float siny_cosp = 2.0f * (w * z + x * y);
    float cosy_cosp = 1.0f - 2.0f * (y * y + z * z);
    euler.y = std::atan2(siny_cosp, cosy_cosp) * 180.0f / 3.14159265f;
    
    return euler;
}

Mat4 Mat4::identity() {
    Mat4 m;
    m.m = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
    return m;
}

Mat4 Mat4::translation(float x, float y, float z) {
    Mat4 m = identity();
    m(0, 3) = x;
    m(1, 3) = y;
    m(2, 3) = z;
    return m;
}

Mat4 Mat4::translation(const Vec3& v) {
    return translation(v.x, v.y, v.z);
}

Mat4 Mat4::rotation(const Quaternion& q) {
    Mat4 m = identity();
    
    float xx = q.x * q.x, yy = q.y * q.y, zz = q.z * q.z;
    float xy = q.x * q.y, xz = q.x * q.z, yz = q.y * q.z;
    float wx = q.w * q.x, wy = q.w * q.y, wz = q.w * q.z;
    
    m(0, 0) = 1.0f - 2.0f * (yy + zz);
    m(0, 1) = 2.0f * (xy - wz);
    m(0, 2) = 2.0f * (xz + wy);
    
    m(1, 0) = 2.0f * (xy + wz);
    m(1, 1) = 1.0f - 2.0f * (xx + zz);
    m(1, 2) = 2.0f * (yz - wx);
    
    m(2, 0) = 2.0f * (xz - wy);
    m(2, 1) = 2.0f * (yz + wx);
    m(2, 2) = 1.0f - 2.0f * (xx + yy);
    
    return m;
}

Mat4 Mat4::scale(float x, float y, float z) {
    Mat4 m = identity();
    m(0, 0) = x;
    m(1, 1) = y;
    m(2, 2) = z;
    return m;
}

Mat4 Mat4::scale(const Vec3& v) {
    return scale(v.x, v.y, v.z);
}

Mat4 Mat4::perspective(float fov, float aspect, float near, float far) {
    Mat4 m;
    m.m.fill(0);
    
    float tanHalfFov = std::tan(fov * 0.5f * 3.14159265f / 180.0f);
    
    m(0, 0) = 1.0f / (aspect * tanHalfFov);
    m(1, 1) = 1.0f / tanHalfFov;
    m(2, 2) = -(far + near) / (far - near);
    m(2, 3) = -(2.0f * far * near) / (far - near);
    m(3, 2) = -1.0f;
    
    return m;
}

Mat4 Mat4::orthographic(float left, float right, float bottom, float top, float near, float far) {
    Mat4 m = identity();
    
    m(0, 0) = 2.0f / (right - left);
    m(1, 1) = 2.0f / (top - bottom);
    m(2, 2) = -2.0f / (far - near);
    m(0, 3) = -(right + left) / (right - left);
    m(1, 3) = -(top + bottom) / (top - bottom);
    m(2, 3) = -(far + near) / (far - near);
    
    return m;
}

Mat4 Mat4::lookAt(const Vec3& eye, const Vec3& target, const Vec3& up) {
    Vec3 f = (target - eye).normalized();
    Vec3 r = f.cross(up).normalized();
    Vec3 u = r.cross(f);
    
    Mat4 m = identity();
    m(0, 0) = r.x;  m(0, 1) = r.y;  m(0, 2) = r.z;  m(0, 3) = -r.dot(eye);
    m(1, 0) = u.x;  m(1, 1) = u.y;  m(1, 2) = u.z;  m(1, 3) = -u.dot(eye);
    m(2, 0) = -f.x; m(2, 1) = -f.y; m(2, 2) = -f.z; m(2, 3) = f.dot(eye);
    
    return m;
}

Mat4 Mat4::operator*(const Mat4& other) const {
    Mat4 result;
    result.m.fill(0);
    
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            for (int k = 0; k < 4; ++k) {
                result(i, j) += (*this)(i, k) * other(k, j);
            }
        }
    }
    return result;
}

Vec4 Mat4::operator*(const Vec4& v) const {
    return {
        m[0]*v.x + m[1]*v.y + m[2]*v.z + m[3]*v.w,
        m[4]*v.x + m[5]*v.y + m[6]*v.z + m[7]*v.w,
        m[8]*v.x + m[9]*v.y + m[10]*v.z + m[11]*v.w,
        m[12]*v.x + m[13]*v.y + m[14]*v.z + m[15]*v.w
    };
}

Vec3 Mat4::transformPoint(const Vec3& p) const {
    Vec4 result = (*this) * Vec4(p, 1.0f);
    if (std::abs(result.w) > 0.0001f) {
        return {result.x / result.w, result.y / result.w, result.z / result.w};
    }
    return result.xyz();
}

Vec3 Mat4::transformDirection(const Vec3& d) const {
    return ((*this) * Vec4(d, 0.0f)).xyz();
}

Mat4 Mat4::transpose() const {
    Mat4 result;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            result(i, j) = (*this)(j, i);
        }
    }
    return result;
}

Mat4 Mat4::inverse() const {
    Mat4 inv;
    float det;
    
    inv.m[0] = m[5]*m[10]*m[15] - m[5]*m[11]*m[14] - m[9]*m[6]*m[15] + m[9]*m[7]*m[14] + m[13]*m[6]*m[11] - m[13]*m[7]*m[10];
    inv.m[4] = -m[4]*m[10]*m[15] + m[4]*m[11]*m[14] + m[8]*m[6]*m[15] - m[8]*m[7]*m[14] - m[12]*m[6]*m[11] + m[12]*m[7]*m[10];
    inv.m[8] = m[4]*m[9]*m[15] - m[4]*m[11]*m[13] - m[8]*m[5]*m[15] + m[8]*m[7]*m[13] + m[12]*m[5]*m[11] - m[12]*m[7]*m[9];
    inv.m[12] = -m[4]*m[9]*m[14] + m[4]*m[10]*m[13] + m[8]*m[5]*m[14] - m[8]*m[6]*m[13] - m[12]*m[5]*m[10] + m[12]*m[6]*m[9];
    inv.m[1] = -m[1]*m[10]*m[15] + m[1]*m[11]*m[14] + m[9]*m[2]*m[15] - m[9]*m[3]*m[14] - m[13]*m[2]*m[11] + m[13]*m[3]*m[10];
    inv.m[5] = m[0]*m[10]*m[15] - m[0]*m[11]*m[14] - m[8]*m[2]*m[15] + m[8]*m[3]*m[14] + m[12]*m[2]*m[11] - m[12]*m[3]*m[10];
    inv.m[9] = -m[0]*m[9]*m[15] + m[0]*m[11]*m[13] + m[8]*m[1]*m[15] - m[8]*m[3]*m[13] - m[12]*m[1]*m[11] + m[12]*m[3]*m[9];
    inv.m[13] = m[0]*m[9]*m[14] - m[0]*m[10]*m[13] - m[8]*m[1]*m[14] + m[8]*m[2]*m[13] + m[12]*m[1]*m[10] - m[12]*m[2]*m[9];
    inv.m[2] = m[1]*m[6]*m[15] - m[1]*m[7]*m[14] - m[5]*m[2]*m[15] + m[5]*m[3]*m[14] + m[13]*m[2]*m[7] - m[13]*m[3]*m[6];
    inv.m[6] = -m[0]*m[6]*m[15] + m[0]*m[7]*m[14] + m[4]*m[2]*m[15] - m[4]*m[3]*m[14] - m[12]*m[2]*m[7] + m[12]*m[3]*m[6];
    inv.m[10] = m[0]*m[5]*m[15] - m[0]*m[7]*m[13] - m[4]*m[1]*m[15] + m[4]*m[3]*m[13] + m[12]*m[1]*m[7] - m[12]*m[3]*m[5];
    inv.m[14] = -m[0]*m[5]*m[14] + m[0]*m[6]*m[13] + m[4]*m[1]*m[14] - m[4]*m[2]*m[13] - m[12]*m[1]*m[6] + m[12]*m[2]*m[5];
    inv.m[3] = -m[1]*m[6]*m[11] + m[1]*m[7]*m[10] + m[5]*m[2]*m[11] - m[5]*m[3]*m[10] - m[9]*m[2]*m[7] + m[9]*m[3]*m[6];
    inv.m[7] = m[0]*m[6]*m[11] - m[0]*m[7]*m[10] - m[4]*m[2]*m[11] + m[4]*m[3]*m[10] + m[8]*m[2]*m[7] - m[8]*m[3]*m[6];
    inv.m[11] = -m[0]*m[5]*m[11] + m[0]*m[7]*m[9] + m[4]*m[1]*m[11] - m[4]*m[3]*m[9] - m[8]*m[1]*m[7] + m[8]*m[3]*m[5];
    inv.m[15] = m[0]*m[5]*m[10] - m[0]*m[6]*m[9] - m[4]*m[1]*m[10] + m[4]*m[2]*m[9] + m[8]*m[1]*m[6] - m[8]*m[2]*m[5];
    
    det = m[0]*inv.m[0] + m[1]*inv.m[4] + m[2]*inv.m[8] + m[3]*inv.m[12];
    
    if (std::abs(det) < 0.0001f) {
        return identity();
    }
    
    det = 1.0f / det;
    for (int i = 0; i < 16; ++i) {
        inv.m[i] *= det;
    }
    
    return inv;
}

Mat4 Transform::toMatrix() const {
    return Mat4::translation(position) * Mat4::rotation(rotation) * Mat4::scale(scale);
}

Transform Transform::fromMatrix(const Mat4& m) {
    // Simplified - full decomposition is complex
    Transform t;
    t.position = {m.m[3], m.m[7], m.m[11]};
    t.scale = {1, 1, 1};
    t.rotation = Quaternion::identity();
    return t;
}

Transform Transform::operator*(const Transform& child) const {
    Transform result;
    result.scale = {scale.x * child.scale.x, scale.y * child.scale.y, scale.z * child.scale.z};
    result.rotation = rotation * child.rotation;
    result.position = position + rotation.rotate(Vec3{
        scale.x * child.position.x,
        scale.y * child.position.y,
        scale.z * child.position.z
    });
    return result;
}

Vec3 Transform::transformPoint(const Vec3& p) const {
    return position + rotation.rotate(Vec3{p.x * scale.x, p.y * scale.y, p.z * scale.z});
}

Vec3 Transform::transformDirection(const Vec3& d) const {
    return rotation.rotate(d);
}


// ============================================================================
// Entity Implementation
// ============================================================================

void EntityImpl::updateWorldTransform() {
    if (auto p = parent.lock()) {
        worldTransform = p->worldTransform * localTransform;
    } else {
        worldTransform = localTransform;
    }
    
    // Recursively update children
    for (auto& child : children) {
        if (child) {
            child->updateWorldTransform();
        }
    }
}

void EntityImpl::addChild(EntityHandle child) {
    if (!child) return;
    
    // Remove from previous parent
    if (auto oldParent = child->parent.lock()) {
        oldParent->removeChild(child);
    }
    
    child->parent = shared_from_this();
    children.push_back(child);
    child->updateWorldTransform();
}

void EntityImpl::removeChild(EntityHandle child) {
    if (!child) return;
    
    auto it = std::find(children.begin(), children.end(), child);
    if (it != children.end()) {
        (*it)->parent.reset();
        children.erase(it);
    }
}

EntityHandle EntityImpl::findChild(const std::string& searchName) {
    for (auto& child : children) {
        if (child && child->name == searchName) {
            return child;
        }
        // Recursive search
        if (child) {
            auto found = child->findChild(searchName);
            if (found) return found;
        }
    }
    return nullptr;
}

// ============================================================================
// Mesh Implementation
// ============================================================================

void MeshImpl::calculateBounds() {
    if (vertices.empty()) {
        boundsMin = boundsMax = Vec3{0, 0, 0};
        return;
    }
    
    boundsMin = boundsMax = vertices[0].position;
    
    for (const auto& v : vertices) {
        boundsMin.x = std::min(boundsMin.x, v.position.x);
        boundsMin.y = std::min(boundsMin.y, v.position.y);
        boundsMin.z = std::min(boundsMin.z, v.position.z);
        boundsMax.x = std::max(boundsMax.x, v.position.x);
        boundsMax.y = std::max(boundsMax.y, v.position.y);
        boundsMax.z = std::max(boundsMax.z, v.position.z);
    }
}

void MeshImpl::calculateNormals() {
    // Reset all normals
    for (auto& v : vertices) {
        v.normal = Vec3{0, 0, 0};
    }
    
    // Calculate face normals and accumulate
    for (size_t i = 0; i + 2 < indices.size(); i += 3) {
        uint32_t i0 = indices[i];
        uint32_t i1 = indices[i + 1];
        uint32_t i2 = indices[i + 2];
        
        if (i0 >= vertices.size() || i1 >= vertices.size() || i2 >= vertices.size()) {
            continue;
        }
        
        Vec3 v0 = vertices[i0].position;
        Vec3 v1 = vertices[i1].position;
        Vec3 v2 = vertices[i2].position;
        
        Vec3 edge1 = v1 - v0;
        Vec3 edge2 = v2 - v0;
        Vec3 faceNormal = edge1.cross(edge2);
        
        // Accumulate face normal to vertex normals (area-weighted)
        vertices[i0].normal = vertices[i0].normal + faceNormal;
        vertices[i1].normal = vertices[i1].normal + faceNormal;
        vertices[i2].normal = vertices[i2].normal + faceNormal;
    }
    
    // Normalize all vertex normals
    for (auto& v : vertices) {
        v.normal = v.normal.normalized();
    }
}

void MeshImpl::calculateTangents() {
    // Reset tangents and bitangents
    for (auto& v : vertices) {
        v.tangent = Vec3{0, 0, 0};
        v.bitangent = Vec3{0, 0, 0};
    }
    
    // Calculate tangents per triangle
    for (size_t i = 0; i + 2 < indices.size(); i += 3) {
        uint32_t i0 = indices[i];
        uint32_t i1 = indices[i + 1];
        uint32_t i2 = indices[i + 2];
        
        if (i0 >= vertices.size() || i1 >= vertices.size() || i2 >= vertices.size()) {
            continue;
        }
        
        Vertex& v0 = vertices[i0];
        Vertex& v1 = vertices[i1];
        Vertex& v2 = vertices[i2];
        
        Vec3 edge1 = v1.position - v0.position;
        Vec3 edge2 = v2.position - v0.position;
        
        float deltaU1 = v1.texCoord.x - v0.texCoord.x;
        float deltaV1 = v1.texCoord.y - v0.texCoord.y;
        float deltaU2 = v2.texCoord.x - v0.texCoord.x;
        float deltaV2 = v2.texCoord.y - v0.texCoord.y;
        
        float f = deltaU1 * deltaV2 - deltaU2 * deltaV1;
        if (std::abs(f) < 0.0001f) {
            f = 1.0f;
        } else {
            f = 1.0f / f;
        }
        
        Vec3 tangent{
            f * (deltaV2 * edge1.x - deltaV1 * edge2.x),
            f * (deltaV2 * edge1.y - deltaV1 * edge2.y),
            f * (deltaV2 * edge1.z - deltaV1 * edge2.z)
        };
        
        Vec3 bitangent{
            f * (-deltaU2 * edge1.x + deltaU1 * edge2.x),
            f * (-deltaU2 * edge1.y + deltaU1 * edge2.y),
            f * (-deltaU2 * edge1.z + deltaU1 * edge2.z)
        };
        
        v0.tangent = v0.tangent + tangent;
        v1.tangent = v1.tangent + tangent;
        v2.tangent = v2.tangent + tangent;
        
        v0.bitangent = v0.bitangent + bitangent;
        v1.bitangent = v1.bitangent + bitangent;
        v2.bitangent = v2.bitangent + bitangent;
    }
    
    // Orthonormalize tangent space using Gram-Schmidt
    for (auto& v : vertices) {
        Vec3 n = v.normal;
        Vec3 t = v.tangent;
        
        // Gram-Schmidt orthogonalize
        t = (t - n * n.dot(t)).normalized();
        v.tangent = t;
        
        // Calculate handedness
        float handedness = (n.cross(t).dot(v.bitangent) < 0.0f) ? -1.0f : 1.0f;
        v.bitangent = n.cross(t) * handedness;
    }
}

// ============================================================================
// Bone Implementation
// ============================================================================

void BoneImpl::updateWorldTransform() {
    if (auto p = parent.lock()) {
        worldTransform = p->worldTransform * localTransform;
    } else {
        worldTransform = localTransform;
    }
    
    for (auto& child : children) {
        if (child) {
            child->updateWorldTransform();
        }
    }
}

// ============================================================================
// Skeleton Implementation
// ============================================================================

BoneHandle SkeletonImpl::findBone(const std::string& name) {
    auto it = boneNameToIndex.find(name);
    if (it != boneNameToIndex.end() && it->second >= 0 && 
        static_cast<size_t>(it->second) < bones.size()) {
        return bones[it->second];
    }
    return nullptr;
}

void SkeletonImpl::updateBoneTransforms() {
    // Update from root bones (bones without parents)
    for (auto& bone : bones) {
        if (bone && bone->parent.expired()) {
            bone->updateWorldTransform();
        }
    }
}

std::vector<Mat4> SkeletonImpl::getFinalBoneMatrices() const {
    std::vector<Mat4> matrices(bones.size());
    
    for (size_t i = 0; i < bones.size(); ++i) {
        if (bones[i]) {
            // Final matrix = GlobalInverse * BoneWorld * BoneOffset
            matrices[i] = globalInverseTransform * bones[i]->worldTransform * bones[i]->offsetMatrix;
        } else {
            matrices[i] = Mat4::identity();
        }
    }
    
    return matrices;
}

// ============================================================================
// Animation Implementation
// ============================================================================

AnimationKeyframe BoneAnimation::interpolate(float time) const {
    if (keyframes.empty()) {
        return AnimationKeyframe{0, Vec3{0,0,0}, Quaternion::identity(), Vec3{1,1,1}};
    }
    
    if (keyframes.size() == 1 || time <= keyframes.front().time) {
        return keyframes.front();
    }
    
    if (time >= keyframes.back().time) {
        return keyframes.back();
    }
    
    // Find the two keyframes to interpolate between
    size_t nextIndex = 0;
    for (size_t i = 0; i < keyframes.size(); ++i) {
        if (keyframes[i].time > time) {
            nextIndex = i;
            break;
        }
    }
    
    size_t prevIndex = nextIndex > 0 ? nextIndex - 1 : 0;
    
    const auto& prev = keyframes[prevIndex];
    const auto& next = keyframes[nextIndex];
    
    float deltaTime = next.time - prev.time;
    float factor = (deltaTime > 0.0001f) ? (time - prev.time) / deltaTime : 0.0f;
    factor = std::clamp(factor, 0.0f, 1.0f);
    
    AnimationKeyframe result;
    result.time = time;
    
    // Linear interpolation for position and scale
    result.position = prev.position + (next.position - prev.position) * factor;
    result.scale = prev.scale + (next.scale - prev.scale) * factor;
    
    // Spherical linear interpolation (SLERP) for rotation
    float dot = prev.rotation.x * next.rotation.x + 
                prev.rotation.y * next.rotation.y + 
                prev.rotation.z * next.rotation.z + 
                prev.rotation.w * next.rotation.w;
    
    Quaternion q2 = next.rotation;
    if (dot < 0.0f) {
        q2 = Quaternion{-q2.x, -q2.y, -q2.z, -q2.w};
        dot = -dot;
    }
    
    if (dot > 0.9995f) {
        // Linear interpolation for very close quaternions
        result.rotation = Quaternion{
            prev.rotation.x + factor * (q2.x - prev.rotation.x),
            prev.rotation.y + factor * (q2.y - prev.rotation.y),
            prev.rotation.z + factor * (q2.z - prev.rotation.z),
            prev.rotation.w + factor * (q2.w - prev.rotation.w)
        }.normalized();
    } else {
        float theta = std::acos(dot);
        float sinTheta = std::sin(theta);
        float w1 = std::sin((1.0f - factor) * theta) / sinTheta;
        float w2 = std::sin(factor * theta) / sinTheta;
        
        result.rotation = Quaternion{
            w1 * prev.rotation.x + w2 * q2.x,
            w1 * prev.rotation.y + w2 * q2.y,
            w1 * prev.rotation.z + w2 * q2.z,
            w1 * prev.rotation.w + w2 * q2.w
        };
    }
    
    return result;
}

void AnimationClip::apply(SkeletonHandle skeleton, float time) const {
    if (!skeleton) return;
    
    // Normalize time to animation duration
    float normalizedTime = time;
    if (duration > 0.0f) {
        normalizedTime = std::fmod(time, duration);
        if (normalizedTime < 0.0f) {
            normalizedTime += duration;
        }
    }
    
    // Convert to ticks
    float tickTime = normalizedTime * ticksPerSecond;
    
    // Apply animation to each bone
    for (const auto& boneAnim : boneAnimations) {
        auto bone = skeleton->findBone(boneAnim.boneName);
        if (!bone) continue;
        
        AnimationKeyframe kf = boneAnim.interpolate(tickTime);
        
        // Build local transform from keyframe
        bone->localTransform = Mat4::translation(kf.position) * 
                               Mat4::rotation(kf.rotation) * 
                               Mat4::scale(kf.scale);
    }
    
    // Update all bone world transforms
    skeleton->updateBoneTransforms();
}

// ============================================================================
// Light Implementation
// ============================================================================

void LightImpl::calculateLightSpaceMatrix() {
    switch (type) {
        case LightType::Directional: {
            // Orthographic projection for directional light
            Vec3 lightDir = direction.normalized();
            Vec3 up = (std::abs(lightDir.y) > 0.99f) ? Vec3{1, 0, 0} : Vec3{0, 1, 0};
            
            // Create view matrix looking in light direction
            Vec3 lightPos = lightDir * -50.0f; // Position light far away
            Mat4 lightView = Mat4::lookAt(lightPos, Vec3{0, 0, 0}, up);
            
            // Orthographic projection covering the scene
            float orthoSize = shadowFarPlane * 0.5f;
            Mat4 lightProj = Mat4::orthographic(-orthoSize, orthoSize, -orthoSize, orthoSize, 
                                                 shadowNearPlane, shadowFarPlane);
            
            lightSpaceMatrix = lightProj * lightView;
            break;
        }
        
        case LightType::Point: {
            // Point lights need 6 matrices for cubemap shadow
            // For simplicity, we'll use a single matrix looking down -Z
            Mat4 lightView = Mat4::lookAt(position, position + Vec3{0, 0, -1}, Vec3{0, 1, 0});
            Mat4 lightProj = Mat4::perspective(90.0f, 1.0f, shadowNearPlane, range);
            lightSpaceMatrix = lightProj * lightView;
            break;
        }
        
        case LightType::Spot: {
            // Perspective projection for spot light
            Vec3 lightDir = direction.normalized();
            Vec3 up = (std::abs(lightDir.y) > 0.99f) ? Vec3{1, 0, 0} : Vec3{0, 1, 0};
            
            Mat4 lightView = Mat4::lookAt(position, position + lightDir, up);
            Mat4 lightProj = Mat4::perspective(outerConeAngle * 2.0f, 1.0f, shadowNearPlane, range);
            
            lightSpaceMatrix = lightProj * lightView;
            break;
        }
    }
}

// ============================================================================
// Camera Implementation
// ============================================================================

void CameraImpl::updateMatrices() {
    // View matrix
    viewMatrix = Mat4::lookAt(position, target, up);
    
    // Projection matrix
    if (type == CameraType::Perspective) {
        projectionMatrix = Mat4::perspective(fov, aspectRatio, nearPlane, farPlane);
    } else {
        float halfHeight = orthoSize * 0.5f;
        float halfWidth = halfHeight * aspectRatio;
        projectionMatrix = Mat4::orthographic(-halfWidth, halfWidth, -halfHeight, halfHeight, 
                                               nearPlane, farPlane);
    }
    
    viewProjectionMatrix = projectionMatrix * viewMatrix;
}

Vec3 CameraImpl::screenToWorld(float x, float y, float depth) const {
    // Convert screen coordinates to NDC (-1 to 1)
    Vec4 ndc{x * 2.0f - 1.0f, 1.0f - y * 2.0f, depth * 2.0f - 1.0f, 1.0f};
    
    // Inverse view-projection
    Mat4 invVP = viewProjectionMatrix.inverse();
    Vec4 world = invVP * ndc;
    
    if (std::abs(world.w) > 0.0001f) {
        return Vec3{world.x / world.w, world.y / world.w, world.z / world.w};
    }
    return world.xyz();
}

Vec3 CameraImpl::worldToScreen(const Vec3& worldPos) const {
    Vec4 clip = viewProjectionMatrix * Vec4(worldPos, 1.0f);
    
    if (std::abs(clip.w) > 0.0001f) {
        Vec3 ndc{clip.x / clip.w, clip.y / clip.w, clip.z / clip.w};
        return Vec3{
            (ndc.x + 1.0f) * 0.5f,
            (1.0f - ndc.y) * 0.5f,
            (ndc.z + 1.0f) * 0.5f
        };
    }
    return Vec3{0, 0, 0};
}


// ============================================================================
// Scene Implementation
// ============================================================================

void SceneImpl::addEntity(EntityHandle entity) {
    if (!entity) return;
    
    allEntities.push_back(entity);
    
    if (!entity->name.empty()) {
        entityByName[entity->name] = entity;
    }
    
    // If no parent, add to root
    if (entity->parent.expired()) {
        if (!root) {
            root = std::make_shared<EntityImpl>();
            root->name = "__root__";
        }
        root->addChild(entity);
    }
}

void SceneImpl::removeEntity(EntityHandle entity) {
    if (!entity) return;
    
    // Remove from name map
    if (!entity->name.empty()) {
        entityByName.erase(entity->name);
    }
    
    // Remove from all entities list
    auto it = std::find(allEntities.begin(), allEntities.end(), entity);
    if (it != allEntities.end()) {
        allEntities.erase(it);
    }
    
    // Remove from parent
    if (auto parent = entity->parent.lock()) {
        parent->removeChild(entity);
    }
    
    // Recursively remove children
    for (auto& child : entity->children) {
        removeEntity(child);
    }
}

EntityHandle SceneImpl::findEntity(const std::string& name) {
    auto it = entityByName.find(name);
    if (it != entityByName.end()) {
        return it->second;
    }
    return nullptr;
}

void SceneImpl::updateTransforms() {
    if (root) {
        root->updateWorldTransform();
    }
}

std::vector<EntityHandle> SceneImpl::getVisibleEntities(const CameraImpl& camera) const {
    std::vector<EntityHandle> visible;
    visible.reserve(allEntities.size());
    
    // Simple frustum culling using bounding spheres
    for (const auto& entity : allEntities) {
        if (!entity || !entity->visible) continue;
        
        // Check if entity has a mesh with bounds
        if (entity->mesh) {
            Vec3 center = (entity->mesh->boundsMin + entity->mesh->boundsMax) * 0.5f;
            Vec3 extents = entity->mesh->boundsMax - entity->mesh->boundsMin;
            float radius = extents.length() * 0.5f;
            
            // Transform center to world space
            Vec3 worldCenter = entity->worldTransform.transformPoint(center);
            
            // Transform to clip space
            Vec4 clipPos = camera.viewProjectionMatrix * Vec4(worldCenter, 1.0f);
            
            // Simple frustum check (sphere vs frustum)
            float w = clipPos.w + radius;
            if (clipPos.x < -w || clipPos.x > w ||
                clipPos.y < -w || clipPos.y > w ||
                clipPos.z < -w || clipPos.z > w) {
                continue; // Outside frustum
            }
        }
        
        visible.push_back(entity);
    }
    
    return visible;
}

// ============================================================================
// Model Implementation
// ============================================================================

bool ModelImpl::load(const std::string& filePath) {
    path = filePath;
    
    // Get file extension
    size_t dotPos = filePath.rfind('.');
    if (dotPos == std::string::npos) {
        return false;
    }
    
    std::string ext = filePath.substr(dotPos + 1);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    
    // Check supported formats
    if (ext != "obj" && ext != "fbx" && ext != "gltf" && ext != "glb") {
        return false;
    }
    
    // Note: Actual loading would use Assimp library
    // This is a placeholder that creates a simple cube mesh for testing
    
    auto mesh = std::make_shared<MeshImpl>();
    mesh->name = "default_mesh";
    
    // Create a simple cube for testing
    float s = 0.5f;
    mesh->vertices = {
        // Front face
        {{-s, -s,  s}, {0, 0, 1}, {0, 0}, {1, 0, 0}, {0, 1, 0}, {-1,-1,-1,-1}, {0,0,0,0}},
        {{ s, -s,  s}, {0, 0, 1}, {1, 0}, {1, 0, 0}, {0, 1, 0}, {-1,-1,-1,-1}, {0,0,0,0}},
        {{ s,  s,  s}, {0, 0, 1}, {1, 1}, {1, 0, 0}, {0, 1, 0}, {-1,-1,-1,-1}, {0,0,0,0}},
        {{-s,  s,  s}, {0, 0, 1}, {0, 1}, {1, 0, 0}, {0, 1, 0}, {-1,-1,-1,-1}, {0,0,0,0}},
        // Back face
        {{ s, -s, -s}, {0, 0, -1}, {0, 0}, {-1, 0, 0}, {0, 1, 0}, {-1,-1,-1,-1}, {0,0,0,0}},
        {{-s, -s, -s}, {0, 0, -1}, {1, 0}, {-1, 0, 0}, {0, 1, 0}, {-1,-1,-1,-1}, {0,0,0,0}},
        {{-s,  s, -s}, {0, 0, -1}, {1, 1}, {-1, 0, 0}, {0, 1, 0}, {-1,-1,-1,-1}, {0,0,0,0}},
        {{ s,  s, -s}, {0, 0, -1}, {0, 1}, {-1, 0, 0}, {0, 1, 0}, {-1,-1,-1,-1}, {0,0,0,0}},
        // Top face
        {{-s,  s,  s}, {0, 1, 0}, {0, 0}, {1, 0, 0}, {0, 0, -1}, {-1,-1,-1,-1}, {0,0,0,0}},
        {{ s,  s,  s}, {0, 1, 0}, {1, 0}, {1, 0, 0}, {0, 0, -1}, {-1,-1,-1,-1}, {0,0,0,0}},
        {{ s,  s, -s}, {0, 1, 0}, {1, 1}, {1, 0, 0}, {0, 0, -1}, {-1,-1,-1,-1}, {0,0,0,0}},
        {{-s,  s, -s}, {0, 1, 0}, {0, 1}, {1, 0, 0}, {0, 0, -1}, {-1,-1,-1,-1}, {0,0,0,0}},
        // Bottom face
        {{-s, -s, -s}, {0, -1, 0}, {0, 0}, {1, 0, 0}, {0, 0, 1}, {-1,-1,-1,-1}, {0,0,0,0}},
        {{ s, -s, -s}, {0, -1, 0}, {1, 0}, {1, 0, 0}, {0, 0, 1}, {-1,-1,-1,-1}, {0,0,0,0}},
        {{ s, -s,  s}, {0, -1, 0}, {1, 1}, {1, 0, 0}, {0, 0, 1}, {-1,-1,-1,-1}, {0,0,0,0}},
        {{-s, -s,  s}, {0, -1, 0}, {0, 1}, {1, 0, 0}, {0, 0, 1}, {-1,-1,-1,-1}, {0,0,0,0}},
        // Right face
        {{ s, -s,  s}, {1, 0, 0}, {0, 0}, {0, 0, -1}, {0, 1, 0}, {-1,-1,-1,-1}, {0,0,0,0}},
        {{ s, -s, -s}, {1, 0, 0}, {1, 0}, {0, 0, -1}, {0, 1, 0}, {-1,-1,-1,-1}, {0,0,0,0}},
        {{ s,  s, -s}, {1, 0, 0}, {1, 1}, {0, 0, -1}, {0, 1, 0}, {-1,-1,-1,-1}, {0,0,0,0}},
        {{ s,  s,  s}, {1, 0, 0}, {0, 1}, {0, 0, -1}, {0, 1, 0}, {-1,-1,-1,-1}, {0,0,0,0}},
        // Left face
        {{-s, -s, -s}, {-1, 0, 0}, {0, 0}, {0, 0, 1}, {0, 1, 0}, {-1,-1,-1,-1}, {0,0,0,0}},
        {{-s, -s,  s}, {-1, 0, 0}, {1, 0}, {0, 0, 1}, {0, 1, 0}, {-1,-1,-1,-1}, {0,0,0,0}},
        {{-s,  s,  s}, {-1, 0, 0}, {1, 1}, {0, 0, 1}, {0, 1, 0}, {-1,-1,-1,-1}, {0,0,0,0}},
        {{-s,  s, -s}, {-1, 0, 0}, {0, 1}, {0, 0, 1}, {0, 1, 0}, {-1,-1,-1,-1}, {0,0,0,0}},
    };
    
    mesh->indices = {
        0, 1, 2, 2, 3, 0,       // Front
        4, 5, 6, 6, 7, 4,       // Back
        8, 9, 10, 10, 11, 8,    // Top
        12, 13, 14, 14, 15, 12, // Bottom
        16, 17, 18, 18, 19, 16, // Right
        20, 21, 22, 22, 23, 20  // Left
    };
    
    mesh->calculateBounds();
    meshes.push_back(mesh);
    
    // Create default material
    auto mat = std::make_shared<MaterialImpl>();
    materials.push_back(mat);
    
    // Create root entity
    rootEntity = std::make_shared<EntityImpl>();
    rootEntity->mesh = mesh;
    rootEntity->material = mat;
    
    calculateBounds();
    
    return true;
}

void ModelImpl::calculateBounds() {
    if (meshes.empty()) {
        boundsMin = boundsMax = Vec3{0, 0, 0};
        return;
    }
    
    boundsMin = Vec3{std::numeric_limits<float>::max(), 
                     std::numeric_limits<float>::max(), 
                     std::numeric_limits<float>::max()};
    boundsMax = Vec3{std::numeric_limits<float>::lowest(), 
                     std::numeric_limits<float>::lowest(), 
                     std::numeric_limits<float>::lowest()};
    
    for (const auto& mesh : meshes) {
        if (!mesh) continue;
        
        boundsMin.x = std::min(boundsMin.x, mesh->boundsMin.x);
        boundsMin.y = std::min(boundsMin.y, mesh->boundsMin.y);
        boundsMin.z = std::min(boundsMin.z, mesh->boundsMin.z);
        boundsMax.x = std::max(boundsMax.x, mesh->boundsMax.x);
        boundsMax.y = std::max(boundsMax.y, mesh->boundsMax.y);
        boundsMax.z = std::max(boundsMax.z, mesh->boundsMax.z);
    }
}

// ============================================================================
// Builder Class Implementations
// ============================================================================

// Entity Builder
Entity::Entity() : m_impl(std::make_shared<EntityImpl>()) {}

Entity Entity::create() {
    return Entity();
}

Entity& Entity::name(const std::string& n) {
    m_impl->name = n;
    return *this;
}

Entity& Entity::position(float x, float y, float z) {
    m_impl->localTransform.position = Vec3{x, y, z};
    return *this;
}

Entity& Entity::position(const Vec3& pos) {
    m_impl->localTransform.position = pos;
    return *this;
}

Entity& Entity::rotation(float pitch, float yaw, float roll) {
    m_impl->localTransform.rotation = Quaternion::fromEuler(pitch, yaw, roll);
    return *this;
}

Entity& Entity::rotation(const Quaternion& rot) {
    m_impl->localTransform.rotation = rot;
    return *this;
}

Entity& Entity::scale(float x, float y, float z) {
    m_impl->localTransform.scale = Vec3{x, y, z};
    return *this;
}

Entity& Entity::scale(const Vec3& scl) {
    m_impl->localTransform.scale = scl;
    return *this;
}

Entity& Entity::scale(float uniform) {
    m_impl->localTransform.scale = Vec3{uniform, uniform, uniform};
    return *this;
}

Entity& Entity::mesh(MeshHandle m) {
    m_impl->mesh = m;
    return *this;
}

Entity& Entity::material(MaterialHandle mat) {
    m_impl->material = mat;
    return *this;
}

Entity& Entity::visible(bool vis) {
    m_impl->visible = vis;
    return *this;
}

Entity& Entity::addChild(Entity& child) {
    m_impl->addChild(child.m_impl);
    return *this;
}

EntityHandle Entity::build() {
    m_impl->updateWorldTransform();
    return m_impl;
}

// Scene Builder
Scene::Scene() : m_impl(std::make_shared<SceneImpl>()) {
    m_impl->root = std::make_shared<EntityImpl>();
    m_impl->root->name = "__root__";
    m_impl->postProcessor = std::make_shared<PostProcessorImpl>();
}

Scene Scene::create() {
    return Scene();
}

Scene& Scene::backgroundColor(const Color& color) {
    m_impl->backgroundColor = color;
    return *this;
}

Scene& Scene::ambientLight(const Color& color, float intensity) {
    m_impl->ambientColor = color;
    m_impl->ambientIntensity = intensity;
    return *this;
}

Scene& Scene::skybox(const std::string& path) {
    m_impl->skyboxPath = path;
    return *this;
}

Scene& Scene::environmentMap(const std::string& path, float intensity) {
    m_impl->environmentMapPath = path;
    m_impl->environmentIntensity = intensity;
    return *this;
}

Scene& Scene::add(Entity& entity) {
    m_impl->addEntity(entity.build());
    return *this;
}

Scene& Scene::add(Light& light) {
    m_impl->lights.push_back(light.build());
    return *this;
}

Scene& Scene::camera(Camera& cam) {
    m_impl->activeCamera = cam.build();
    return *this;
}

Scene& Scene::postProcessing(PostProcessor& pp) {
    m_impl->postProcessor = pp.build();
    return *this;
}

SceneHandle Scene::build() {
    m_impl->updateTransforms();
    if (m_impl->activeCamera) {
        m_impl->activeCamera->updateMatrices();
    }
    for (auto& light : m_impl->lights) {
        if (light && light->castShadow) {
            light->calculateLightSpaceMatrix();
        }
    }
    return m_impl;
}


// Model Builder
Model::Model() : m_impl(std::make_shared<ModelImpl>()) {}

Model Model::load(const std::string& path) {
    Model model;
    model.m_impl->load(path);
    return model;
}

Model& Model::position(float x, float y, float z) {
    m_transform.position = Vec3{x, y, z};
    return *this;
}

Model& Model::rotation(float pitch, float yaw, float roll) {
    m_transform.rotation = Quaternion::fromEuler(pitch, yaw, roll);
    return *this;
}

Model& Model::scale(float sx, float sy, float sz) {
    m_transform.scale = Vec3{sx, sy, sz};
    return *this;
}

Model& Model::scale(float uniform) {
    m_transform.scale = Vec3{uniform, uniform, uniform};
    return *this;
}

Model& Model::material(Material& mat) {
    m_overrideMaterial = mat.build();
    return *this;
}

ModelHandle Model::build() {
    // Apply transform to root entity
    if (m_impl->rootEntity) {
        m_impl->rootEntity->localTransform = m_transform;
        m_impl->rootEntity->updateWorldTransform();
        
        // Apply override material if set
        if (m_overrideMaterial) {
            m_impl->rootEntity->material = m_overrideMaterial;
        }
    }
    return m_impl;
}

EntityHandle Model::toEntity() {
    build();
    return m_impl->rootEntity;
}

// Camera Builder
Camera::Camera() : m_impl(std::make_shared<CameraImpl>()) {}

Camera Camera::perspective(float fov, float near, float far) {
    Camera cam;
    cam.m_impl->type = CameraType::Perspective;
    cam.m_impl->fov = fov;
    cam.m_impl->nearPlane = near;
    cam.m_impl->farPlane = far;
    return cam;
}

Camera Camera::orthographic(float size, float near, float far) {
    Camera cam;
    cam.m_impl->type = CameraType::Orthographic;
    cam.m_impl->orthoSize = size;
    cam.m_impl->nearPlane = near;
    cam.m_impl->farPlane = far;
    return cam;
}

Camera& Camera::position(float x, float y, float z) {
    m_impl->position = Vec3{x, y, z};
    return *this;
}

Camera& Camera::position(const Vec3& pos) {
    m_impl->position = pos;
    return *this;
}

Camera& Camera::lookAt(float x, float y, float z) {
    m_impl->target = Vec3{x, y, z};
    return *this;
}

Camera& Camera::lookAt(const Vec3& target) {
    m_impl->target = target;
    return *this;
}

Camera& Camera::up(float x, float y, float z) {
    m_impl->up = Vec3{x, y, z};
    return *this;
}

Camera& Camera::up(const Vec3& upVec) {
    m_impl->up = upVec;
    return *this;
}

Camera& Camera::aspectRatio(float ratio) {
    m_impl->aspectRatio = ratio;
    return *this;
}

CameraHandle Camera::build() {
    m_impl->updateMatrices();
    return m_impl;
}

// Light Builder
Light::Light() : m_impl(std::make_shared<LightImpl>()) {}

Light Light::directional(const Color& color, float intensity) {
    Light light;
    light.m_impl->type = LightType::Directional;
    light.m_impl->color = color;
    light.m_impl->intensity = intensity;
    light.m_impl->direction = Vec3{0, -1, 0};
    return light;
}

Light Light::point(const Color& color, float intensity, float range) {
    Light light;
    light.m_impl->type = LightType::Point;
    light.m_impl->color = color;
    light.m_impl->intensity = intensity;
    light.m_impl->range = range;
    return light;
}

Light Light::spot(const Color& color, float intensity, float range, float angle) {
    Light light;
    light.m_impl->type = LightType::Spot;
    light.m_impl->color = color;
    light.m_impl->intensity = intensity;
    light.m_impl->range = range;
    light.m_impl->outerConeAngle = angle;
    light.m_impl->innerConeAngle = angle * 0.8f;
    return light;
}

Light& Light::position(float x, float y, float z) {
    m_impl->position = Vec3{x, y, z};
    return *this;
}

Light& Light::position(const Vec3& pos) {
    m_impl->position = pos;
    return *this;
}

Light& Light::direction(float x, float y, float z) {
    m_impl->direction = Vec3{x, y, z}.normalized();
    return *this;
}

Light& Light::direction(const Vec3& dir) {
    m_impl->direction = dir.normalized();
    return *this;
}

Light& Light::castShadow(bool enabled) {
    m_impl->castShadow = enabled;
    return *this;
}

Light& Light::shadowMapSize(int size) {
    m_impl->shadowMapSize = size;
    return *this;
}

Light& Light::shadowBias(float bias) {
    m_impl->shadowBias = bias;
    return *this;
}

Light& Light::innerConeAngle(float angle) {
    m_impl->innerConeAngle = angle;
    return *this;
}

LightHandle Light::build() {
    if (m_impl->castShadow) {
        m_impl->calculateLightSpaceMatrix();
    }
    return m_impl;
}

// Material Builder
Material::Material() : m_impl(std::make_shared<MaterialImpl>()) {}

Material Material::pbr() {
    return Material();
}

Material& Material::albedo(const Color& color) {
    m_impl->albedoColor = color;
    return *this;
}

Material& Material::albedoMap(const std::string& path) {
    m_impl->albedoMapPath = path;
    m_impl->hasAlbedoMap = !path.empty();
    return *this;
}

Material& Material::normalMap(const std::string& path) {
    m_impl->normalMapPath = path;
    m_impl->hasNormalMap = !path.empty();
    return *this;
}

Material& Material::metallicMap(const std::string& path) {
    m_impl->metallicMapPath = path;
    m_impl->hasMetallicMap = !path.empty();
    return *this;
}

Material& Material::roughnessMap(const std::string& path) {
    m_impl->roughnessMapPath = path;
    m_impl->hasRoughnessMap = !path.empty();
    return *this;
}

Material& Material::aoMap(const std::string& path) {
    m_impl->aoMapPath = path;
    m_impl->hasAoMap = !path.empty();
    return *this;
}

Material& Material::emissiveMap(const std::string& path) {
    m_impl->emissiveMapPath = path;
    m_impl->hasEmissiveMap = !path.empty();
    return *this;
}

Material& Material::metallic(float value) {
    m_impl->metallic = std::clamp(value, 0.0f, 1.0f);
    return *this;
}

Material& Material::roughness(float value) {
    m_impl->roughness = std::clamp(value, 0.0f, 1.0f);
    return *this;
}

Material& Material::ao(float value) {
    m_impl->ao = std::clamp(value, 0.0f, 1.0f);
    return *this;
}

Material& Material::emissive(const Color& color) {
    m_impl->emissiveColor = color;
    return *this;
}

Material& Material::emissiveStrength(float strength) {
    m_impl->emissiveStrength = std::max(0.0f, strength);
    return *this;
}

Material& Material::doubleSided(bool enabled) {
    m_impl->doubleSided = enabled;
    return *this;
}

Material& Material::transparent(bool enabled) {
    m_impl->transparent = enabled;
    return *this;
}

MaterialHandle Material::build() {
    return m_impl;
}

// PostProcessor Builder
PostProcessor::PostProcessor() : m_impl(std::make_shared<PostProcessorImpl>()) {}

PostProcessor PostProcessor::create() {
    return PostProcessor();
}

PostProcessor& PostProcessor::bloom(bool enabled) {
    m_impl->bloomEnabled = enabled;
    return *this;
}

PostProcessor& PostProcessor::bloomThreshold(float threshold) {
    m_impl->bloomThreshold = std::max(0.0f, threshold);
    return *this;
}

PostProcessor& PostProcessor::bloomIntensity(float intensity) {
    m_impl->bloomIntensity = std::max(0.0f, intensity);
    return *this;
}

PostProcessor& PostProcessor::bloomBlurPasses(int passes) {
    m_impl->bloomBlurPasses = std::clamp(passes, 1, 20);
    return *this;
}

PostProcessor& PostProcessor::ssao(bool enabled) {
    m_impl->ssaoEnabled = enabled;
    return *this;
}

PostProcessor& PostProcessor::ssaoRadius(float radius) {
    m_impl->ssaoRadius = std::max(0.01f, radius);
    return *this;
}

PostProcessor& PostProcessor::ssaoBias(float bias) {
    m_impl->ssaoBias = bias;
    return *this;
}

PostProcessor& PostProcessor::ssaoKernelSize(int size) {
    m_impl->ssaoKernelSize = std::clamp(size, 8, 128);
    return *this;
}

PostProcessor& PostProcessor::motionBlur(bool enabled) {
    m_impl->motionBlurEnabled = enabled;
    return *this;
}

PostProcessor& PostProcessor::motionBlurStrength(float strength) {
    m_impl->motionBlurStrength = std::clamp(strength, 0.0f, 2.0f);
    return *this;
}

PostProcessor& PostProcessor::motionBlurSamples(int samples) {
    m_impl->motionBlurSamples = std::clamp(samples, 2, 32);
    return *this;
}

PostProcessor& PostProcessor::colorGrading(bool enabled) {
    m_impl->colorGradingEnabled = enabled;
    return *this;
}

PostProcessor& PostProcessor::exposure(float exp) {
    m_impl->exposure = std::max(0.01f, exp);
    return *this;
}

PostProcessor& PostProcessor::gamma(float g) {
    m_impl->gamma = std::clamp(g, 0.1f, 5.0f);
    return *this;
}

PostProcessor& PostProcessor::contrast(float c) {
    m_impl->contrast = std::clamp(c, 0.0f, 3.0f);
    return *this;
}

PostProcessor& PostProcessor::saturation(float s) {
    m_impl->saturation = std::clamp(s, 0.0f, 3.0f);
    return *this;
}

PostProcessor& PostProcessor::colorFilter(const Color& filter) {
    m_impl->colorFilter = Vec3{filter.r, filter.g, filter.b};
    return *this;
}

PostProcessor& PostProcessor::toneMapping(PostProcessorImpl::ToneMapping tm) {
    m_impl->toneMapping = tm;
    return *this;
}

PostProcessor& PostProcessor::vignette(bool enabled) {
    m_impl->vignetteEnabled = enabled;
    return *this;
}

PostProcessor& PostProcessor::vignetteIntensity(float intensity) {
    m_impl->vignetteIntensity = std::clamp(intensity, 0.0f, 1.0f);
    return *this;
}

PostProcessor& PostProcessor::vignetteSmoothness(float smoothness) {
    m_impl->vignetteSmoothness = std::clamp(smoothness, 0.0f, 1.0f);
    return *this;
}

PostProcessor& PostProcessor::depthOfField(bool enabled) {
    m_impl->dofEnabled = enabled;
    return *this;
}

PostProcessor& PostProcessor::dofFocusDistance(float distance) {
    m_impl->dofFocusDistance = std::max(0.1f, distance);
    return *this;
}

PostProcessor& PostProcessor::dofFocusRange(float range) {
    m_impl->dofFocusRange = std::max(0.1f, range);
    return *this;
}

PostProcessor& PostProcessor::dofBokehSize(float size) {
    m_impl->dofBokehSize = std::clamp(size, 1.0f, 20.0f);
    return *this;
}

PostProcessorHandle PostProcessor::build() {
    return m_impl;
}

// Skeleton Builder
Skeleton::Skeleton() : m_impl(std::make_shared<SkeletonImpl>()) {}

Skeleton Skeleton::create() {
    return Skeleton();
}

Skeleton& Skeleton::addBone(Bone& bone) {
    auto boneHandle = bone.build();
    if (boneHandle) {
        m_impl->boneNameToIndex[boneHandle->name] = static_cast<int>(m_impl->bones.size());
        m_impl->bones.push_back(boneHandle);
    }
    return *this;
}

Skeleton& Skeleton::globalInverseTransform(const Mat4& mat) {
    m_impl->globalInverseTransform = mat;
    return *this;
}

SkeletonHandle Skeleton::build() {
    m_impl->updateBoneTransforms();
    return m_impl;
}

// Bone Builder
Bone::Bone() : m_impl(std::make_shared<BoneImpl>()) {}

Bone Bone::create(const std::string& name, int id) {
    Bone bone;
    bone.m_impl->name = name;
    bone.m_impl->id = id;
    bone.m_impl->offsetMatrix = Mat4::identity();
    bone.m_impl->localTransform = Mat4::identity();
    bone.m_impl->worldTransform = Mat4::identity();
    return bone;
}

Bone& Bone::offsetMatrix(const Mat4& mat) {
    m_impl->offsetMatrix = mat;
    return *this;
}

Bone& Bone::localTransform(const Mat4& mat) {
    m_impl->localTransform = mat;
    return *this;
}

Bone& Bone::parent(BoneHandle parentBone) {
    m_impl->parent = parentBone;
    if (parentBone) {
        parentBone->children.push_back(m_impl);
    }
    return *this;
}

BoneHandle Bone::build() {
    return m_impl;
}

// ============================================================================
// Model Loader Implementation
// ============================================================================

ModelHandle ModelLoader::loadFromFile(const std::string& path, const LoadOptions& options) {
    auto model = std::make_shared<ModelImpl>();
    
    // Check if file exists
    std::ifstream file(path);
    if (!file.good()) {
        return nullptr;
    }
    file.close();
    
    // Get extension
    size_t dotPos = path.rfind('.');
    if (dotPos == std::string::npos) {
        return nullptr;
    }
    
    std::string ext = path.substr(dotPos + 1);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    
    if (!isFormatSupported(ext)) {
        return nullptr;
    }
    
    // Load the model
    if (!model->load(path)) {
        return nullptr;
    }
    
    // Apply options
    if (options.calculateTangents) {
        for (auto& mesh : model->meshes) {
            if (mesh) {
                mesh->calculateTangents();
            }
        }
    }
    
    if (std::abs(options.scaleFactor - 1.0f) > 0.0001f) {
        // Apply scale to all vertices
        for (auto& mesh : model->meshes) {
            if (mesh) {
                for (auto& v : mesh->vertices) {
                    v.position = v.position * options.scaleFactor;
                }
                mesh->calculateBounds();
            }
        }
        model->calculateBounds();
    }
    
    return model;
}

bool ModelLoader::isFormatSupported(const std::string& extension) {
    std::string ext = extension;
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    
    static const std::vector<std::string> supported = {
        "obj", "fbx", "gltf", "glb", "dae", "3ds", "blend", "stl", "ply"
    };
    
    return std::find(supported.begin(), supported.end(), ext) != supported.end();
}

std::vector<std::string> ModelLoader::getSupportedFormats() {
    return {"obj", "fbx", "gltf", "glb", "dae", "3ds", "blend", "stl", "ply"};
}

} // namespace KGK3D
