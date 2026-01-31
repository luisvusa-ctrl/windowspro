#pragma once

#include <fstream>
#include <thread>

#include "utils.hpp"

// https://github.com/Read1dno/VisCheckCS2

const size_t LEAF_THRESHOLD{ 4 };

struct Triangle
{
    int a{ 0 };
    int b{ 0 };
    int c{ 0 };

    Triangle() = default;
    Triangle(int a_, int b_, int c_) : a(a_), b(b_), c(c_) {}
};

struct AABB
{
    Vector3 min;
    Vector3 max;

    bool RayIntersects(const Vector3& rayOrigin, const Vector3& rayDir) const
    {
        float tmin{ std::numeric_limits<float>::lowest() };
        float tmax{ (std::numeric_limits<float>::max)() };

        const float* rayOriginArr{ &rayOrigin.x };
        const float* rayDirArr{ &rayDir.x };
        const float* minArr{ &min.x };
        const float* maxArr{ &max.x };

        for (int i{ 0 }; i < 3; ++i) {
            float invDir{ 1.0f / rayDirArr[i] };
            float t0{ (minArr[i] - rayOriginArr[i]) * invDir };
            float t1{ (maxArr[i] - rayOriginArr[i]) * invDir };

            if (invDir < 0.0f) std::swap(t0, t1);
            tmin = (std::max)(tmin, t0);
            tmax = (std::min)(tmax, t1);
        }

        return tmax >= tmin && tmax >= 0;
    }
};

struct TriangleCombined
{
    Vector3 v0;
    Vector3 v1;
    Vector3 v2;

    TriangleCombined() = default;
    TriangleCombined(const Vector3& v0_, const Vector3& v1_, const Vector3& v2_): v0(v0_), v1(v1_), v2(v2_) {}

    AABB ComputeAABB() const
    {
        Vector3 min_point;
        Vector3 max_point;

        min_point.x = (std::min)({ v0.x, v1.x, v2.x });
        min_point.y = (std::min)({ v0.y, v1.y, v2.y });
        min_point.z = (std::min)({ v0.z, v1.z, v2.z });

        max_point.x = (std::max)({ v0.x, v1.x, v2.x });
        max_point.y = (std::max)({ v0.y, v1.y, v2.y });
        max_point.z = (std::max)({ v0.z, v1.z, v2.z });

        return { min_point, max_point };
    }
};

struct BVHNode
{
    AABB bounds;
    std::unique_ptr<BVHNode> left;
    std::unique_ptr<BVHNode> right;
    std::vector<TriangleCombined> triangles;

    bool IsLeaf() const
    {
        return left == nullptr && right == nullptr;
    }
};


class Parser
{
private:
    std::string DataPath;

    std::vector<std::vector<Triangle>> TrianglesList;
    std::vector<std::vector<Vector3>> VerticesList;
    std::vector<std::vector<TriangleCombined>> CombinedList;

    std::vector<std::vector<Triangle>> GetTriangles();
    std::vector<std::vector<Vector3>> GetVertices();

    template<typename T>
    std::vector<T> ParseElements(const unsigned char* data, size_t dataSize);

    template<typename T>
    std::vector<std::vector<T>> ParseSection(const unsigned char* fileData, size_t fileSize, const std::string& sectionName);

    void fetchTriangles()
    {
        TrianglesList = GetTriangles();
    }

    void fetchVertices()
    {
        VerticesList = GetVertices();
    }

public:
    Parser(const std::string& path);

    const std::vector<std::vector<TriangleCombined>>& GetCombinedList() const
    {
        return CombinedList;
    }
};

class Geometry
{
public:
    std::vector<std::vector<TriangleCombined>> meshes;

    bool LoadFromFile(const std::string& optimizedFile);

    bool CreateOptimizedFile(const std::string& rawFile, const std::string& optimizedFile);
};

