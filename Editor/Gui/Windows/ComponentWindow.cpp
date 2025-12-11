#include "ComponentWindow.h"
#include "Editor/Gui/Utils/Panel.h"
#include <glm/gtc/type_ptr.hpp>

#include "Engine/Components/TagComponent.h"
#include "Engine/Components/TransformComponent.h"
#include "Engine/Components/CameraComponent.h"

#include "Engine/Components/MaterialComponent.h"
#include "Engine/Components/ShapeComponent.h"
#include "Engine/Components/ModelComponent.h"
#include "Engine/Components/AnimationComponent.h"

#include "Engine/Components/DirectionLightComponent.h"
#include "Engine/Components/PointLightComponent.h"
#include "Engine/Components/SpotLightComponent.h"

#include "Engine/Utils/BitsetFlag.h"

#define TITLE_CP(title) (std::string(title) + "##ComponentPanel").c_str()

constexpr float ALIGN_WIDTH = 100.0f;

void ComponentWindow::Render(std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, std::set<VkDescriptorSet>& textureSet, uint32_t frameIndex)
{
    static Window::Config windowConfig{
        .name = "Components"
    };

    Window::RenderWindow(windowConfig, [&]() -> void
        {
            auto activeEntity = registry->GetActiveEntity();

            if (activeEntity != NULL_ENTITY)
            {
                if (registry->HasComponent<TagComponent>(activeEntity))
                    RenderTagComponent(registry, activeEntity);

                if (registry->HasComponent<TransformComponent>(activeEntity))
                    RenderTransformComponent(registry, activeEntity);

                if (registry->HasComponent<CameraComponent>(activeEntity))
                    RenderCameraComponent(registry, activeEntity);

                if (registry->HasComponent<ShapeComponent>(activeEntity))
                    RenderShapeComponent(registry, activeEntity);

                if (registry->HasComponent<ModelComponent>(activeEntity))
                    RenderModelComponent(registry, activeEntity);

                if (registry->HasComponent<MaterialComponent>(activeEntity))
                    RenderMaterialComponent(registry, activeEntity);

                if (registry->HasComponent<AnimationComponent>(activeEntity))
                    RenderAnimationComponent(registry, activeEntity);

                if (registry->HasComponent<DirectionLightComponent>(activeEntity))
                    RenderDirectionLightComponent(registry, activeEntity);

                if (registry->HasComponent<PointLightComponent>(activeEntity))
                    RenderPointLightComponent(registry, activeEntity);

                if (registry->HasComponent<SpotLightComponent>(activeEntity))
                    RenderSpotLightComponent(registry, activeEntity);

                ImGui::Separator();
                if (ImGui::Button("Add Component", ImVec2(ImGui::GetContentRegionAvail().x, 0)))
                {
                    ImGui::OpenPopup("AddComponentPopup");
                }

                RenderAddComponentPopUp(registry, activeEntity);
            }
            else
            {
                ImGui::Text("No entity selected.");
            }
        });
}

void ComponentWindow::DrawLabel(const char* label)
{
    ImGui::AlignTextToFramePadding();
    ImGui::Text("%s", label);
    ImGui::SameLine(ALIGN_WIDTH);
}

