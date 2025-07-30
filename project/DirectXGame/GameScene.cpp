#include "GameScene.h"

#include <cassert>
#include <fstream>
#include <json.hpp>
#include <string>
#include <vector>

using namespace KamataEngine;

GameScene::GameScene() {}

GameScene::~GameScene() {}

void GameScene::Initialize() {

	//-------------------------------------------------------------------//
	// レベルデータを格納する為の構造体
	//-------------------------------------------------------------------//

	//-------------------------------------------------------------------//
	// Jsonファイルのデシリアライズ化
	//-------------------------------------------------------------------//

	// jsonファイルのパス名
	const std::string fullpath = std::string("Resources/levels/") + "scene.json";

	// ファイルストリーム
	std::ifstream file;

	// ファイルを開く
	file.open(fullpath);
	// ファイルオープン失敗をチェック
	if (file.fail()) {
		assert(0);
	}

	nlohmann::json deserialized; // deserialized : 逆シリアライズ化
	                             // →1つの文字列をメモリ中のデータ構造化すること
	                             // serialize : 一列に並べる操作のこと
	                             // →１つの文字列で表現する「直列化」のこと

	// ファイルから読み込み、メモリへ格納
	file >> deserialized;

	// 正しいレベルデータファイルかチェック
	assert(deserialized.is_object());         // objectか※json形式にはさまざまな型がある
	                                          // object型はその中でも「キーと値のペアを持つ構造」つまり連想配列が扱えるか聞いている
	assert(deserialized.contains("name"));    //"name"が含まれているか
	assert(deserialized["name"].is_string()); //["name"]は文字列か？

	//-------------------------------------------------------------------//
	// レベルデータを構造体に格納していく
	//-------------------------------------------------------------------//

	levelData = new LevelData();

	//"name"を文字列として取得
	levelData->name = deserialized["name"].get<std::string>();
	assert(levelData->name == "scene"); // それは"scene"か?

	//"objects"の全オブジェクトを走査
	for (nlohmann::json& object : deserialized["objects"]) {
		// オブジェクト１つ分の妥当性のチェック
		assert(object.contains("type")); //"type"が含まれているか

		if (object["type"].get<std::string>() == "MESH") {
			// １個分の要素の準備
			levelData->objects.emplace_back(ObjectData{});
			ObjectData& objectData = levelData->objects.back();

			objectData.type = object["type"].get<std::string>(); //"type"
			objectData.type = object["name"].get<std::string>(); //"name"

			// トランスフォームのパラメーター読み込み
			nlohmann::json& transform = object["transform"];

			// 平行移動"transform"
			objectData.transform.translation.x = (float)transform["translation"][0];
			objectData.transform.translation.y = (float)transform["translation"][2];
			objectData.transform.translation.z = (float)transform["translation"][1];

			// 回転角"rotation"
			objectData.transform.rotation.x = -(float)transform["rotation"][0];
			objectData.transform.rotation.y = -(float)transform["rotation"][2];
			objectData.transform.rotation.z = -(float)transform["rotation"][1];

			// 拡大縮小"scaling"
			objectData.transform.scaling.x = (float)transform["scaling"][0];
			objectData.transform.scaling.y = (float)transform["scaling"][2];
			objectData.transform.scaling.z = (float)transform["scaling"][1];

			// TODO: オブジェクト走査を再起関数にまとめ、再起呼び出しで枝を走査する
			if (object.contains("children")) {
			}

			//"file name"
			if (object.contains("file_name")) {
				objectData.file_name = object["file_name"].get<std::string>();
			}
		}
	}
	//-------------------------------------------------------------------//
	// レベルデータからオブジェクトを生成、配置
	//-------------------------------------------------------------------//
	for (auto& objectData_ : levelData->objects) {
		// モデルファイル名
		Model* model = nullptr;
		decltype(models)::iterator it = models.find(objectData_.file_name);
		if (it != models.end()) {
			model = it->second;
		} else {
			model = Model::CreateFromOBJ(objectData_.file_name);
			models[objectData_.file_name] = model;
		}

		// モデルを指定して3Dオブジェクトを生成
		WorldTransform* newObject = new WorldTransform();

		// 位置の設定
		newObject->translation_ = objectData_.transform.translation;

		// 回転の設定
		newObject->rotation_ = objectData_.transform.rotation;

		// 拡大縮小
		newObject->scale_ = objectData_.transform.scaling;

		newObject->Initialize();

		// 配列に登録
		worldTransforms.push_back(newObject);
	}

	camera_.Initialize();
}

void GameScene::Update() {

    for (WorldTransform* object : worldTransforms) {
		using namespace KamataEngine::MathUtility;

		Matrix4x4 scale = MakeScaleMatrix(object->scale_);

		Matrix4x4 rotateX = MakeRotateXMatrix(object->rotation_.x);
		Matrix4x4 rotateY = MakeRotateYMatrix(object->rotation_.y);
		Matrix4x4 rotateZ = MakeRotateZMatrix(object->rotation_.z);
		Matrix4x4 rotate = rotateZ * rotateY * rotateX; // 順番は調整可能

		Matrix4x4 translate = MakeTranslateMatrix(object->translation_);

		object->matWorld_ = scale * rotate * translate;
		object->TransferMatrix();
	}
}

void GameScene::Draw() {

	Model::PreDraw();

	int i = 0;

	for (auto& objectData_ : levelData->objects) {
		models[objectData_.file_name]->Draw(*worldTransforms[i], camera_);
		i++;
	}

	Model::PostDraw();
}