class VisCheck
{
public:
    VisCheck(const std::string& geometryFile);
    bool IsPointVisible(const Vector3& point1, const Vector3& point2);
    bool RayIntersectsTriangle(const Vector3& rayOrigin, const Vector3& rayDir, const TriangleCombined& triangle, float& t);

private:
    Geometry geometry;
    std::vector<std::unique_ptr<BVHNode>> bvhNodes;
    std::unique_ptr<BVHNode> BuildBVH(const std::vector<TriangleCombined>& tris);
    bool IntersectBVH(const BVHNode* node, const Vector3& rayOrigin, const Vector3& rayDir, float maxDistance, float& hitDistance);
};

bool Geometry::CreateOptimizedFile(const std::string& rawFile, const std::string& optimizedFile)
{
    Parser parser(rawFile);
    meshes = parser.GetCombinedList();

    std::ofstream out(optimizedFile, std::ios::binary);
    if (!out)
    {
        std::cerr << "Failed to open file: " << optimizedFile << '\n';
        return false;
    }

    size_t numMeshes{ meshes.size() };
    out.write(reinterpret_cast<const char*>(&numMeshes), sizeof(size_t));

    for (const auto& mesh : meshes) {
        size_t numTris{ mesh.size() };
        out.write(reinterpret_cast<const char*>(&numTris), sizeof(size_t));

        for (const auto& tri : mesh)
        {
            out.write(reinterpret_cast<const char*>(&tri.v0), sizeof(Vector3));
            out.write(reinterpret_cast<const char*>(&tri.v1), sizeof(Vector3));
            out.write(reinterpret_cast<const char*>(&tri.v2), sizeof(Vector3));
        }
    }
    out.close();
    return true;
}

bool Geometry::LoadFromFile(const std::string& optimizedFile)
{
    std::ifstream in(optimizedFile, std::ios::binary);
    if (!in) {
        std::cerr << "Failed to open file: " << optimizedFile << '\n';
        return false;
    }
    meshes.clear();

    size_t numMeshes;
    in.read(reinterpret_cast<char*>(&numMeshes), sizeof(size_t));

    for (size_t i{ 0 }; i < numMeshes; ++i) {
        size_t numTris;
        in.read(reinterpret_cast<char*>(&numTris), sizeof(size_t));

        std::vector<TriangleCombined> mesh;
        mesh.resize(numTris);

        for (size_t j{ 0 }; j < numTris; ++j) {
            in.read(reinterpret_cast<char*>(&mesh[j].v0), sizeof(Vector3));
            in.read(reinterpret_cast<char*>(&mesh[j].v1), sizeof(Vector3));
            in.read(reinterpret_cast<char*>(&mesh[j].v2), sizeof(Vector3));
        }
        meshes.push_back(mesh);
    }

    in.close();
    return true;
}

static std::vector<unsigned char> HexStringToBytes(const std::string& hex)
{
    std::string hexCleaned;
    hexCleaned.reserve(hex.length());

    std::copy_if(hex.begin(), hex.end(), std::back_inserter(hexCleaned), [](char c) {
        return !std::isspace(c);
    });

    std::vector<unsigned char> bytes;
    bytes.reserve(hexCleaned.length() / 2);

    for (size_t i{ 0 }; i < hexCleaned.length(); i += 2) {
        std::string byteString{ hexCleaned.substr(i, 2) };
        unsigned char byte{ static_cast<unsigned char>(std::stoul(byteString, nullptr, 16)) };
        bytes.push_back(byte);
    }

    return bytes;
}

Parser::Parser(const std::string& path) : DataPath(path)
{
    std::thread trianglesThread(&Parser::fetchTriangles, this);
    std::thread verticesThread(&Parser::fetchVertices, this);

    trianglesThread.join();
    verticesThread.join();

    for (size_t i{ 0 }; i < TrianglesList.size(); ++i) {
        const std::vector<Triangle>& triangles{ TrianglesList[i] };
        const std::vector<Vector3>& vertices{ VerticesList[i] };

        std::vector<TriangleCombined> Combined;

        for (const Triangle& triangle : triangles)
        {
            TriangleCombined t;
            t.v0 = vertices[triangle.a];
            t.v1 = vertices[triangle.b];
            t.v2 = vertices[triangle.c];
            Combined.push_back(t);
        }

        CombinedList.push_back(Combined);
    }
}

