#include "EntityWindow.h"

void EntityWindow::Render(std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, std::set<VkDescriptorSet>& textureSet, uint32_t frameIndex)
{
	static Window::Config windowConfig{
		.name = "Entity-Window"
	};

	Window::RenderWindow(windowConfig, [&]() -> void
		{
			for (auto entity : registry->GetLevels()[0])
				DisplayEntity(registry, entity);
		}
	);
}

void EntityWindow::DisplayEntity(std::shared_ptr<Registry> registry, Entity entity)
{
	if (registry->GetActiveEntity() == entity)
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 0, 1));

	bool open = ImGui::TreeNodeEx(std::to_string(entity).c_str(), ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick);

	if (registry->GetActiveEntity() == entity)
		ImGui::PopStyleColor();

	if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
		registry->SetActiveEntity(entity);

	if (ImGui::BeginDragDropSource())
	{
		ImGui::SetDragDropPayload("Entity_Tree", &entity, sizeof(Entity));
		ImGui::Text("Entity | %d", entity);
		ImGui::EndDragDropSource();
	}

	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Entity_Tree"))
		{
			Entity droppedEntity = *(Entity*)payload->Data;

			if (droppedEntity != NULL_ENTITY && !registry->IsDeepConnected(entity, droppedEntity))
				registry->SetParent(droppedEntity, entity);
		}

		ImGui::EndDragDropTarget();
	}


	if (open)
	{
		for (auto child : registry->GetComponent<Relationship>(entity).children)
			DisplayEntity(registry, child);

		ImGui::TreePop();
	}
}
