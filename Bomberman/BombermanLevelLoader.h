#pragma once


//-----------------------------------------------------
// Include Files
//-----------------------------------------------------
#include <string>

class GridComponent;

//-----------------------------------------------------
// BombermanLevelLoader Class									 
//-----------------------------------------------------
class BombermanLevelLoader final
{
public:
	static bool LoadLevelFromFile(const std::string& filePath, GridComponent& environmentGrid);
};
