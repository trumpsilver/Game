﻿#include "TestScene.h"

TestScene::TestScene()
{
	_viewport = new Viewport(0, WINDOW_HEIGHT, WINDOW_WIDTH, WINDOW_HEIGHT);
}

TestScene::~TestScene()
{
	delete _viewport;
	_viewport = nullptr;
}

void TestScene::setViewport(Viewport * viewport)
{
	if (_viewport != viewport)
		_viewport = viewport;
}

void TestScene::moveViewport(float offset, bool moveup, sigcxx::SLOT slot)
{
	GVector2 position = _viewport->getPositionWorld();
	if (moveup == true)
	{
		position.y += offset;
		_updateViewport = false;
	}
	else
	{
		position.y -= offset;
		_updateViewport = true;
	}
	_viewport->setPositionWorld(position);
}

void TestScene::writeXMLQuadTree()
{
	_root->writeXML("Resources//Maps//stage1_quadtree.xml");
}

bool TestScene::init()
{

	_Aladdin = ObjectFactory::getAladdin();
	_Aladdin->getPhysicsComponent()->setPosition(200, 200);

	auto aladdinBehavior = (AladdinBehaviorComponent*)_Aladdin->getBehaviorComponent();
	aladdinBehavior->setRespawnPosition(GVector2(100, 200));
	aladdinBehavior->moveViewport.Connect(this, &TestScene::moveViewport);
	aladdinBehavior->addToScene.Connect(this, &TestScene::addToScene);
	_listobject.push_back(_Aladdin);

	auto hakim = ObjectFactory::getHakim(GVector2(400, 100),200, 600);
	_listobject.push_back(hakim);



	_mapBack = SpriteResource::getInstance()->getSprite(eObjectID::MAP1);
	_mapBack->setFrameRect(SpriteResource::getInstance()->getSourceRect(eObjectID::MAP1, "back"));
	_mapBack->setPositionX(0);
	_mapBack->setPositionY(0);
	_mapBack->setOrigin(GVector2(0.f, 0.f));
	_mapBack->setScale(SCALE_FACTOR);
	_mapBack->setZIndex(0.f);

	_mapFront = SpriteResource::getInstance()->getSprite(eObjectID::MAP1);
	_mapFront->setFrameRect(SpriteResource::getInstance()->getSourceRect(eObjectID::MAP1, "front"));
	_mapFront->setPositionX(0);
	_mapFront->setPositionY(0);
	_mapFront->setOrigin(GVector2(0.f, 0.f));
	_mapFront->setScale(SCALE_FACTOR);
	_mapFront->setZIndex(0.f);

	RECT mapRECT = SpriteResource::getInstance()->getSourceRect(eObjectID::MAP1, "back");
	rootRect;
	rootRect.left = 0;
	rootRect.bottom = 0;
	rootRect.right = mapRECT.right * SCALE_FACTOR;
	rootRect.top = mapRECT.bottom * SCALE_FACTOR;
	_root = new QuadTreeNode(rootRect, 0);


	map<string, GameObject*>* maptemp = ObjectFactory::getMapObjectFromFile("Resources//Maps//stage1.xml");
	_mapobject.insert(maptemp->begin(), maptemp->end());

	for (auto it = _mapobject.begin(); it != _mapobject.end(); it++)
	{
		_root->insert(it->first, it->second->getPhysicsComponent()->getBounding());
	}

	writeXMLQuadTree();

	_updateViewport = true;
	//SoundManager::getInstance()->PlayLoop(eSoundId::BACKGROUND_STAGE1);
	return true;
}


void TestScene::update(float dt)
{
	updateViewport(_Aladdin, dt);
	GVector2 viewport_position = _viewport->getPositionWorld();
	RECT viewport_in_transform = _viewport->getBounding();

	RECT screen;
	//hệ bot-left
	screen.left = viewport_in_transform.left;
	screen.right = viewport_in_transform.right ;
	screen.top = viewport_position.y;
	screen.bottom = screen.top - _viewport->getHeight();

	// getlistobject

	// [Bước 1]
	this->destroyobject();

	// [Bước 2]
	_active_object.clear();

	// [Bước 3]
	auto listobjectname = _root->getActiveObject(screen);

	// [Bước 4]
	//OutputDebugStringW(L"Object in screen : ");
	for (auto name : listobjectname)
	{
		auto obj = _mapobject.find(name);
		if (obj == _mapobject.end() || obj._Ptr == nullptr)
			continue;
		//OutputDebugStringA(obj->first.c_str());
		//OutputDebugStringW(L" ");
		_active_object.push_back(obj->second);
	}
//	OutputDebugStringW(L"\n ");

	// [Bước 5]
	_active_object.insert(_active_object.end(), _listobject.begin(), _listobject.end());

	// [Bước 6]
	for (GameObject* obj : _active_object)
	{
		// một vài trạng thái không cần thiết phải check hàm va chạm
		if (obj == nullptr || obj->getID() == eObjectID::LAND || obj->getID() == eObjectID::ROPE)
			continue;
		auto collisionComponent = (CollisionComponent*)obj->getPhysicsComponent()->getComponent("Collision");
		collisionComponent->reset();
		for (GameObject* passiveobj : _active_object)
		{
			if (passiveobj == nullptr || passiveobj == obj)
				continue;
			if (collisionComponent != nullptr)
			{
				if (passiveobj->getID() != eObjectID::ROPE) // aladdin can overlap rope so don't update target postion. Let aladdin behavior do it instead
					collisionComponent->checkCollision(passiveobj, dt, false);
				else
					collisionComponent->checkCollision(passiveobj, dt, false);
			}
			
		}
	}
	//[Bước 7]
	for (GameObject* obj : _active_object)
	{
		obj->update(dt);
	}
}

