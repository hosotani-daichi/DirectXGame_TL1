#pragma once
#include <3d/Camera.h>
#include <KamataEngine.h>
#include <map>

class GameScene {

public:

	GameScene();
	~GameScene();
	void Initialize();
	void Update();
	void Draw();

	// オブジェクト一個分のデータ
	struct ObjectData {
		std::string type; //"type"
		std::string name; //"name"

		//"transform"
		struct Transform {
			KamataEngine::Vector3 translation; //"translation"
			KamataEngine::Vector3 rotation;    //"rotation"
			KamataEngine::Vector3 scaling;     //"scaling"
		};

		Transform transform; // メンバの準備

		//"file name"
		std::string file_name;
	};

	// レベルデータ
	struct LevelData {
		//"name"
		std::string name;

		//"object"
		std::vector<ObjectData> objects;
	};

private:

	std::map<std::string, KamataEngine::Model*> models;
	std::vector<KamataEngine::WorldTransform*> worldTransforms;
	LevelData* levelData = nullptr;
	KamataEngine::Camera camera_;
};