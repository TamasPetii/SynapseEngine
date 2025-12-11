#include "EntityWindow.h"
#include "Editor/Gui/Utils/Panel.h"
#include "IconsFontAwesome5.h"

#include "Engine/Components/TagComponent.h"
#include "Engine/Components/TransformComponent.h"
#include "Engine/Components/CameraComponent.h"
#include "Engine/Components/ModelComponent.h"
#include "Engine/Components/ShapeComponent.h"
#include "Engine/Components/MaterialComponent.h"
#include "Engine/Components/DirectionLightComponent.h"
#include "Engine/Components/PointLightComponent.h"
#include "Engine/Components/SpotLightComponent.h"
#include "Engine/Components/AnimationComponent.h"

#define TITLE_EW(title) (std::string(title) + "##EntityWindow").c_str()

void EntityWindow::Render(std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, std::set<VkDescriptorSet>& textureSet, uint32_t frameIndex)
{
	static Window::Config windowConfig{
		.name = "Scene Hierarchy",
	};

	Window::RenderWindow(windowConfig, [&]() -> void
		{
			if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && ImGui::IsWindowHovered())
			{
				registry->SetActiveEntity(NULL_ENTITY);
			}

			if (ImGui::BeginPopupContextWindow(TITLE_EW("HierarchyContext"), ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
			{
				RenderContextMenu(registry, resourceManager);
				ImGui::EndPopup();
			}

			ImVec2 avail = ImGui::GetContentRegionAvail();
			if (avail.y < 50) avail.y = 50;

			auto& rootEntities = registry->GetLevels()[0];
			for (auto entity : rootEntities)
			{
				DrawEntityNode(registry, entity);
			}

			ImGui::Dummy(ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y));
			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY_DRAG"))
				{
					Entity droppedEntity = *(Entity*)payload->Data;
					if (droppedEntity != NULL_ENTITY)
					{
						registry->SetParent(droppedEntity, NULL_ENTITY); //
					}
				}
				ImGui::EndDragDropTarget();
			}

			for (auto entity : m_EntitiesToDelete)
			{
				if (registry->GetActiveEntity() == entity)
					registry->SetActiveEntity(NULL_ENTITY);
				registry->DestroyEntity(entity);
			}
			m_EntitiesToDelete.clear();

			for (auto entity : m_EntitiesToDuplicate)
			{
				/*
				Entity newEntity = registry->CreateEntity();
				if (registry->HasComponent<TagComponent>(entity))
				{
					std::string oldName = registry->GetComponent<TagComponent>(entity).name;
					registry->AddComponents<TagComponent>(newEntity, { oldName + " (Copy)" });
				}
				registry->AddComponents<TransformComponent>(newEntity);
				// Itt kellene a többi komponenst átmásolni...
				*/
			}
			m_EntitiesToDuplicate.clear();
		});
}

void EntityWindow::DrawEntityNode(std::shared_ptr<Registry> registry, Entity entity)
{
	std::string name = "Entity " + std::to_string(entity);
	if (registry->HasComponent<TagComponent>(entity))
	{
		auto& tag = registry->GetComponent<TagComponent>(entity);
		if (!tag.name.empty()) name = tag.name;
	}

	std::string icon = GetEntityIcon(registry, entity);
	std::string label = icon + " " + name;

	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;

	bool isSelected = (registry->GetActiveEntity() == entity);
	if (isSelected)
	{
		flags |= ImGuiTreeNodeFlags_Selected;
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.8f, 0.0f, 1.0f)); // Arany szín a kiválasztottnak
	}

	auto& relationship = registry->GetComponent<Relationship>(entity);
	if (relationship.children.empty())
	{
		flags |= ImGuiTreeNodeFlags_Leaf;
	}

	bool opened = ImGui::TreeNodeEx((void*)(uint64_t)entity, flags, label.c_str());

	if (isSelected)
		ImGui::PopStyleColor();

	if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
	{
		registry->SetActiveEntity(entity);
	}

	if (ImGui::BeginPopupContextItem())
	{
		if (ImGui::MenuItem(TITLE_EW(std::string(ICON_FA_TRASH) + " Delete")))
		{
			m_EntitiesToDelete.push_back(entity);
		}
		if (ImGui::MenuItem(TITLE_EW(std::string(ICON_FA_COPY) + " Duplicate")))
		{
			m_EntitiesToDuplicate.push_back(entity);
		}
		ImGui::Separator();
		if (ImGui::BeginMenu(TITLE_EW("Add Child")))
		{
			if (ImGui::MenuItem("Empty Child"))
			{
				Entity child = registry->CreateEntity();
				registry->SetParent(child, entity);
				registry->AddComponents<TagComponent>(child, { "Child" });
				registry->AddComponents<TransformComponent>(child);
			}
			ImGui::EndMenu();
		}

		ImGui::EndPopup();
	}

	if (ImGui::BeginDragDropSource())
	{
		ImGui::SetDragDropPayload("ENTITY_DRAG", &entity, sizeof(Entity));
		ImGui::Text("%s %s", icon.c_str(), name.c_str()); // Preview
		ImGui::EndDragDropSource();
	}

	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY_DRAG"))
		{
			Entity droppedEntity = *(Entity*)payload->Data;

			if (droppedEntity != entity && !registry->IsDeepConnected(entity, droppedEntity))
			{
				registry->SetParent(droppedEntity, entity);
			}
		}
		ImGui::EndDragDropTarget();
	}

	if (opened)
	{
		for (auto child : relationship.children)
		{
			DrawEntityNode(registry, child);
		}
		ImGui::TreePop();
	}
}

