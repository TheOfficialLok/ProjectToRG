/*
 * LuaAiAgent.cpp
 *
 *  Created on: 21/01/2012
 *      Author: victor
 */

#include "LuaAiAgent.h"

#include <engine/core/ManagedReference.h>
#include <system/lang/ref/Reference.h>
#include <system/lang/String.h>

#include "server/chat/ChatManager.h"
#include "server/zone/ZoneServer.h"
#include "server/chat/StringIdChatParameter.h"
#include "server/ServerCore.h"

#include "server/zone/managers/collision/CollisionManager.h"
#include "server/zone/managers/reaction/ReactionManager.h"
#include "server/zone/objects/intangible/PetControlDevice.h"
#include "server/zone/objects/creature/ai/AiAgent.h"
#include "server/zone/objects/intangible/tasks/PetControlDeviceStoreTask.h"
#include "server/zone/objects/area/ActiveArea.h"
#include "server/zone/managers/creature/PetManager.h"

const char LuaAiAgent::className[] = "LuaAiAgent";

Luna<LuaAiAgent>::RegType LuaAiAgent::Register[] = {
		{ "_setObject", &LuaAiAgent::_setObject },
		{ "_getObject", &LuaSceneObject::_getObject },
		{ "setAITemplate", &LuaAiAgent::setAITemplate },
		{ "setFollowObject", &LuaAiAgent::setFollowObject },
		{ "setOblivious", &LuaAiAgent::setOblivious },
		{ "setWatchObject", &LuaAiAgent::setWatchObject },
		{ "setStalkObject", &LuaAiAgent::setStalkObject },
		{ "getFollowObject", &LuaAiAgent::getFollowObject },
		{ "storeFollowObject", &LuaAiAgent::storeFollowObject },
		{ "restoreFollowObject", &LuaAiAgent::restoreFollowObject },
		{ "getTargetOfTargetID", &LuaAiAgent::getTargetOfTargetID },
		{ "getTargetID", &LuaCreatureObject::getTargetID },
		{ "getObjectID", &LuaSceneObject::getObjectID },
		{ "getMovementState", &LuaAiAgent::getMovementState },
		{ "setMovementState", &LuaAiAgent::setMovementState },
		{ "setNextPosition", &LuaAiAgent::setNextPosition },
		{ "getMaxDistance", &LuaAiAgent::getMaxDistance },
		{ "generatePatrol", &LuaAiAgent::generatePatrol },
		{ "clearPatrolPoints", &LuaAiAgent::clearPatrolPoints },
		{ "clearCurrentPath", &LuaAiAgent::clearCurrentPath },
		{ "setDestination", &LuaAiAgent::setDestination },
		{ "isWaiting", &LuaAiAgent::isWaiting },
		{ "stopWaiting", &LuaAiAgent::stopWaiting },
		{ "setWait", &LuaAiAgent::setWait },
		{ "getCurrentSpeed", &LuaCreatureObject::getCurrentSpeed },
		{ "setCurrentSpeed", &LuaAiAgent::setCurrentSpeed },
		{ "getTargetFromMap", &LuaAiAgent::getTargetFromMap },
		{ "getTargetFromDefenders", &LuaAiAgent::getTargetFromDefenders },
		{ "getTargetFromTargetsDefenders", &LuaAiAgent::getTargetFromTargetsDefenders },
		{ "validateTarget", &LuaAiAgent::validateTarget },
		{ "validateFollow", &LuaAiAgent::validateFollow },
		{ "followHasState", &LuaAiAgent::followHasState },
		{ "selectSpecialAttack", &LuaAiAgent::selectSpecialAttack },
		{ "selectDefaultAttack", &LuaAiAgent::selectDefaultAttack },
		{ "validateStateAttack", &LuaAiAgent::validateStateAttack },
		{ "removeDefender", &LuaAiAgent::removeDefender },
		{ "removeDefenders", &LuaAiAgent::removeDefenders },
		{ "enqueueAttack", &LuaAiAgent::enqueueAttack },
		{ "isRetreating", &LuaAiAgent::isRetreating },
		{ "isFleeing", &LuaAiAgent::isFleeing },
		{ "runAway", &LuaAiAgent::runAway },
		{ "getKinetic", &LuaAiAgent::getKinetic },
		{ "getEnergy", &LuaAiAgent::getEnergy },
		{ "getElectricity", &LuaAiAgent::getElectricity },
		{ "getStun", &LuaAiAgent::getStun },
		{ "getBlast", &LuaAiAgent::getBlast },
		{ "getHeat", &LuaAiAgent::getHeat },
		{ "getCold", &LuaAiAgent::getCold },
		{ "getAcid", &LuaAiAgent::getAcid },
		{ "getLightSaber", &LuaAiAgent::getLightSaber },
		{ "isStalker", &LuaAiAgent::isStalker },
		{ "isKiller", &LuaAiAgent::isKiller },
		{ "getFerocity", &LuaAiAgent::getFerocity },
		{ "getAggroRadius", &LuaAiAgent::getAggroRadius },
		{ "getArmor", &LuaAiAgent::getArmor },
		{ "getDespawnOnNoPlayerInRange", &LuaAiAgent::getDespawnOnNoPlayerInRange },
		{ "getNumberOfPlayersInRange", &LuaAiAgent::getNumberOfPlayersInRange },
		{ "getFactionString", &LuaAiAgent::getFactionString },
		{ "getChanceHit", &LuaAiAgent::getChanceHit },
		{ "getDamageMin", &LuaAiAgent::getDamageMin },
		{ "getDamageMax", &LuaAiAgent::getDamageMax },
		{ "getBaseXp", &LuaAiAgent::getBaseXp },
		{ "getDiet", &LuaAiAgent::getDiet },
		{ "hasLoot", &LuaAiAgent::hasLoot },
		{ "isEventMob", &LuaAiAgent::isEventMob },
		{ "isPet", &LuaAiAgent::isPet },
		{ "isFactionPet", &LuaAiAgent::isFactionPet },
		{ "isCreature", &LuaSceneObject::isCreature},
		{ "isAggressiveTo", &LuaAiAgent::isAggressiveTo },
		{ "isAttackableBy", &LuaAiAgent::isAttackableBy },
		{ "isCamouflaged", &LuaAiAgent::isCamouflaged },
		{ "shouldRetreat", &LuaAiAgent::shouldRetreat },
		{ "leash", &LuaAiAgent::leash },
		{ "clearCombatState", &LuaAiAgent::clearCombatState },
		{ "isInCombat", &LuaCreatureObject::isInCombat },
		{ "checkLineOfSight", &LuaAiAgent::checkLineOfSight },
		{ "activateRecovery", &LuaAiAgent::activateRecovery },
		{ "executeBehavior", &LuaAiAgent::executeBehavior },
		{ "info", &LuaAiAgent::info },
		{ "setDefender", &LuaAiAgent::setDefender },
		{ "addDefender", &LuaAiAgent::addDefender },
		{ "assist", &LuaAiAgent::assist },
		{ "checkRange", &LuaAiAgent::checkRange },
		{ "getSocialGroup", &LuaAiAgent::getSocialGroup },
		{ "getOwner", &LuaCreatureObject::getOwner },
		{ "getLastCommand", &LuaAiAgent::getLastCommand },
		{ "getLastCommandTarget", &LuaAiAgent::getLastCommandTarget },
		{ "setLastCommandTarget", &LuaAiAgent::setLastCommandTarget },
		{ "setAlertDuration", &LuaAiAgent::setAlertDuration },
		{ "alertedTimeIsPast", &LuaAiAgent::alertedTimeIsPast },
		{ "setLevel", &LuaAiAgent::setLevel },
		{ "hasReactionChatMessages", &LuaAiAgent::hasReactionChatMessages },
		{ "sendReactionChat", &LuaAiAgent::sendReactionChat },
		{ "addPatrolPoint", &LuaAiAgent::addPatrolPoint },
		{ "setConvoTemplate", &LuaAiAgent::setConvoTemplate },
		{ "setHomeLocation", &LuaAiAgent::setHomeLocation },
		{ "setNoAiAggro", &LuaAiAgent::setNoAiAggro },
		{ "doDespawn", &LuaAiAgent::doDespawn },
		{ "getCreatureTemplateName", &LuaAiAgent::getCreatureTemplateName },
		{ "isInRangeOfHome", &LuaAiAgent::isInRangeOfHome },
		{ "getPatrolPointsSize", &LuaAiAgent::getPatrolPointsSize },
		{ "addObjectFlag", &LuaAiAgent::addObjectFlag },
		{ "removeObjectFlag", &LuaAiAgent::removeObjectFlag },
		{ "setAIDebug", &LuaAiAgent::setAIDebug },
		{ "storePet", &LuaAiAgent::storePet },
		{ "setEventArea", &LuaAiAgent::setEventArea },
		{ "setHamRegenDisabled", &LuaAiAgent::setHamRegenDisabled },
		{ 0, 0 }
};


