#include <vector>
#include <chrono>
#include <random>
#include <print>
#include <string>
#include <cmath>
#include <algorithm>
#include "Engine/Registry/Registry.h"
#include "entt.hpp"

struct Position { float x, y, z; };
struct Velocity { float dx, dy, dz; };
struct DataBlob { double matrix[16]; int id; };

template<typename Func>
void Measure(std::string_view name, Func&& func) {
    auto start = std::chrono::high_resolution_clock::now();
    func();
    auto end = std::chrono::high_resolution_clock::now();
    double ms = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() / 1000.0;
    std::println("\033[1;32m{:<30}\033[0m : \033[1;33m{:>10.4f} ms\033[0m", name, ms);
}

void RunEcsBenchmark(int entityCount, int iterations) {
    std::println("==================================================");
    std::println("   ECS BENCHMARK (Entities: {}, Iterations: {})", entityCount, iterations);
    std::println("==================================================");

    Syn::Registry synReg;
    entt::registry enttReg;

    std::println("\n--- 1. Creation & Insertion ---");

    Measure("Syn::Registry Create + Add", [&]() {
        for (int i = 0; i < entityCount; ++i) {
            auto e = synReg.CreateEntity();
            synReg.AddComponent<Position>(e, { 1.0f * i, 2.0f * i, 0.0f });
            synReg.AddComponent<Velocity>(e, { 0.1f, 0.1f, 0.1f });
            if (i % 2 == 0) synReg.AddComponent<DataBlob>(e);
        }
        });

    Measure("EnTT Registry Create + Emplace", [&]() {
        for (int i = 0; i < entityCount; ++i) {
            auto e = enttReg.create();
            enttReg.emplace<Position>(e, 1.0f * i, 2.0f * i, 0.0f);
            enttReg.emplace<Velocity>(e, 0.1f, 0.1f, 0.1f);
            if (i % 2 == 0) enttReg.emplace<DataBlob>(e);
        }
        });

    std::println("\n--- 2. Iteration (Non-trivial math) ---");

    float dt = 0.016f;

    Measure("Syn::Registry ViewSeq", [&]() {
        for (int k = 0; k < iterations; ++k) {
            synReg.ViewSeq<Position, Velocity>([dt](Position& pos, Velocity& vel) {
                pos.x += vel.dx * dt;
                pos.y += vel.dy * dt;
                pos.z += vel.dz * dt;
                float dist = std::sqrt(pos.x * pos.x + pos.y * pos.y);
                pos.z += std::sin(dist);
                });
        }
        });

    Measure("EnTT View Each", [&]() {
        for (int k = 0; k < iterations; ++k) {
            enttReg.view<Position, Velocity>().each([dt](auto& pos, auto& vel) {
                pos.x += vel.dx * dt;
                pos.y += vel.dy * dt;
                pos.z += vel.dz * dt;
                float dist = std::sqrt(pos.x * pos.x + pos.y * pos.y);
                pos.z += std::sin(dist);
                });
        }
        });

    std::println("\n--- 3. Syn Direct Pool Access ---");

    Measure("Syn::Registry GetPool -> Loop", [&]() {
        for (int k = 0; k < iterations; ++k) {
            auto* posPool = synReg.GetPool<Position>();
            auto* velPool = synReg.GetPool<Velocity>();
            auto entities = posPool->GetDenseEntities();

            for (auto entity : entities) {
                if (velPool->Has(entity)) {
                    Position& pos = posPool->Get(entity);
                    Velocity& vel = velPool->Get(entity);

                    pos.x += vel.dx * dt;
                    pos.y += vel.dy * dt;
                    pos.z -= std::cos(pos.x);
                }
            }
        }
        });

    std::println("\n--- 4. Random Access (GetComponent) ---");

    std::vector<Syn::EntityID> randomSynEntities;
    {
        auto span = synReg.GetPool<Position>()->GetDenseEntities();
        randomSynEntities.assign(span.begin(), span.end());
    }
    std::vector<entt::entity> randomEnttEntities;
    {
        auto view = enttReg.view<Position>();
        randomEnttEntities.assign(view.begin(), view.end());
    }

    std::mt19937 gen(std::random_device{}());
    std::shuffle(randomSynEntities.begin(), randomSynEntities.end(), gen);
    std::shuffle(randomEnttEntities.begin(), randomEnttEntities.end(), gen);

    size_t sampleSize = entityCount / 2;
    volatile float accumulator = 0.0f;

    Measure("Syn::Registry GetComponent", [&]() {
        for (int k = 0; k < iterations; ++k) {
            for (size_t i = 0; i < sampleSize; ++i) {
                const auto& pos = synReg.GetComponent<Position>(randomSynEntities[i]);
                accumulator += pos.x;
            }
        }
        });

    Measure("EnTT get", [&]() {
        for (int k = 0; k < iterations; ++k) {
            for (size_t i = 0; i < sampleSize; ++i) {
                const auto& pos = enttReg.get<Position>(randomEnttEntities[i]);
                accumulator += pos.x;
            }
        }
        });

    std::println("\n--- 5. Destruction ---");

    std::vector<Syn::EntityID> synToDestroy;
    {
        auto span = synReg.GetPool<Position>()->GetDenseEntities();
        synToDestroy.assign(span.begin(), span.end());
    }

    Measure("Syn::Registry DestroyAll", [&]() {
        for (auto e : synToDestroy) {
            synReg.DestroyEntity(e);
        }
        });

    std::vector<entt::entity> enttToDestroy;
    {
        auto view = enttReg.view<Position>();
        enttToDestroy.assign(view.begin(), view.end());
    }

    Measure("EnTT DestroyAll", [&]() {
        for (auto e : enttToDestroy) {
            enttReg.destroy(e);
        }
        });
}

int main() {
    RunEcsBenchmark(1'000'000, 10);
    return 0;
}