template<typename T>
std::vector<T> Parser::ParseElements(const unsigned char* data, size_t dataSize)
{
    std::vector<T> elements;
    size_t elementSize{ sizeof(T) };
    elements.reserve(dataSize / elementSize);

    for (size_t i{ 0 }; i < dataSize; i += elementSize) {
        T element;
        std::memcpy(&element, data + i, elementSize);
        elements.push_back(element);
    }

    return elements;
}

template<typename T>
std::vector<std::vector<T>> Parser::ParseSection(const unsigned char* fileData, size_t fileSize, const std::string& sectionName)
{
    std::vector<std::vector<T>> elementsLists;
    std::istringstream fileStream(std::string(reinterpret_cast<const char*>(fileData), fileSize));
    std::string line;
    bool inMeshSection{ false };

    while (std::getline(fileStream, line)) {
        if (line.find("m_meshes") != std::string::npos) {
            inMeshSection = true;
        }

        if (inMeshSection && line.find(sectionName) != std::string::npos)
        {
            std::getline(fileStream, line);
            if (line.find("#[") != std::string::npos) {
                std::string hexString;
                while (std::getline(fileStream, line) && line.find("]") == std::string::npos) {
                    hexString += line;
                }

                auto bytes{ HexStringToBytes(hexString) };
                auto parsedElements{ ParseElements<T>(bytes.data(), bytes.size()) };
                elementsLists.push_back(std::move(parsedElements));
            }
        }
    }

    return elementsLists;
}

std::vector<std::vector<Triangle>> Parser::GetTriangles()
{
    std::ifstream file(DataPath, std::ios::binary);
    if (!file.is_open()) {
        return {};
    }

    file.seekg(0, std::ios::end);
    size_t fileSize{ static_cast<size_t>(file.tellg()) };
    file.seekg(0, std::ios::beg);

    std::vector<unsigned char> fileData(fileSize);
    file.read(reinterpret_cast<char*>(fileData.data()), fileSize);

    return ParseSection<Triangle>(fileData.data(), fileSize, "m_Triangles");
}

std::vector<std::vector<Vector3>> Parser::GetVertices()
{
    std::ifstream file(DataPath, std::ios::binary);
    if (!file.is_open()) {
        return {};
    }

    file.seekg(0, std::ios::end);
    size_t fileSize{ static_cast<size_t>(file.tellg()) };
    file.seekg(0, std::ios::beg);

    std::vector<unsigned char> fileData(fileSize);
    file.read(reinterpret_cast<char*>(fileData.data()), fileSize);

    return ParseSection<Vector3>(fileData.data(), fileSize, "m_Vertices");
}

VisCheck::VisCheck(const std::string& geometryFile)
{
    if (geometryFile != "")
    {
        if (!geometry.LoadFromFile(geometryFile)) {
            std::cerr << "Failed to load file: " << geometryFile << '\n';
        }

        for (const auto& mesh : geometry.meshes) {
            bvhNodes.push_back(BuildBVH(mesh));
        }
    }
}