void TestScene::draw(LPD3DXSPRITE spriteHandle)
{
	_mapBack->render(spriteHandle, _viewport);
	for (GameObject* object : _active_object)
	{
		object->draw(spriteHandle, _viewport);
	}
	_mapFront->render(spriteHandle, _viewport);
}

void TestScene::release()
{
}


GameObject * TestScene::getObject(eObjectID id)
{
	if (id == eObjectID::ALADDIN)
		return getAladdin();
	eObjectID objectID;
	if (_active_object.size() == 0)
	{
		return nullptr;
	}
	for (GameObject* object : _active_object)
	{
		objectID = object->getID();
		if (objectID == id)
			return object;
	}
	return nullptr;
}

GameObject * TestScene::getAladdin()
{
	return _Aladdin;
}

void TestScene::destroyobject()
{
	for (auto object : _listobject)
	{
		if (object->getBehaviorComponent() != nullptr && object->getBehaviorComponent()->getStatus() == eStatus::DESTROY)	// kiểm tra nếu là destroy thì loại khỏi list
		{
			object->release();
			// http://www.cplusplus.com/reference/algorithm/remove/
			auto rs1 = std::remove(_listobject.begin(), _listobject.end(), object);
			_listobject.pop_back();			// sau khi remove thì còn một phần tử cuối cùng vôi ra. giống như dịch mảng. nên cần bỏ nó đi

											//https://msdn.microsoft.com/en-us/library/cby9kycs.aspx (dynamic_cast) 
											// loại khỏi list control

			delete object;
			break;		// sau pop_back phần tử đi thì list bị thay đồi, nên vòng for-each không còn nguyên trạng nữa. -> break (mỗi frame chỉ remove được 1 đối tượng)
		}
	}
	//for (auto name : QNode::ActiveObject)
	//{
	//	auto object = _mapobject.find(name);
	//	if (object == _mapobject.end() || object._Ptr == nullptr)
	//		continue;
	//	if (object->second->getStatus() == eStatus::DESTROY)	// kiểm tra nếu là destroy thì loại khỏi list
	//	{
	//		//if (dynamic_cast<BaseEnemy*> (object->second) != nullptr)
	//		//{
	//		//	SoundManager::getInstance()->Play(eSoundId::DESTROY_ENEMY);
	//		//}
	//		object->second->release();
	//		delete object->second;
	//		object->second = NULL;
	//		_mapobject.erase(name);

	//	}
	//}
}

void TestScene::updateViewport(GameObject * objTracker, float deltatime)
{
	if (_updateViewport == false)
	{
		return;
	}
	float lerp = 5.0f;
	// Vị trí hiện tại của viewport. 
	GVector2 current_position = _viewport->getPositionWorld();
	GVector2 worldsize;
	worldsize.x = _mapBack->getTextureWidth();
	worldsize.y = _mapBack->getTextureHeight();
	// Bám theo object.
	GVector2 new_position;
	float trackerX = max(objTracker->getPhysicsComponent()->getPositionX() - 260, 0);
	float trackerY = max(objTracker->getPhysicsComponent()->getPositionY() + 300, WINDOW_HEIGHT);
	current_position.x += (trackerX - current_position.x) * lerp * deltatime / 1000;
	current_position.y += (trackerY - current_position.y) * lerp * deltatime / 1000;


	// Không cho đi quá map.
	//if (current_position.x + WINDOW_WIDTH > current_position.x)
	//{
	//	current_position.x = worldsize.x - WINDOW_WIDTH;
	//}

	_viewport->setPositionWorld(current_position);
}


void TestScene::addToScene(GameObject * obj, sigcxx::SLOT slot)
{
	_listobject.push_back(obj);
}

void TestScene::updateInput(float dt)
{

}
