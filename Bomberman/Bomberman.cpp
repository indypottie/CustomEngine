#include <SDL.h>

#if _DEBUG
#if __has_include(<vld.h>)
#include <vld.h>
#endif
#endif

#include "Minigin.h"
#include "PrefabFactory.h"
#include "ResourceManager.h"
#include "SceneManager.h"

void load()
{
	auto& scene = dae::SceneManager::GetInstance().CreateScene("BomberMan");

	Prefab::CreateStartMenu(scene);
}

int main(int, char*[])
{
	dae::Minigin engine("../Data/");
	engine.Run(load);
	return 0;
}