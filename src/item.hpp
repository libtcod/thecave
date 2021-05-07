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
#pragma once

#include <cstdint>

#include "bas_entity.hpp"
#include "map_lightmap.hpp"
#include "map_light.hpp"
#include "item_modifier.hpp"

class Creature;

// item class, mainly for weapons. higher classes have more modifiers
enum ItemClass {
	ITEM_CLASS_STANDARD,
	ITEM_CLASS_GREEN,
	ITEM_CLASS_ORANGE,
	ITEM_CLASS_RED,
	ITEM_CLASS_SILVER,
	ITEM_CLASS_GOLD,
	NB_ITEM_CLASSES
};

enum ItemTypeId {
	// MISC
	ITEM_OAK_TWIG, ITEM_PINE_TWIG, ITEM_CAMPFIRE, ITEM_SCROLL,
	// BUILDING
	ITEM_DOOR, ITEM_WALL, ITEM_WINDOW,
	// STATIC CONTAINERS
	ITEM_CHEST,
	// CONTAINERS
	ITEM_BAG,
	// parts
	ITEM_WOODEN_HANDLE,
	ITEM_SMALL_BRONZE_BLADE,
	// TREES
	ITEM_PINE, ITEM_OAK, ITEM_APPLE_TREE,
	// FOOD
	ITEM_HEALTH_POTION, ITEM_APPLE, ITEM_ROTTEN_APPLE, ITEM_LIVING_FISH, ITEM_UNCOOKED_FISH, ITEM_COOKED_FISH, ITEM_SMOKED_FISH, ITEM_ROTTEN_FISH,
	// WEAPONS
	ITEM_KNIFE, ITEM_STONE, ITEM_WAND, ITEM_STAFF,
	NB_ITEMS
};

// item categories
enum ItemTagId {
	ITEM_WOOD=1,
	ITEM_CUT=2,
	ITEM_HANDLE=4,
	ITEM_SMALL_BLADE=8,
	ITEM_TAG_DOOR=16,
	ITEM_TREE=32,
	ITEM_BUILD_NOT_BLOCK=64, // building cell that cannot be blocked (door, window)
};

enum ItemFlags {
	ITEM_NOT_WALKABLE   	=   1,
	ITEM_NOT_TRANSPARENT	=   2,
	ITEM_NOT_PICKABLE    	=   4,
	ITEM_STACKABLE       	=   8,
	ITEM_SOFT_STACKABLE  	=  16,
	ITEM_DELETE_ON_USE   	=  32,
	ITEM_USE_WHEN_PICKED	=  64,
	ITEM_CONTAINER			= 128,
	ITEM_ACTIVATE_ON_BUMP	= 256,
};

enum WeaponPhase { IDLE, CAST, RELOAD };

// an ingredient for a crafting recipe
struct ItemIngredient {
	unsigned long long tag; // either a category
	ItemTypeId type; // or a specific item (if tag == 0)
	int quantity;
	bool optional;
	bool destroy; // consumed by the combination ?
	bool revert; // can be disassembled back ?
};

#define MAX_INGREDIENTS 5

// item recipe
struct ItemCombination {
	ItemTypeId resultType;
	ItemIngredient tool;
	int nbIngredients;
	ItemIngredient ingredients[MAX_INGREDIENTS];
};

enum ItemFeatureId {
	ITEM_FEAT_FIRE_EFFECT,  // fire has an effect on item (either transform or destroy)
	ITEM_FEAT_PRODUCES,  // item produces other items when clicked
	ITEM_FEAT_FOOD, // item can be eaten and restore health
	ITEM_FEAT_AGE_EFFECT, // time has an effect on item (either transform or destroy)
	ITEM_FEAT_ATTACK, // when wielded, deal damages or can be thrown
	ITEM_FEAT_LIGHT, // produces light when lying on ground
	ITEM_FEAT_HEAT, // produces heat
	ITEM_FEAT_CONTAINER, // can contain other items
	NB_ITEM_FEATURES
};

// context-menu actions in the inventory screen
enum ItemActionId {
	ITEM_ACTION_TAKE,
	ITEM_ACTION_USE,
	ITEM_ACTION_DROP,
	ITEM_ACTION_THROW,
	ITEM_ACTION_DISASSEMBLE,
	NB_ITEM_ACTIONS};

