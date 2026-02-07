#pragma once
#include "utils.hpp"
#include <vector>

// Cápsula (cilindro com hemisférios nas pontas)
struct Capsule {
    Vector3 start;
    Vector3 end;
    float radius;
};

namespace HitboxSystem {
    // Offsets para hitboxes (precisam ser atualizados)
    constexpr ptrdiff_t m_pCollision = 0x338;
    constexpr ptrdiff_t m_vecMins = 0x00;
    constexpr ptrdiff_t m_vecMaxs = 0x0C;
    
    // Converter bone matrix para hitbox
    inline Capsule BoneToCapsule(const Vector3& boneStart, const Vector3& boneEnd, float radius) {
        return Capsule{ boneStart, boneEnd, radius };
    }
    
    // Projetar cápsula 3D para polígono 2D
    inline std::vector<ImVec2> ProjectCapsuleToScreen(
        const Capsule& capsule,
        const Matrix& viewMatrix,
        int segments = 16
    ) {
        std::vector<ImVec2> points;
        
        // Vetor direção da cápsula
        Vector3 dir = {
            capsule.end.x - capsule.start.x,
            capsule.end.y - capsule.start.y,
            capsule.end.z - capsule.start.z
        };
        
        float length = sqrt(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);
        if (length < 0.001f) {
            // Cápsula degenerada = esfera
            Vector2 center2D = WorldToScreen(capsule.start, viewMatrix);
            if (center2D.x == 0) return points;
            
            for (int i = 0; i < segments; ++i) {
                float angle = (float)i / segments * 2.0f * 3.14159265f;
                points.push_back({
                    center2D.x + cos(angle) * capsule.radius * 10.0f,
                    center2D.y + sin(angle) * capsule.radius * 10.0f
                });
            }
            return points;
        }
        
        // Normalizar direção
        dir.x /= length;
        dir.y /= length;
        dir.z /= length;
        
        // Criar vetor perpendicular (para círculos ao redor da cápsula)
        Vector3 perp;
        if (abs(dir.z) < 0.9f) {
            perp = { -dir.y, dir.x, 0.0f };
        } else {
            perp = { 0.0f, -dir.z, dir.y };
        }
        
        // Normalizar perpendicular
        float perpLen = sqrt(perp.x * perp.x + perp.y * perp.y + perp.z * perp.z);
        perp.x /= perpLen;
        perp.y /= perpLen;
        perp.z /= perpLen;
        
        // Gerar pontos ao redor da cápsula
        for (int i = 0; i <= segments / 2; ++i) {
            float angle = (float)i / segments * 2.0f * 3.14159265f;
            
            // Ponto no círculo inicial
            Vector3 p1 = {
                capsule.start.x + perp.x * cos(angle) * capsule.radius,
                capsule.start.y + perp.y * cos(angle) * capsule.radius,
                capsule.start.z + perp.z * cos(angle) * capsule.radius
            };
            
            Vector2 p1_2d = WorldToScreen(p1, viewMatrix);
            if (p1_2d.x != 0) points.push_back({ p1_2d.x, p1_2d.y });
        }
        
        for (int i = segments / 2; i <= segments; ++i) {
            float angle = (float)i / segments * 2.0f * 3.14159265f;
            
            // Ponto no círculo final
            Vector3 p2 = {
                capsule.end.x + perp.x * cos(angle) * capsule.radius,
                capsule.end.y + perp.y * cos(angle) * capsule.radius,
                capsule.end.z + perp.z * cos(angle) * capsule.radius
            };
            
            Vector2 p2_2d = WorldToScreen(p2, viewMatrix);
            if (p2_2d.x != 0) points.push_back({ p2_2d.x, p2_2d.y });
        }
        
        return points;
    }
    
    // Definição das hitboxes do CS2 (baseado em análise reversa)
    struct HitboxDefinition {
        int startBone;
        int endBone;
        float radius;
        const char* name;
    };
    
    // Hitboxes oficiais do CS2
    inline const std::vector<HitboxDefinition>& GetCS2Hitboxes() {
        static std::vector<HitboxDefinition> hitboxes = {
            // Cabeça
            { 6, 6, 0.110f, "Head" },
            
            // Torso (coluna vertebral)
            { 0, 2, 0.135f, "Pelvis-Spine1" },
            { 2, 4, 0.130f, "Spine1-Spine2" },
            { 4, 5, 0.120f, "Spine2-Neck" },
            { 5, 6, 0.100f, "Neck-Head" },
            
            // Braço esquerdo
            { 5, 13, 0.070f, "LeftShoulder" },
            { 13, 14, 0.065f, "LeftUpperArm" },
            { 14, 15, 0.055f, "LeftForearm" },
            
            // Braço direito
            { 5, 8, 0.070f, "RightShoulder" },
            { 8, 9, 0.065f, "RightUpperArm" },
            { 9, 10, 0.055f, "RightForearm" },
            
            // Perna esquerda
            { 0, 25, 0.095f, "LeftHip" },
            { 25, 26, 0.085f, "LeftThigh" },
            { 26, 27, 0.065f, "LeftCalf" },
            
            // Perna direita
            { 0, 22, 0.095f, "RightHip" },
            { 22, 23, 0.085f, "RightThigh" },
            { 23, 24, 0.065f, "RightCalf" }
        };
        return hitboxes;
    }
}