LuaAiAgent::LuaAiAgent(lua_State *L) : LuaCreatureObject(L) {
#ifdef DYNAMIC_CAST_LUAOBJECTS
	realObject = dynamic_cast<AiAgent*>(_getRealSceneObject());

	E3_ASSERT(!_getRealSceneObject() || realObject != nullptr);
#else
	realObject = static_cast<AiAgent*>(lua_touserdata(L, 1));
#endif
}

LuaAiAgent::~LuaAiAgent(){
}

int LuaAiAgent::_setObject(lua_State* L) {
	LuaCreatureObject::_setObject(L);

#ifdef DYNAMIC_CAST_LUAOBJECTS
	auto obj = dynamic_cast<AiAgent*>(_getRealSceneObject());

	if (realObject != obj)
		realObject = obj;

	E3_ASSERT(!_getRealSceneObject() || realObject != nullptr);
#else
	auto obj = static_cast<AiAgent*>(lua_touserdata(L, -1));

	if (realObject != obj)
		realObject = obj;
#endif

	return 0;
}

int LuaAiAgent::setAITemplate(lua_State* L) {
	Locker locker(realObject);
	realObject->setAITemplate();

	return 0;
}

int LuaAiAgent::setFollowObject(lua_State* L) {
	SceneObject* obj = (SceneObject*) lua_touserdata(L, -1);

	realObject->setFollowObject(obj);

	return 0;
}