enum ItemActionFlag {
	ITEM_ACTION_INVENTORY=1,
	ITEM_ACTION_LOOT=2,
};

struct ItemAction {
	const char *name;
	int flags;
	static ItemAction *getFromId(ItemActionId id);
	inline bool onInventory() {return (flags & ITEM_ACTION_INVENTORY) != 0;}
	inline bool onLoot() {return (flags & ITEM_ACTION_LOOT) != 0;}
};

struct ItemType;

struct ItemFireEffect {
	float resistance; // in seconds
	ItemType *type; // if NULL, fire destroys the item
};

struct ItemProduce {
	float delay; // in seconds
	float chance;
	ItemType *type;
};

struct ItemFood {
	int health;
};

struct ItemAgeEffect {
	float delay; // in seconds
	ItemType *type; // if NULL, destroy item
};

// cannot use TCODColor in a union because it has a constructor...
struct ItemColor {
	uint8_t r,g,b;
	ItemColor& operator=(const TCODColor&);
};

struct ItemLight {
	float range;
	ItemColor color;
	float patternDelay;
	const char *pattern;
};

enum AttackWieldType {
	WIELD_NONE,
	WIELD_ONE_HAND,
	WIELD_MAIN_HAND,
	WIELD_OFF_HAND,
	WIELD_TWO_HANDS };

enum AttackFlags {
	WEAPON_PROJECTILE = 1, // something that is thrown (stone, shuriken, ...)
};
struct ItemAttack {
	AttackWieldType wield;
	float minCastDelay;
	float maxCastDelay;
	float minReloadDelay;
	float maxReloadDelay;
	float minDamagesCoef;
	float maxDamagesCoef;
	int flags;
};

struct ItemHeat {
	float intensity;
	float radius;
};

struct ItemContainer {
	int size;
};

struct ItemFeature {
	ItemFeatureId id;
	union {
		ItemFireEffect fireEffect;
		ItemProduce produce;
		ItemFood food;
		ItemAgeEffect ageEffect;
		ItemAttack attack;
		ItemLight light;
		ItemHeat heat;
		ItemContainer container;
	};
	static ItemFeature *getProduce(float delay, float chance, ItemType *type);
	static ItemFeature *getFireEffect(float resistance, ItemType *type);
	static ItemFeature *getAgeEffect(float delay, ItemType *type);
	static ItemFeature *getFood(int health);
	static ItemFeature *getLight(float range, const TCODColor &color, float patternDelay, const char *pattern);
	static ItemFeature *getAttack(AttackWieldType wield, float minCastDelay, float maxCastDelay, float minReloadDelay,
		float maxReloadDelay, float minDamagesCoef, float maxDamagesCoef, int flags );
	static ItemFeature *getHeat(float intensity, float radius);
	static ItemFeature *getContainer(int size);

};


enum InventoryTabId {
	INV_ALL,
	INV_ARMOR,
	INV_WEAPON,
	INV_FOOD,
	INV_MISC,
	NB_INV_TABS };

// data shared by all item types
struct ItemType {
	ItemTypeId id;
	InventoryTabId inventoryTab;
	const char *name; // name displayed to the player
	TCODColor color; // color on screen
	int character; // character on screen
	bool an; // an <name> or a <name> ?
	int flags;
	unsigned long long tags;
	TCODList<ItemFeature *>features;
	TCODList<ItemActionId> actions;
	ItemFeature *getFeature(ItemFeatureId id) const;
	bool isA(unsigned long long tag) const { return (tags & tag) != 0; }
	bool hasAction(ItemActionId id_) const { return actions.contains(id_); }
	bool hasComponents() const;
	ItemCombination *getCombination() const;
	void computeActions();
};

class Item : public DynamicEntity {
public :
	// factories
	static Item *getItem(const char *type, float x, float y, bool createComponents=true);
	static Item *getItem(const ItemType *type, float x, float y, bool createComponents=true);
	static Item *getItem(ItemTypeId type, float x, float y, bool createComponents=true);
	static Item *getRandomWeapon(ItemTypeId id,ItemClass itemClass);


	static bool init();
	static ItemType *getType(const char *name);
	static ItemType *getType(ItemTypeId id);
	static ItemType *getTypeFromTag(unsigned long long tag);

