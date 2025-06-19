#pragma once
#include "SceneManager.h"
#include "gameObject.h"

namespace dae
{
	class GameObject;

	class Scene final
	{
		friend Scene& SceneManager::CreateScene(const std::string& name);
	public:
		void Add(std::unique_ptr<GameObject> object);
		void Remove(const std::unique_ptr<GameObject>& object);
		void RemoveAll();

		void Update();
		void Render() const;

		GameObject* FindObjectByTag(const std::string& tag);

		std::vector<GameObject*> FindObjectsByTag(const std::string& tag);

		~Scene();
		Scene(const Scene& other) = delete;
		Scene(Scene&& other) = delete;
		Scene& operator=(const Scene& other) = delete;
		Scene& operator=(Scene&& other) = delete;

	private: 
		explicit Scene(std::string name);

		std::string m_name;
		std::vector < std::unique_ptr<GameObject>> m_objects{};
		std::vector < std::unique_ptr<GameObject>> m_objectsToAdd{};

		static unsigned int m_idCounter;
	};

}