int LuaAiAgent::setOblivious(lua_State* L) {
	realObject->setOblivious();
	return 0;
}

int LuaAiAgent::setWatchObject(lua_State* L) {
	SceneObject* obj = (SceneObject*) lua_touserdata(L, -1);
	realObject->setWatchObject(obj);
	return 0;
}

int LuaAiAgent::setStalkObject(lua_State* L) {
	SceneObject* obj = (SceneObject*) lua_touserdata(L, -1);
	realObject->setStalkObject(obj);
	return 0;
}

int LuaAiAgent::getFollowObject(lua_State* L) {
	SceneObject* followObject = realObject->getFollowObject().get();

	if (followObject == nullptr)
		lua_pushnil(L);
	else
		lua_pushlightuserdata(L, followObject);

	return 1;
}

int LuaAiAgent::storeFollowObject(lua_State* L) {
	realObject->storeFollowObject();
	return 0;
}

int LuaAiAgent::restoreFollowObject(lua_State* L) {
	realObject->restoreFollowObject();
	return 0;
}

int LuaAiAgent::getTargetOfTargetID(lua_State* L) {
	SceneObject* target = realObject->getFollowObject().get();
	if (target == nullptr || !target->isCreatureObject()) {
		lua_pushnil(L);
		return 1;
	}

	CreatureObject* targetCreo = cast<CreatureObject*>(target);
	if (targetCreo == nullptr) {
		lua_pushnil(L);
		return 1;
	}

	lua_pushinteger(L, targetCreo->getTargetID());

	return 1;
}

int LuaAiAgent::getMovementState(lua_State* L) {
	unsigned int state = realObject->getMovementState();

	lua_pushinteger(L, state);

	return 1;
}

int LuaAiAgent::setNextPosition(lua_State* L) {
	ZoneServer* zoneServer = ServerCore::getZoneServer();
	if (zoneServer == nullptr)
		return 0;

	uint64 cellID = lua_tointeger(L, -1);
	CellObject* cell = nullptr;

	if (cellID != 0) {
		cell = zoneServer->getObject(cellID).castTo<CellObject*>();
	}

	float y = lua_tonumber(L, -2);
	float z = lua_tonumber(L, -3);
	float x = lua_tonumber(L, -4);

	realObject->setNextPosition(x, z, y, cell);

	return 0;
}