std::string EntityWindow::GetEntityIcon(std::shared_ptr<Registry> registry, Entity entity)
{
	if (registry->HasComponent<CameraComponent>(entity)) return ICON_FA_VIDEO;
	if (registry->HasComponent<DirectionLightComponent>(entity)) return ICON_FA_SUN;
	if (registry->HasComponent<PointLightComponent>(entity)) return ICON_FA_LIGHTBULB;
	if (registry->HasComponent<SpotLightComponent>(entity)) return ICON_FA_LIGHTBULB;
	if (registry->HasComponent<AnimationComponent>(entity)) return ICON_FA_RUNNING;
	if (registry->HasComponent<ShapeComponent>(entity)) return ICON_FA_SHAPES;
	if (registry->HasComponent<ModelComponent>(entity)) return ICON_FA_CUBE;

	return ICON_FA_CUBE;
}

void EntityWindow::RenderContextMenu(std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager)
{
	if (ImGui::MenuItem(TITLE_EW(std::string(ICON_FA_CUBE) + " Empty Entity")))
	{
		auto entity = registry->CreateEntity();
		registry->AddComponents<TagComponent>(entity, { "Empty Entity" });
		registry->AddComponents<TransformComponent>(entity);
		registry->SetActiveEntity(entity);
	}

	ImGui::Separator();

	if (ImGui::MenuItem(TITLE_EW(std::string(ICON_FA_VIDEO) + " Camera")))
	{
		auto entity = registry->CreateEntity();
		registry->AddComponents<TagComponent>(entity, { "Camera" });
		registry->AddComponents<TransformComponent>(entity);
		registry->AddComponents<CameraComponent>(entity);
		registry->SetActiveEntity(entity);
	}

	if (ImGui::BeginMenu(TITLE_EW(std::string(ICON_FA_SHAPES) + " Shapes")))
	{
		const char* shapes[] = { "Cube", "Sphere", "Cylinder", "Cone", "Plane", "Torus" };
		for (const char* shapeName : shapes)
		{
			if (ImGui::MenuItem(shapeName))
			{
				auto entity = registry->CreateEntity();
				registry->AddComponents<TagComponent>(entity, { shapeName });
				registry->AddComponents<TransformComponent>(entity);
				registry->AddComponents<ShapeComponent>(entity);
				registry->AddComponents<MaterialComponent>(entity);

				registry->GetComponent<ShapeComponent>(entity).shape = resourceManager->GetGeometryManager()->GetShape(shapeName);

				registry->SetActiveEntity(entity);
			}
		}
		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu(TITLE_EW(std::string(ICON_FA_LIGHTBULB) + " Lights")))
	{
		if (ImGui::MenuItem("Directional Light"))
		{
			auto entity = registry->CreateEntity();
			registry->AddComponents<TagComponent>(entity, { "Directional Light" });
			registry->AddComponents<TransformComponent>(entity);
			registry->AddComponents<DirectionLightComponent>(entity);
			registry->SetActiveEntity(entity);
		}
		if (ImGui::MenuItem("Point Light"))
		{
			auto entity = registry->CreateEntity();
			registry->AddComponents<TagComponent>(entity, { "Point Light" });
			registry->AddComponents<TransformComponent>(entity);
			registry->AddComponents<PointLightComponent>(entity);
			registry->SetActiveEntity(entity);
		}
		if (ImGui::MenuItem("Spot Light"))
		{
			auto entity = registry->CreateEntity();
			registry->AddComponents<TagComponent>(entity, { "Spot Light" });
			registry->AddComponents<TransformComponent>(entity);
			registry->AddComponents<SpotLightComponent>(entity);
			registry->SetActiveEntity(entity);
		}
		ImGui::EndMenu();
	}
}