void ComponentWindow::DrawVec3Control(const std::string& label, glm::vec3& values, float resetValue, const std::function<void()>& onUpdate)
{
    ImGui::PushID(label.c_str());

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::AlignTextToFramePadding();
    ImGui::Text("%s", label.c_str());

    ImGui::TableSetColumnIndex(1);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

    float lineHeight = ImGui::GetFont()->FontSize + ImGui::GetStyle().FramePadding.y * 2.0f;
    ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

    bool changed = false;

    // X Axis
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
    if (ImGui::Button("X", buttonSize))
    {
        values.x = resetValue;
        changed = true;
    }
    ImGui::PopStyleColor(3);

    ImGui::SameLine();
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    if (ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f")) changed = true;

    ImGui::TableSetColumnIndex(2);
    // Y Axis
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
    if (ImGui::Button("Y", buttonSize))
    {
        values.y = resetValue;
        changed = true;
    }
    ImGui::PopStyleColor(3);

    ImGui::SameLine();
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    if (ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f")) changed = true;

    ImGui::TableSetColumnIndex(3);
    // Z Axis
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
    if (ImGui::Button("Z", buttonSize))
    {
        values.z = resetValue;
        changed = true;
    }
    ImGui::PopStyleColor(3);

    ImGui::SameLine();
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    if (ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f")) changed = true;

    ImGui::PopStyleVar();
    ImGui::PopID();

    if (changed)
        onUpdate();
}

void ComponentWindow::RenderTagComponent(std::shared_ptr<Registry> registry, Entity entity)
{
    auto& component = registry->GetComponent<TagComponent>(entity);
    auto pool = registry->GetPool<TagComponent>();
    static bool visible = true;

    if (ImGui::CollapsingHeader(TITLE_CP("Tag Component"), &visible, ImGuiTreeNodeFlags_DefaultOpen))
    {
        // Name
        std::string bufferName = component.name;
        char buffer[256];
        memset(buffer, 0, sizeof(buffer));
        strcpy_s(buffer, sizeof(buffer), bufferName.c_str());

        DrawLabel("Name");
        if (ImGui::InputText("##NameInput", buffer, sizeof(buffer)))
        {
            component.name = std::string(buffer);
            pool->SetBit<UPDATE_BIT>(entity);
        }

        // Tag
        std::string bufferTag = component.tag;
        memset(buffer, 0, sizeof(buffer));
        strcpy_s(buffer, sizeof(buffer), bufferTag.c_str());

        DrawLabel("Tag");
        if (ImGui::InputText("##TagInput", buffer, sizeof(buffer)))
        {
            component.tag = std::string(buffer);
            pool->SetBit<UPDATE_BIT>(entity);
        }
    }

    if (!visible)
        registry->RemoveComponents<TagComponent>(entity);

    visible = true;
}

void ComponentWindow::RenderTransformComponent(std::shared_ptr<Registry> registry, Entity entity)
{
    auto& component = registry->GetComponent<TransformComponent>(entity);
    auto pool = registry->GetPool<TransformComponent>();
    static bool visible = true;

    if (ImGui::CollapsingHeader(TITLE_CP("Transform Component"), &visible, ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2{ 1, 3 });
        if (ImGui::BeginTable("TransformTable", 4, ImGuiTableFlags_NoBordersInBody | ImGuiTableFlags_SizingStretchSame))
        {
            ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, 60.0f);
            ImGui::TableSetupColumn("X");
            ImGui::TableSetupColumn("Y");
            ImGui::TableSetupColumn("Z");

            auto onUpdate = [&]() { pool->SetBit<UPDATE_BIT>(entity); };

            DrawVec3Control("Trans", component.translation, 0.0f, onUpdate);
            DrawVec3Control("Rot", component.rotation, 0.0f, onUpdate);
            DrawVec3Control("Scale", component.scale, 1.0f, onUpdate);

            ImGui::EndTable();
        }
        ImGui::PopStyleVar();
    }

    if (!visible)
        registry->RemoveComponents<TransformComponent>(entity);

    visible = true;
}

void ComponentWindow::RenderCameraComponent(std::shared_ptr<Registry> registry, Entity entity)
{
    auto& component = registry->GetComponent<CameraComponent>(entity);
    auto pool = registry->GetPool<CameraComponent>();
    static bool visible = true;

    if (ImGui::CollapsingHeader(TITLE_CP("Camera Component"), &visible, ImGuiTreeNodeFlags_DefaultOpen))
    {
        // Is Main
        DrawLabel("Is Main");
        if (ImGui::Checkbox("##IsMainCamera", &component.isMain))
            pool->SetBit<UPDATE_BIT>(entity);

        // FOV
        DrawLabel("FOV");
        if (ImGui::DragFloat("##CameraFOV", &component.fov, 0.1f, 1.0f, 180.0f))
            pool->SetBit<UPDATE_BIT>(entity);

        // Near Plane
        DrawLabel("Near Plane");
        if (ImGui::DragFloat("##CameraNear", &component.nearPlane, 0.01f, 0.01f, 1000.0f))
            pool->SetBit<UPDATE_BIT>(entity);

        // Far Plane
        DrawLabel("Far Plane");
        if (ImGui::DragFloat("##CameraFar", &component.farPlane, 0.1f, 0.01f, 10000.0f))
            pool->SetBit<UPDATE_BIT>(entity);

        // Speed
        DrawLabel("Speed");
        if (ImGui::DragFloat("##CameraSpeed", &component.speed, 0.05f))
            pool->SetBit<UPDATE_BIT>(entity);

        // Sensitivity
        DrawLabel("Sensitivity");
        if (ImGui::DragFloat("##CameraSens", &component.sensitivity, 0.01f))
            pool->SetBit<UPDATE_BIT>(entity);

        // Yaw / Pitch (Manual override)
        DrawLabel("Yaw");
        if (ImGui::DragFloat("##CameraYaw", &component.yaw, 0.1f))
            pool->SetBit<UPDATE_BIT>(entity);

        DrawLabel("Pitch");
        if (ImGui::DragFloat("##CameraPitch", &component.pitch, 0.1f))
            pool->SetBit<UPDATE_BIT>(entity);
    }

    if (!visible)
        registry->RemoveComponents<CameraComponent>(entity);

    visible = true;
}

void ComponentWindow::RenderShapeComponent(std::shared_ptr<Registry> registry, Entity entity)
{
    auto& component = registry->GetComponent<ShapeComponent>(entity);
    auto pool = registry->GetPool<ShapeComponent>();
    static bool visible = true;

    if (ImGui::CollapsingHeader(TITLE_CP("Shape Component"), &visible, ImGuiTreeNodeFlags_DefaultOpen))
    {
        DrawLabel("Shape Asset");
        std::string shapeName = (component.shape) ? "component.shape->GetName()" : "None";
        ImGui::TextDisabled("%s", shapeName.c_str());

        if (ImGui::Button("Change Shape (N/A)", ImVec2(ImGui::GetContentRegionAvail().x, 0)))
        {
            // TODO: Asset browser megnyitása
        }
    }

    if (!visible)
        registry->RemoveComponents<ShapeComponent>(entity);

    visible = true;
}

void ComponentWindow::RenderModelComponent(std::shared_ptr<Registry> registry, Entity entity)
{
    auto& component = registry->GetComponent<ModelComponent>(entity);
    auto pool = registry->GetPool<ModelComponent>();
    static bool visible = true;

    if (ImGui::CollapsingHeader(TITLE_CP("Model Component"), &visible, ImGuiTreeNodeFlags_DefaultOpen))
    {
        DrawLabel("DX Normals");
        if (ImGui::Checkbox("##DXNormals", &component.hasDirectxNormals))
            pool->SetBit<UPDATE_BIT>(entity);

        DrawLabel("Model Asset");
        std::string modelName = (component.model) ? "Loaded" : "None";
        ImGui::TextDisabled("%s", modelName.c_str());

        if (ImGui::Button("Change Model (N/A)", ImVec2(ImGui::GetContentRegionAvail().x, 0)))
        {
            // TODO: Asset browser
        }
    }

    if (!visible)
        registry->RemoveComponents<ModelComponent>(entity);

    visible = true;
}

void ComponentWindow::RenderMaterialComponent(std::shared_ptr<Registry> registry, Entity entity)
{
    auto& component = registry->GetComponent<MaterialComponent>(entity);
    static bool visible = true;

    if (ImGui::CollapsingHeader(TITLE_CP("Material Component"), &visible, ImGuiTreeNodeFlags_DefaultOpen))
    {
        DrawLabel("Material Asset");
        std::string matName = (component.material) ? "Assigned" : "None";
        ImGui::TextDisabled("%s", matName.c_str());

        if (ImGui::Button("Change Material (N/A)", ImVec2(ImGui::GetContentRegionAvail().x, 0)))
        {
            // TODO: Asset browser integration
        }

        if (component.material)
        {
            ImGui::Separator();
            ImGui::Text("Properties");
            ImGui::Spacing();

            auto& material = component.material;
            bool changed = false;

            DrawLabel("Color");
            if (ImGui::ColorEdit4("##MatColor", glm::value_ptr(material->color)))
                changed = true;

            DrawLabel("UV Scale");
            if (ImGui::DragFloat2("##MatUV", glm::value_ptr(material->uvScale), 0.05f))
                changed = true;

            DrawLabel("Metalness");
            if (ImGui::SliderFloat("##MatMetal", &material->metalness, 0.0f, 1.0f))
                changed = true;

            DrawLabel("Roughness");
            if (ImGui::SliderFloat("##MatRough", &material->roughness, 0.0f, 1.0f))
                changed = true;

            DrawLabel("AO Strength");
            if (ImGui::DragFloat("##MatAO", &material->aoStrength, 0.01f, 0.0f, 1.0f))
                changed = true;

            // Emissive Settings
            DrawLabel("Emissive Color");
            if (ImGui::ColorEdit3("##MatEmissiveCol", glm::value_ptr(material->emissiveColor)))
                changed = true;

            DrawLabel("Emissive Int.");
            if (ImGui::DragFloat("##MatEmissiveInt", &material->emissiveIntensity, 0.1f, 0.0f, 100.0f))
                changed = true;

            DrawLabel("Use Bloom");
            if (ImGui::Checkbox("##MatBloom", &material->useBloom))
                changed = true;

            if (changed)
            {
                material->SetBit<UPDATE_BIT>();
            }
        }
    }

    if (!visible)
        registry->RemoveComponents<MaterialComponent>(entity);

    visible = true;
}

void ComponentWindow::RenderAnimationComponent(std::shared_ptr<Registry> registry, Entity entity)
{
    auto& component = registry->GetComponent<AnimationComponent>(entity);
    auto pool = registry->GetPool<AnimationComponent>();
    static bool visible = true;

    if (ImGui::CollapsingHeader(TITLE_CP("Animation Component"), &visible, ImGuiTreeNodeFlags_DefaultOpen))
    {
        // Speed
        DrawLabel("Speed");
        if (ImGui::DragFloat("##AnimSpeed", &component.speed, 0.05f, 0.0f, 5.0f))
            pool->SetBit<UPDATE_BIT>(entity);

        // Time (Read-only info)
        DrawLabel("Time");
        float time = (float)component.time;
        ImGui::Text("%.2f", time);

        // Asset placeholder
        DrawLabel("Animation");
        std::string animName = (component.animation) ? "Loaded" : "None";
        ImGui::TextDisabled("%s", animName.c_str());
    }

    if (!visible)
        registry->RemoveComponents<AnimationComponent>(entity);

    visible = true;
}

void ComponentWindow::RenderDirectionLightComponent(std::shared_ptr<Registry> registry, Entity entity)
{
    auto& component = registry->GetComponent<DirectionLightComponent>(entity);
    auto pool = registry->GetPool<DirectionLightComponent>();
    static bool visible = true;

    if (ImGui::CollapsingHeader(TITLE_CP("Direction Light"), &visible, ImGuiTreeNodeFlags_DefaultOpen))
    {
        // Color
        DrawLabel("Color");
        if (ImGui::ColorEdit3("##DirLightColor", glm::value_ptr(component.color)))
            pool->SetBit<UPDATE_BIT>(entity);

        // Direction
        DrawLabel("Direction");
        if (ImGui::DragFloat3("##DirLightDirection", glm::value_ptr(component.direction), 0.01f, -1.0f, 1.0f))
            pool->SetBit<UPDATE_BIT>(entity);

        // Strength
        DrawLabel("Strength");
        if (ImGui::DragFloat("##DirLightStrength", &component.strength, 0.1f, 0.0f, 100.0f))
            pool->SetBit<UPDATE_BIT>(entity);

        ImGui::SeparatorText("Shadow Settings");
        ImGui::TextDisabled("Shadow properties are managed by system.");
    }

    if (!visible)
        registry->RemoveComponents<DirectionLightComponent>(entity);

    visible = true;
}

void ComponentWindow::RenderPointLightComponent(std::shared_ptr<Registry> registry, Entity entity)
{
    auto& component = registry->GetComponent<PointLightComponent>(entity);
    auto pool = registry->GetPool<PointLightComponent>();
    static bool visible = true;

    if (ImGui::CollapsingHeader(TITLE_CP("Point Light"), &visible, ImGuiTreeNodeFlags_DefaultOpen))
    {
        // Color
        DrawLabel("Color");
        if (ImGui::ColorEdit3("##PointLightColor", glm::value_ptr(component.color)))
            pool->SetBit<UPDATE_BIT>(entity);

        // Position
        DrawLabel("Position");
        if (ImGui::DragFloat3("##PointLightPos", glm::value_ptr(component.position), 0.1f))
            pool->SetBit<UPDATE_BIT>(entity);

        // Strength
        DrawLabel("Strength");
        if (ImGui::DragFloat("##PointLightStrength", &component.strength, 0.1f, 0.0f, 1000.0f))
            pool->SetBit<UPDATE_BIT>(entity);

        // Radius
        DrawLabel("Radius");
        if (ImGui::DragFloat("##PointLightRadius", &component.radius, 0.1f, 0.0f, 100.0f))
            pool->SetBit<UPDATE_BIT>(entity);

        // Weaken Distance
        DrawLabel("Weaken Dist");
        if (ImGui::DragFloat("##PointLightWeaken", &component.weakenDistance, 0.1f, 0.0f, 100.0f))
            pool->SetBit<UPDATE_BIT>(entity);
    }

    if (!visible)
        registry->RemoveComponents<PointLightComponent>(entity);

    visible = true;
}

void ComponentWindow::RenderSpotLightComponent(std::shared_ptr<Registry> registry, Entity entity)
{
    auto& component = registry->GetComponent<SpotLightComponent>(entity);
    auto pool = registry->GetPool<SpotLightComponent>();
    static bool visible = true;

    if (ImGui::CollapsingHeader(TITLE_CP("Spot Light"), &visible, ImGuiTreeNodeFlags_DefaultOpen))
    {
        // Color
        DrawLabel("Color");
        if (ImGui::ColorEdit3("##SpotLightColor", glm::value_ptr(component.color)))
            pool->SetBit<UPDATE_BIT>(entity);

        // Position
        DrawLabel("Position");
        if (ImGui::DragFloat3("##SpotLightPos", glm::value_ptr(component.position), 0.1f))
            pool->SetBit<UPDATE_BIT>(entity);

        // Direction
        DrawLabel("Direction");
        if (ImGui::DragFloat3("##SpotLightDir", glm::value_ptr(component.direction), 0.01f, -1.0f, 1.0f))
            pool->SetBit<UPDATE_BIT>(entity);

        // Strength
        DrawLabel("Strength");
        if (ImGui::DragFloat("##SpotLightStr", &component.strength, 0.1f, 0.0f, 100.0f))
            pool->SetBit<UPDATE_BIT>(entity);

        // Range
        DrawLabel("Range");
        if (ImGui::DragFloat("##SpotLightRange", &component.range, 0.1f, 0.0f, 200.0f))
            pool->SetBit<UPDATE_BIT>(entity);

        // Inner Angle
        DrawLabel("Inner Angle");
        if (ImGui::DragFloat("##SpotLightInner", &component.angles.x, 0.1f, 0.0f, 90.0f))
            pool->SetBit<UPDATE_BIT>(entity);

        // Outer Angle
        DrawLabel("Outer Angle");
        if (ImGui::DragFloat("##SpotLightOuter", &component.angles.y, 0.1f, 0.0f, 90.0f))
            pool->SetBit<UPDATE_BIT>(entity);
    }

    if (!visible)
        registry->RemoveComponents<SpotLightComponent>(entity);

    visible = true;
}

void ComponentWindow::RenderAddComponentPopUp(std::shared_ptr<Registry> registry, Entity entity)
{
    if (ImGui::BeginPopupContextItem("AddComponentPopup") || ImGui::BeginPopup("AddComponentPopup"))
    {
        ImGui::SeparatorText("General");

        {
            bool has = registry->HasComponent<TagComponent>(entity);
            if (ImGui::MenuItem("Tag Component", nullptr, has, !has))
                registry->AddComponents<TagComponent>(entity);
        }

        {
            bool has = registry->HasComponent<TransformComponent>(entity);
            if (ImGui::MenuItem("Transform Component", nullptr, has, !has))
                registry->AddComponents<TransformComponent>(entity);
        }

        {
            bool has = registry->HasComponent<CameraComponent>(entity);
            if (ImGui::MenuItem("Camera Component", nullptr, has, !has))
                registry->AddComponents<CameraComponent>(entity);
        }

        ImGui::SeparatorText("Render Objects");

        {
            bool has = registry->HasComponent<ShapeComponent>(entity);
            if (ImGui::MenuItem("Shape Component", nullptr, has, !has))
                registry->AddComponents<ShapeComponent>(entity);
        }

        {
            bool has = registry->HasComponent<ModelComponent>(entity);
            if (ImGui::MenuItem("Model Component", nullptr, has, !has))
                registry->AddComponents<ModelComponent>(entity);
        }

        {
            bool has = registry->HasComponent<MaterialComponent>(entity);
            if (ImGui::MenuItem("Material Component", nullptr, has, !has))
                registry->AddComponents<MaterialComponent>(entity);
        }

        {
            bool has = registry->HasComponent<AnimationComponent>(entity);
            if (ImGui::MenuItem("Animation Component", nullptr, has, !has))
                registry->AddComponents<AnimationComponent>(entity);
        }

        ImGui::SeparatorText("Lights");

        {
            bool has = registry->HasComponent<DirectionLightComponent>(entity);
            if (ImGui::MenuItem("Direction Light", nullptr, has, !has))
                registry->AddComponents<DirectionLightComponent>(entity);
        }

        {
            bool has = registry->HasComponent<PointLightComponent>(entity);
            if (ImGui::MenuItem("Point Light", nullptr, has, !has))
                registry->AddComponents<PointLightComponent>(entity);
        }

        {
            bool has = registry->HasComponent<SpotLightComponent>(entity);
            if (ImGui::MenuItem("Spot Light", nullptr, has, !has))
                registry->AddComponents<SpotLightComponent>(entity);
        }

        ImGui::EndPopup();
    }
}