	virtual ~Item();
	virtual void render(LightMap *lightMap);
	virtual void renderDescription(int x, int y, bool below=true);
	virtual void renderGenericDescription(int x, int y, bool below=true);
	virtual bool age(float elapsed); // the item gets older
	virtual bool update(float elapsed, TCOD_key_t key, TCOD_mouse_t *mouse);
	ItemFeature *getFeature(ItemFeatureId featureId) { return typeData->getFeature(featureId); }

	virtual Item *pickup(Creature *owner, const char *verb="pick up"); // move the item from ground to owner's inventory
	virtual void use(); // use the item (depends on the type)
	virtual void use(int /* dx */, int /* dy */) {} // use the item in place (static items)
	virtual Item *drop(); // move the item from it's owner inventory to the ground
	bool isEquiped();

	// add to the list, posibly stacking
	Item * addToList(TCODList<Item *> *list);
	// remove one item, possibly unstacking
	Item * removeFromList(TCODList<Item *> *list, bool fast=true);

    void addModifier(ItemModifierId id, float value);

	bool isA(unsigned long long tag) const { return typeData->isA(tag); }
	bool isA(ItemTypeId id) const { return typeData->id == id; }
	bool isA(const ItemType *type) const { return typeData == type; }

	// flags checks
	bool isWalkable() const { return (typeData->flags & ITEM_NOT_WALKABLE) == 0; }
	bool isTransparent() const { return (typeData->flags & ITEM_NOT_TRANSPARENT) ==0; }
	bool isPickable() const { return (typeData->flags & ITEM_NOT_PICKABLE) == 0 ; }
	bool isStackable() const { return (typeData->flags & ITEM_STACKABLE ) != 0 && name == NULL; }
	bool isSoftStackable() const { return (typeData->flags & ITEM_SOFT_STACKABLE) != 0 && name == NULL ; }
	bool isDeletedOnUse() const { return typeData->flags & ITEM_DELETE_ON_USE; }
	bool isUsedWhenPicked() const { return typeData->flags & ITEM_USE_WHEN_PICKED; }
	bool isActivatedOnBump() const { return typeData->flags & ITEM_ACTIVATE_ON_BUMP; }

	// containers
	bool putInside(Item *it); // put it in this container (false if no more room)
	bool pullOutside(Item *it); // remove it from this container (false if not inside)

    // returns "A/An <item name>"
    const char *AName() const;
    // returns "a/an <item name>"
    const char *aName() const;
    // returns "The <item name>"
    const char *TheName() const;
    // returns "the <item name>"
    const char *theName() const;

    const char *getRateName(float rate) const;

	virtual bool loadData(uint32_t chunkId, uint32_t chunkVersion, TCODZip *zip);
	virtual void saveData(uint32_t chunkId, TCODZip *zip);

	static TCODColor classColor[NB_ITEM_CLASSES];

	//crafting
	static ItemCombination combinations[];
	static ItemCombination *getCombination(const Item *it1, const Item *it2);
	bool hasComponents() const;
	void addComponent(Item *component);
	ItemCombination *getCombination() const;


	const ItemType *typeData;
	ItemClass itemClass;
	TCODColor col;
	char *typeName;
	char *name;
	bool an;
	int count;
	TCODList<ItemModifier *> modifiers;
	Creature *owner; // this is in owner's inventory
	Item *container; // this is inside container
	Creature *asCreature; // a creature corresponding to this item
	float fireResistance;
	bool toDelete;
	int ch;
	TCODList<Item *> stack; // for soft stackable items or containers
	TCODList<Item *> components; // for items that can be disassembled
protected :
	static void addFeature(ItemTypeId id, ItemFeature *feat);
	static TCODList<ItemType *>types;
	Item(float x,float y,const ItemType &type);
	friend class Dungeon;
	bool active;
	float life; // remaining time before aging effect turn this item into something else
	// attack feature data
	WeaponPhase phase;
	float phaseTimer;
	float castDelay;
	float reloadDelay;
	float damages;
	float cumulatedElapsed;
	int targetx,targety;
	float heatTimer; // time before next heat update (1 per second)
	bool onoff; // for doors, torchs, ... on = open/turned on, off = closed/turned off

	ExtendedLight *light;
	static TCODConsole *descCon; // offscreen console for item description

	void initLight();
	void convertTo(ItemType *type);
	void renderDescriptionFrame(int x, int y, bool below=true);
	void generateComponents();
};