int LuaAiAgent::setMovementState(lua_State* L) {
	int state = lua_tonumber(L, -1);

	realObject->setMovementState(state);

	return 0;
}

int LuaAiAgent::getMaxDistance(lua_State* L) {
	float maxDistance = realObject->getMaxDistance();

	lua_pushnumber(L, maxDistance);

	return 1;
}

int LuaAiAgent::generatePatrol(lua_State* L) {
	float dist = lua_tonumber(L, -1);
	int num = lua_tointeger(L, -2);

	bool retVal = realObject->generatePatrol(num, dist);

	lua_pushboolean(L, retVal);

	return 1;
}

int LuaAiAgent::clearPatrolPoints(lua_State* L) {
	Locker lock(realObject);

	realObject->clearPatrolPoints();

	return 0;
}

int LuaAiAgent::clearCurrentPath(lua_State* L) {
	Locker lock(realObject);

	realObject->clearCurrentPath();

	return 0;
}

int LuaAiAgent::setDestination(lua_State* L) {
	int retVal = realObject->setDestination();

	lua_pushinteger(L, retVal);

	return 1;
}

int LuaAiAgent::setLevel(lua_State* L) {
	int level = (int) lua_tonumber(L, -1);

	Locker locker(realObject);

	realObject->setLevel(level);

	return 0;
}

int LuaAiAgent::isWaiting(lua_State* L) {
	bool retVal = realObject->isWaiting();

	lua_pushboolean(L, retVal);

	return 1;
}

int LuaAiAgent::stopWaiting(lua_State* L) {
	Locker locker(realObject);

	realObject->stopWaiting();

	return 0;
}

int LuaAiAgent::setWait(lua_State* L) {
	int wait = (int) lua_tonumber(L, -1);

	Locker locker(realObject);

	realObject->setWait(wait);

	return 0;
}

int LuaAiAgent::setCurrentSpeed(lua_State* L) {
	float currentSpeed = lua_tonumber(L, -1);

	realObject->setCurrentSpeed(currentSpeed);

	return 0;
}

int LuaAiAgent::getTargetFromMap(lua_State* L) {
	Locker locker(realObject);

	SceneObject* retVal = realObject->getTargetFromMap();

	if (retVal == nullptr)
		lua_pushnil(L);
	else
		lua_pushlightuserdata(L, retVal);

	return 1;
}

int LuaAiAgent::getTargetFromDefenders(lua_State* L) {
	Locker locker(realObject);

	SceneObject* retVal = realObject->getTargetFromDefenders();

	if (retVal == nullptr)
		lua_pushnil(L);
	else
		lua_pushlightuserdata(L, retVal);

	return 1;
}

int LuaAiAgent::getTargetFromTargetsDefenders(lua_State* L) {
	Locker locker(realObject);

	SceneObject* retVal = realObject->getTargetFromTargetsDefenders();

	if (retVal == nullptr)
		lua_pushnil(L);
	else
		lua_pushlightuserdata(L, retVal);

	return 1;
}

int LuaAiAgent::validateTarget(lua_State* L) {
	SceneObject* obj = (SceneObject*) lua_touserdata(L, -1);

	bool retVal = realObject->validateTarget(obj);

	lua_pushboolean(L, retVal);

	return 1;
}

int LuaAiAgent::validateFollow(lua_State* L) {
	bool retVal = realObject->validateTarget();

	lua_pushboolean(L, retVal);

	return 1;
}

int LuaAiAgent::followHasState(lua_State* L) {
	SceneObject* follow = realObject->getFollowObject().get();

	if (follow == nullptr || !follow->isCreatureObject()) {
		lua_pushboolean(L, false);
		return 1;
	}

	int state = lua_tointeger(L, -1);

	bool retVal = cast<CreatureObject*>(follow)->hasState(state);

	lua_pushboolean(L, retVal);

	return 1;
}

