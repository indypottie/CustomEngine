#include "Scene.h"

#include <algorithm>
#include <iostream>
#include <stdexcept>

#include "InputManager.h"

using namespace dae;

unsigned int Scene::m_idCounter = 0;

Scene::Scene(std::string name) : m_name(std::move(name))
{
}

Scene::~Scene() = default;

void Scene::Add(std::unique_ptr<GameObject> object)
{
	assert(object && "Trying to add a null GameObject!");

	const int newLayer = object->GetRenderLayer();

	// Find the first object with a higher layer
	const auto it = std::ranges::find_if(m_objectsToAdd
	                               ,
	                               [newLayer](const std::unique_ptr<GameObject>& obj)
	                               {
		                               return obj->GetRenderLayer() > newLayer;
	                               });

	// Insert before it
	m_objectsToAdd.insert(it, std::move(object));
}

void Scene::Remove(const std::unique_ptr<GameObject>& object)
{
	std::erase(m_objects, object);
}

void Scene::RemoveAll()
{
	for (auto& object : m_objects)
	{
		object.get()->MarkForDeletion();
	}
}

void Scene::Update()
{
	for (const auto& object : m_objects)
	{
		if (object && !object->IsMarkedForDeletion())
			object->Update();
	}

	for (auto& objectToAdd : m_objectsToAdd)
	{
		m_objects.emplace_back(std::move(objectToAdd));
	}

	m_objectsToAdd.clear();

	std::erase_if(m_objects,
		[](std::unique_ptr<GameObject>& obj)
		{
			if (obj && obj->IsMarkedForDeletion())
			{
				obj = nullptr;
				return true;
			}
			return false;
		});
}

void Scene::Render() const
{
	for (const auto& object : m_objects)
	{
		object->Render();
	}
}

GameObject* Scene::FindObjectByTag(const std::string& tag)
{
	for (const auto& object : m_objects)
	{
		if (object->GetTag() == tag)
			return object.get();
	}
	return nullptr;
}

std::vector<GameObject*> Scene::FindObjectsByTag(const std::string& tag)
{
	std::vector<GameObject*> result;

	for (const auto& object : m_objects)
	{
		if (object->GetTag() == tag)
		{
			result.push_back(object.get());
		}
	}

	return result;
}

