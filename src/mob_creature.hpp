/*
* Copyright (c) 2009 Jice
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * The name of Jice may not be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY Jice ``AS IS'' AND ANY
* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL Jice BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

class Game;
class Creature;

enum CreatureTypeId {
	CREATURE_FRIEND,
	CREATURE_FISH,
	CREATURE_SUEDE,
	NB_CREATURE_TYPES
};

enum CreatureFlags {
	CREATURE_REPLACABLE = 1, // if too far, put it back near the player
	CREATURE_OFFSCREEN  = 2, // updated even if out of console screen
	CREATURE_SAVE       = 4, // save this creature in savegame
	CREATURE_NOTBLOCK   = 8, // does not block path
	CREATURE_CATCHABLE  =16, // can catch a creature by clicking on it when adjacent
};

#define CREATURE_NAME_SIZE 32
#define CREATURE_TALK_SIZE 64
#define VISIBLE_HEIGHT 0.05f
#define MIN_VISIBLE_HEIGHT 0.02f

enum ConditionTypeId {
	STUNNED,
	BLEED,
	HEAL,
	POISONED,
	IMMUNE,
	PARALIZED,
	CRIPPLED,
	WOUNDED,
};
class ConditionType {
public :
	ConditionTypeId type;
	const char *name;
	static TCODList<ConditionType *> list;
	static ConditionType *find(const char *name);
	static ConditionType *get(ConditionTypeId type);
	static void init();
	// return true if condition can be applied
	bool check(Creature *cr);
private :
	ConditionType(ConditionTypeId type,const char *name) : type(type),name(name) {}
};

class Condition {
public :
	ConditionType *type;
	Creature *target;
	float initialDuration,duration,amount; // remaining duration
	float curAmount;
	const char *alias;
	Condition(ConditionTypeId type, float duration, float amount, const char *alias = NULL);
	// return true if condition finished
	bool update(float elapsed);
	void applyTo(Creature *cr);
	const char *getName() { return alias ? alias : type->name; }
};

class Creature : public DynamicEntity, public ITCODPathCallback, public NoisyThing, public SaveListener {
public :
	CreatureTypeId type;
	TCODColor col;
	int ch; // character
	float life,maxLife;
	float speed;
	float height; // in meters
	TCODPath *path;
	bool ignoreCreatures; // walk mode
	bool burn;
	int flags;
	char name[CREATURE_NAME_SIZE];
	Item *mainHand, *offHand;
	Item *asItem; // an item corresponding to this creature
	TCODList<Condition *> conditions;

	// ai
	Behavior *currentBehavior;

	Creature();
	virtual ~Creature();

	// conditions
	void addCondition(Condition *cond);
	bool hasCondition(ConditionTypeId type, const char *alias=NULL);
	Condition *getCondition(ConditionTypeId type, const char *alias=NULL);
	void updateConditions(float elapsed);

	// factory
	static Creature *getCreature(CreatureTypeId id);
	static TCODList<Creature *> creatureByType[NB_CREATURE_TYPES];

	virtual bool update(float elapsed);
	virtual void render(LightMap *lightMap);
	void renderTalk();
	virtual void takeDamage(float amount);
	virtual void stun(float delay);
	virtual float getWalkCost( int xFrom, int yFrom, int xTo, int yTo, void *userData ) const;
	void talk(const char *text);
	bool isTalking() { return talkText.delay > 0.0f; }
	bool isInRange(int x, int y);
	bool isPlayer();

	// flags
	bool isReplacable() const { return ( flags & CREATURE_REPLACABLE ) != 0; }
	bool isUpdatedOffscreen() const { return ( flags & CREATURE_OFFSCREEN ) != 0; }
	bool mustSave() const { return ( flags & CREATURE_SAVE) != 0; }
	bool isBlockingPath() const { return (flags & CREATURE_NOTBLOCK) == 0; }
	bool isCatchable() const { return (flags & CREATURE_CATCHABLE) != 0; }

	// items
	Item * addToInventory(Item *it); // in case of stackable items, returned item might be != it
	Item * removeFromInventory(Item *it); // same as addToInventory
	void removeFromInventory(Item *it,bool del);
	Item **inventoryBegin() { return inventory.begin(); }
	Item **inventoryEnd() { return inventory.end(); }
	void equip(Item *it);
	void unequip(Item *it);
	// same as equip/unequip but with messages (you're wielding...)
	void wield(Item *it);
	void unwield(Item *it);
	virtual void initItem() {} // build asItem member

	// SaveListener
	bool loadData(uint32 chunkId, uint32 chunkVersion, TCODZip *zip);
	void saveData(uint32 chunkId, TCODZip *zip);

	float fovRange;
	bool toDelete;
protected :
	friend class Behavior;
	friend class FollowBehavior;
	friend class HerdBehavior;
	TCODList<Item *> inventory;
	float walkTimer,pathTimer;
	float curDmg;
	struct TalkText : public Rect {
		char text[CREATURE_TALK_SIZE];
		float delay;
	} talkText;
	bool walk(float elapsed);
	void randomWalk(float elapsed);
};