int LuaAiAgent::selectSpecialAttack(lua_State* L) {
	int attackNum = lua_tointeger(L, -1);

	if (attackNum == -1)
		realObject->selectSpecialAttack();
	else
		realObject->selectSpecialAttack(attackNum);

	return 0;
}

int LuaAiAgent::selectDefaultAttack(lua_State* L) {
	realObject->selectDefaultAttack();

	return 0;
}

int LuaAiAgent::validateStateAttack(lua_State* L) {
	bool retVal = realObject->validateStateAttack();

	lua_pushboolean(L, retVal);

	return 1;
}

int LuaAiAgent::removeDefender(lua_State* L) {
	Locker locker(realObject);

	realObject->removeDefender(realObject->getFollowObject().get());

	return 0;
}

int LuaAiAgent::removeDefenders(lua_State* L) {
	Locker locker(realObject);

	realObject->removeDefenders();

	return 0;
}

int LuaAiAgent::enqueueAttack(lua_State* L) {
	realObject->enqueueAttack();

	return 0;
}

int LuaAiAgent::isRetreating(lua_State* L) {
	bool retVal = realObject->isRetreating();
	lua_pushboolean(L, retVal);
	return 1;
}

int LuaAiAgent::isFleeing(lua_State* L) {
	bool retVal = realObject->isFleeing();
	lua_pushboolean(L, retVal);
	return 1;
}

int LuaAiAgent::runAway(lua_State* L) {
	SceneObject* scene = static_cast<CreatureObject*>(lua_touserdata(L, -2));
	Reference<CreatureObject*> target = dynamic_cast<CreatureObject*>(scene);
	float range = lua_tonumber(L, -1);
	Reference<AiAgent*> agentObject = realObject;

	if (target != nullptr) {
		Core::getTaskManager()->executeTask([=] () {
			Locker locker(agentObject);

			agentObject->runAway(target, range, false);
		}, "RunAwayLambda");
	}

	return 0;
}

int LuaAiAgent::getKinetic(lua_State* L) {
	lua_pushnumber(L, realObject->getKinetic());
	return 1;
}

int LuaAiAgent::getEnergy(lua_State* L) {
	lua_pushnumber(L, realObject->getEnergy());
	return 1;
}

int LuaAiAgent::getElectricity(lua_State* L) {
	lua_pushnumber(L, realObject->getElectricity());
	return 1;
}

int LuaAiAgent::getStun(lua_State* L) {
	lua_pushnumber(L, realObject->getStun());
	return 1;
}

int LuaAiAgent::getBlast(lua_State* L) {
	lua_pushnumber(L, realObject->getBlast());
	return 1;
}

int LuaAiAgent::getHeat(lua_State* L) {
	lua_pushnumber(L, realObject->getHeat());
	return 1;
}

int LuaAiAgent::getCold(lua_State* L) {
	lua_pushnumber(L, realObject->getCold());
	return 1;
}

int LuaAiAgent::getAcid(lua_State* L) {
	lua_pushnumber(L, realObject->getAcid());
	return 1;
}

int LuaAiAgent::getLightSaber(lua_State* L) {
	lua_pushnumber(L, realObject->getLightSaber());
	return 1;
}

int LuaAiAgent::isStalker(lua_State* L) {
	lua_pushboolean(L, realObject->isStalker());
	return 1;
}

int LuaAiAgent::isKiller(lua_State* L) {
	lua_pushboolean(L, realObject->isKiller());
	return 1;
}

int LuaAiAgent::getFerocity(lua_State* L) {
	lua_pushinteger(L, realObject->getFerocity());
	return 1;
}

int LuaAiAgent::getAggroRadius(lua_State* L) {
	lua_pushinteger(L, realObject->getAggroRadius());
	return 1;
}

int LuaAiAgent::getArmor(lua_State* L) {
	lua_pushinteger(L, realObject->getArmor());
	return 1;
}

int LuaAiAgent::getDespawnOnNoPlayerInRange(lua_State* L) {
	lua_pushboolean(L, realObject->getDespawnOnNoPlayerInRange());
	return 1;
}

int LuaAiAgent::getNumberOfPlayersInRange(lua_State* L) {
	lua_pushinteger(L, realObject->getNumberOfPlayersInRange());
	return 1;
}