std::unique_ptr<BVHNode> VisCheck::BuildBVH(const std::vector<TriangleCombined>& tris)
{
    auto node{ std::make_unique<BVHNode>() };

    if (tris.empty()) return node;

    AABB bounds{ tris[0].ComputeAABB() };
    for (size_t i{ 1 }; i < tris.size(); ++i) {
        AABB triAABB{ tris[i].ComputeAABB() };
        bounds.min.x = (std::min)(bounds.min.x, triAABB.min.x);
        bounds.min.y = (std::min)(bounds.min.y, triAABB.min.y);
        bounds.min.z = (std::min)(bounds.min.z, triAABB.min.z);
        bounds.max.x = (std::max)(bounds.max.x, triAABB.max.x);
        bounds.max.y = (std::max)(bounds.max.y, triAABB.max.y);
        bounds.max.z = (std::max)(bounds.max.z, triAABB.max.z);
    }

    node->bounds = bounds;

    if (tris.size() <= LEAF_THRESHOLD)
    {
        node->triangles = tris;
        return node;
    }

    Vector3 diff{ bounds.max - bounds.min };
    int axis{ (diff.x > diff.y && diff.x > diff.z) ? 0 : ((diff.y > diff.z) ? 1 : 2) };
    std::vector<TriangleCombined> sortedTris{ tris };

    std::sort(sortedTris.begin(), sortedTris.end(), [axis](const TriangleCombined& a, const TriangleCombined& b)
        {
        AABB aabbA{ a.ComputeAABB() };
        AABB aabbB{ b.ComputeAABB() };
        float centerA, centerB;
        if (axis == 0)
        {
            centerA = (aabbA.min.x + aabbA.max.x) / 2.0f;
            centerB = (aabbB.min.x + aabbB.max.x) / 2.0f;
        }
        else if (axis == 1)
        {
            centerA = (aabbA.min.y + aabbA.max.y) / 2.0f;
            centerB = (aabbB.min.y + aabbB.max.y) / 2.0f;
        }
        else
        {
            centerA = (aabbA.min.z + aabbA.max.z) / 2.0f;
            centerB = (aabbB.min.z + aabbB.max.z) / 2.0f;
        }
        return centerA < centerB;
    });

    size_t mid{ sortedTris.size() / 2 };
    std::vector<TriangleCombined> leftTris(sortedTris.begin(), sortedTris.begin() + mid);
    std::vector<TriangleCombined> rightTris(sortedTris.begin() + mid, sortedTris.end());

    node->left = BuildBVH(leftTris);
    node->right = BuildBVH(rightTris);

    return node;
}

bool VisCheck::IntersectBVH(const BVHNode* node, const Vector3& rayOrigin, const Vector3& rayDir, float maxDistance, float& hitDistance)
{
    if (!node->bounds.RayIntersects(rayOrigin, rayDir))
    {
        return false;
    }

    bool hit{ false };
    if (node->IsLeaf())
    {
        for (const auto& tri : node->triangles)
        {
            float t;
            if (RayIntersectsTriangle(rayOrigin, rayDir, tri, t))
            {
                if (t < maxDistance && t < hitDistance)
                {
                    hitDistance = t;
                    hit = true;
                }
            }
        }
    }
    else {
        if (node->left)
        {
            hit |= IntersectBVH(node->left.get(), rayOrigin, rayDir, maxDistance, hitDistance);
        }
        if (node->right)
        {
            hit |= IntersectBVH(node->right.get(), rayOrigin, rayDir, maxDistance, hitDistance);
        }
    }

    return hit;
}

bool VisCheck::IsPointVisible(const Vector3& point1, const Vector3& point2)
{
    Vector3 rayDir{ point2.x - point1.x, point2.y - point1.y, point2.z - point1.z };
    float distance{ std::sqrt(rayDir.dot(rayDir)) };
    rayDir = { rayDir.x / distance, rayDir.y / distance, rayDir.z / distance };
    float hitDistance{ (std::numeric_limits<float>::max)() };

    for (const auto& bvhRoot : bvhNodes)
    {
        if (IntersectBVH(bvhRoot.get(), point1, rayDir, distance, hitDistance))
        {
            if (hitDistance < distance)
            {
                return false;
            }
        }
    }

    return true;
}

bool VisCheck::RayIntersectsTriangle(const Vector3& rayOrigin, const Vector3& rayDir, const TriangleCombined& triangle, float& t)
{
    const float EPSILON{ 1e-7f };

    Vector3 edge1{ triangle.v1 - triangle.v0 };
    Vector3 edge2{ triangle.v2 - triangle.v0 };
    Vector3 h{ rayDir.cross(edge2) };
    float a{ edge1.dot(h) };

    if (a > -EPSILON && a < EPSILON)
        return false;

    float f{ 1.0f / a };
    Vector3 s{ rayOrigin - triangle.v0 };
    float u{ f * s.dot(h) };

    if (u < 0.0f || u > 1.0f)
        return false;

    Vector3 q{ s.cross(edge1) };
    float v{ f * rayDir.dot(q) };

    if (v < 0.0f || u + v > 1.0f)
        return false;

    t = f * edge2.dot(q);

    return (t > EPSILON);
}