int LuaAiAgent::getFactionString(lua_State* L) {
	lua_pushstring(L, realObject->getFactionString().toCharArray());
	return 1;
}

int LuaAiAgent::getChanceHit(lua_State* L) {
	lua_pushnumber(L, realObject->getChanceHit());
	return 1;
}

int LuaAiAgent::getDamageMin(lua_State* L) {
	lua_pushinteger(L, realObject->getDamageMin());
	return 1;
}

int LuaAiAgent::getDamageMax(lua_State* L) {
	lua_pushinteger(L, realObject->getDamageMax());
	return 1;
}

int LuaAiAgent::getBaseXp(lua_State* L) {
	lua_pushinteger(L, realObject->getBaseXp());
	return 1;
}

int LuaAiAgent::getDiet(lua_State* L) {
	lua_pushinteger(L, realObject->getDiet());
	return 1;
}

int LuaAiAgent::hasLoot(lua_State* L) {
	lua_pushboolean(L, realObject->hasLoot());
	return 1;
}

int LuaAiAgent::isEventMob(lua_State* L) {
	lua_pushboolean(L, realObject->isEventMob());
	return 1;
}

int LuaAiAgent::isPet(lua_State* L) {
	lua_pushboolean(L, realObject->isPet());
	return 1;
}

int LuaAiAgent::isFactionPet(lua_State* L) {
	bool factionPet = false;

	if (realObject->isPet()) {
		ManagedReference<PetControlDevice*> controlDevice = realObject->getControlDevice().get().castTo<PetControlDevice*>();

		if (controlDevice != nullptr) {
			factionPet = controlDevice->getPetType() == PetManager::FACTIONPET;
		}
	}

	lua_pushboolean(L, factionPet);

	return 1;
}

int LuaAiAgent::isAggressiveTo(lua_State* L) {
	CreatureObject* obj = (CreatureObject*) lua_touserdata(L, -1);

	bool retVal = realObject->isAggressiveTo(obj);
	lua_pushboolean(L, retVal);

	return 1;
}

int LuaAiAgent::isAttackableBy(lua_State* L) {
	CreatureObject* obj = (CreatureObject*) lua_touserdata(L, -1);

	bool retVal = realObject->isAttackableBy(obj);
	lua_pushboolean(L, retVal);

	return 1;
}

int LuaAiAgent::isCamouflaged(lua_State* L) {
	CreatureObject* obj = (CreatureObject*) lua_touserdata(L, -1);

	bool retVal = realObject->isCamouflaged(obj);
	lua_pushboolean(L, retVal);

	return 1;
}

int LuaAiAgent::shouldRetreat(lua_State* L) {
	float range = lua_tonumber(L, -1);
	PatrolPoint* homeLocation = realObject->getHomeLocation();

	bool retVal;
	SceneObject* target = realObject->getFollowObject().get();

	if (realObject->isRetreating())
		retVal = false;
	else if (target != nullptr)
		retVal = !homeLocation->isInRange(target, range);
	else
		retVal = !homeLocation->isInRange(realObject, range);

	lua_pushboolean(L, retVal);

	return 1;
}

int LuaAiAgent::leash(lua_State* L) {
	bool forcePeace = true;

	int numberOfArguments = lua_gettop(L);

	if (numberOfArguments == 1) {
		forcePeace = lua_toboolean(L, -1);
	}

	Locker locker(realObject);

	realObject->leash(forcePeace);

	return 0;
}

int LuaAiAgent::clearCombatState(lua_State* L) {
	bool clearDefenders = lua_toboolean(L, -1);

	Locker locker(realObject);

	realObject->clearCombatState(clearDefenders);

	return 0;
}

int LuaAiAgent::checkLineOfSight(lua_State* L) {
	SceneObject* obj = (SceneObject*) lua_touserdata(L, -1);
	bool retVal = CollisionManager::checkLineOfSight(realObject, obj);

	lua_pushboolean(L, retVal);

	return 1;
}

int LuaAiAgent::activateRecovery(lua_State* L) {
	Locker locker(realObject);

	realObject->activateRecovery();

	return 0;
}

int LuaAiAgent::executeBehavior(lua_State* L) {
	Locker locker(realObject);
	realObject->activateAiBehavior();
	return 0;
}

int LuaAiAgent::info(lua_State* L) {
	String msg = lua_tostring(L, -1);

	realObject->info(msg, true);

	return 0;
}

int LuaAiAgent::setDefender(lua_State* L) {
	SceneObject* obj = (SceneObject*) lua_touserdata(L, -1);

	Locker locker(realObject);

	realObject->setDefender(obj);

	return 0;
}

int LuaAiAgent::addDefender(lua_State* L) {
	SceneObject* obj = (SceneObject*) lua_touserdata(L, -1);

	Locker locker(realObject);

	realObject->addDefender(obj);

	return 0;
}

int LuaAiAgent::assist(lua_State* L) {
	SceneObject* obj = (SceneObject*) lua_touserdata(L, -1);
	if (obj == nullptr || !obj->isAiAgent())
		return 0;

	AiAgent* agent = cast<AiAgent*>(obj);
	if (agent == nullptr)
		return 0;

	SceneObject* target = agent->getFollowObject().get();
	if (target == nullptr)
		return 0;

	Locker locker(realObject);

	realObject->sendReactionChat(nullptr, ReactionManager::ASSIST);

	realObject->setDefender(target);

	return 0;
}

int LuaAiAgent::checkRange(lua_State* L) {
	float dist = lua_tonumber(L, -1);
	SceneObject* obj = (SceneObject*) lua_touserdata(L, -2);

	bool retVal = false;
	if (realObject->isInRange(obj, dist))
		retVal = true;

	lua_pushboolean(L, retVal);
	return 1;
}

int LuaAiAgent::getSocialGroup(lua_State* L) {
	String socGroup = realObject->getSocialGroup();

	lua_pushstring(L, socGroup.toCharArray());

	return 1;
}

int LuaAiAgent::getLastCommand(lua_State* L) {
	ManagedReference<PetControlDevice*> controlDevice = realObject->getControlDevice().get().castTo<PetControlDevice*>();

	if (controlDevice == nullptr)
		lua_pushinteger(L, 0);
	else
		lua_pushinteger(L, controlDevice->getLastCommand());

	return 1;
}

int LuaAiAgent::getLastCommandTarget(lua_State* L) {
	ManagedReference<PetControlDevice*> controlDevice = realObject->getControlDevice().get().castTo<PetControlDevice*>();
	if (controlDevice == nullptr)
		lua_pushnil(L);

	SceneObject* target = controlDevice->getLastCommandTarget().get();

	if (target == nullptr)
		lua_pushnil(L);
	else
		lua_pushlightuserdata(L, target);

	return 1;
}

int LuaAiAgent::setLastCommandTarget(lua_State* L) {
	SceneObject* obj = (SceneObject*) lua_touserdata(L, -1);

	ManagedReference<PetControlDevice*> controlDevice = realObject->getControlDevice().get().castTo<PetControlDevice*>();
	if (controlDevice == nullptr)
		return 0;

  	Locker locker(controlDevice);

	controlDevice->setLastCommandTarget(obj);

	return 0;
}

int LuaAiAgent::setAlertDuration(lua_State* L) {
	int duration = lua_tointeger(L, -1);

	Time* alert = realObject->getAlertedTime();
	if (alert != nullptr) {
		alert->updateToCurrentTime();
		alert->addMiliTime(duration);
	}

	return 0;
}

int LuaAiAgent::alertedTimeIsPast(lua_State* L) {
	Time* alert = realObject->getAlertedTime();

	if (alert != nullptr)
		lua_pushboolean(L, alert->isPast());
	else
		lua_pushboolean(L, true);

	return 1;
}

int LuaAiAgent::hasReactionChatMessages(lua_State* L) {
	lua_pushboolean(L, realObject->hasReactionChatMessages());

	return 1;
}

int LuaAiAgent::sendReactionChat(lua_State* L) {
	int state = lua_tointeger(L, -1);
	int type = lua_tointeger(L, -2);

	Locker locker(realObject);

	realObject->sendReactionChat(nullptr, type, state);

	return 0;
}

int LuaAiAgent::addPatrolPoint(lua_State* L) {
	CellObject* cell = (CellObject*) lua_touserdata(L, -1);
	float y = lua_tonumber(L, -2);
	float z = lua_tonumber(L, -3);
	float x = lua_tonumber(L, -4);

	PatrolPoint newPoint(x, z, y, cell);

	realObject->addPatrolPoint(newPoint);

	return 0;
}

int LuaAiAgent::setConvoTemplate(lua_State* L) {
	String templateName = lua_tostring(L, -1);

	Locker locker(realObject);

	realObject->setConvoTemplate(templateName);

	return 0;
}

int LuaAiAgent::setHomeLocation(lua_State* L) {
	CellObject* cell = (CellObject*) lua_touserdata(L, -1);
	float y = lua_tonumber(L, -2);
	float z = lua_tonumber(L, -3);
	float x = lua_tonumber(L, -4);

	Locker locker(realObject);

	realObject->setHomeLocation(x, z, y, cell);

	return 0;
}

int LuaAiAgent::setNoAiAggro(lua_State* L) {
	Locker locker(realObject);

	if (!(realObject->getCreatureBitmask() & ObjectFlag::NOAIAGGRO)) {
		uint32 creatureBitmask = realObject->getCreatureBitmask();
		creatureBitmask |= ObjectFlag::NOAIAGGRO;

		realObject->setCreatureBitmask(creatureBitmask);
	}

	return 0;
}

int LuaAiAgent::doDespawn(lua_State* L) {
	Zone* zone = realObject->getZone();

	if (zone == nullptr)
		return 0;

	Locker locker(realObject);

	realObject->destroyObjectFromWorld(true);
	realObject->notifyDespawn(zone);

	return 0;
}

int LuaAiAgent::getCreatureTemplateName(lua_State* L) {
	String creoTemplName = realObject->getCreatureTemplate()->getTemplateName();

	lua_pushstring(L, creoTemplName.toCharArray());
	return 1;
}

int LuaAiAgent::isInRangeOfHome(lua_State* L) {
	float range = lua_tonumber(L, -1);
	PatrolPoint* home = realObject->getHomeLocation();

	bool ret = home->isInRange(realObject, range);

	lua_pushboolean(L, ret);

	return 1;
}

int LuaAiAgent::getPatrolPointsSize(lua_State* L) {

	int ret = realObject->getPatrolPointSize();

	lua_pushinteger(L, ret);

	return 1;
}
int LuaAiAgent::addObjectFlag(lua_State* L) {
	uint32 flag = lua_tointeger(L, -1);

	Locker locker(realObject);
	realObject->addObjectFlag(flag);
	realObject->setAITemplate();

	return 0;
}

int LuaAiAgent::removeObjectFlag(lua_State* L) {
	uint32 flag = lua_tointeger(L, -1);

	Locker locker(realObject);
	realObject->removeObjectFlag(flag);
	realObject->setAITemplate();

	return 0;
}

int LuaAiAgent::setAIDebug(lua_State* L) {
	//bool flag = lua_toboolean(L, -1);

	Locker locker(realObject);
	realObject->setAIDebug(true);

	return 0;
}

int LuaAiAgent::storePet(lua_State* L) {
	if (!realObject->isPet())
		return 0;

	CreatureObject* owner = realObject->getLinkedCreature().get();
	ManagedReference<PetControlDevice*> controlDevice = realObject->getControlDevice().get().castTo<PetControlDevice*>();

	if (owner != nullptr && controlDevice != nullptr) {
		Reference<PetControlDeviceStoreTask*> task = new PetControlDeviceStoreTask(controlDevice, owner, true);
		task->execute();
	}

	return 0;
}

int LuaAiAgent::setEventArea(lua_State* L) {
	ActiveArea* area = (ActiveArea*) lua_touserdata(L, -1);

	if (area == nullptr)
		return 0;

	Locker lock(realObject);

	realObject->setEventArea(area);

	return 0;
}

int LuaAiAgent::setHamRegenDisabled(lua_State* L) {
	bool regenDisabled = lua_toboolean(L, -1);

	Locker lock(realObject);

	realObject->setHamRegenDisabled(regenDisabled);

	return 0;
}
