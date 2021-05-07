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

#include <stdio.h>
#include <math.h>
#include "main.hpp"

TCODConsole *Item::descCon=NULL;

TCODList<ItemActionId> featureActions[NB_ITEM_FEATURES];

TCODList<ItemType *>Item::types;
// text generator for item names
static TextGenerator *textgen=NULL;

static ItemAction itemActions[NB_ITEM_ACTIONS] = {
	{"Take", ITEM_ACTION_LOOT },
	{"Use", ITEM_ACTION_LOOT|ITEM_ACTION_INVENTORY },
	{"Drop", ITEM_ACTION_INVENTORY },
	{"Throw", ITEM_ACTION_LOOT|ITEM_ACTION_INVENTORY },
	{"Disassemble", ITEM_ACTION_LOOT|ITEM_ACTION_INVENTORY },
};

ItemAction *ItemAction::getFromId(ItemActionId id) {
	return &itemActions[id];
}

ItemCombination Item::combinations[] = {
	{ITEM_WOODEN_HANDLE, {ITEM_CUT,NB_ITEMS,1,false,false}, 1,{
		{ITEM_WOOD,NB_ITEMS,1,false,true,false}
	}},
	{ITEM_KNIFE, {0,NB_ITEMS}, 2, {
		{ITEM_HANDLE,NB_ITEMS,1,false,true,true},
		{ITEM_SMALL_BLADE,NB_ITEMS,1,false,true,true}
	}},
	{NB_ITEMS} // resultType == NB_ITEMS indicates the end of the array
};

TCODColor Item::classColor[NB_ITEM_CLASSES] = {
	TCODColor::white,
	TCODColor::chartreuse,
	TCODColor::orange,
	TCODColor::red,
	TCODColor::lightGrey,
	TCODColor::lightYellow,
};

ItemColor& ItemColor::operator=(const TCODColor& col) {
	r=col.r;
	g=col.g;
	b=col.b;
	return *this;
}

ItemFeature *ItemFeature::getProduce(float delay, float chance, ItemType *type) {
	ItemFeature *ret=new ItemFeature();
	ret->id = ITEM_FEAT_PRODUCES;
	ret->produce.delay=delay;
	ret->produce.chance=chance;
	ret->produce.type=type;
	return ret;
}

ItemFeature *ItemFeature::getFireEffect(float resistance, ItemType *type) {
	ItemFeature *ret=new ItemFeature();
	ret->id = ITEM_FEAT_FIRE_EFFECT;
	ret->fireEffect.resistance=resistance;
	ret->fireEffect.type=type;
	return ret;
}

ItemFeature *ItemFeature::getAgeEffect(float delay, ItemType *type) {
	ItemFeature *ret=new ItemFeature();
	ret->id = ITEM_FEAT_AGE_EFFECT;
	ret->ageEffect.delay=delay;
	ret->ageEffect.type=type;
	return ret;
}

ItemFeature *ItemFeature::getFood(int health) {
	ItemFeature *ret=new ItemFeature();
	ret->id = ITEM_FEAT_FOOD;
	ret->food.health=health;
	return ret;
}

ItemFeature *ItemFeature::getLight(float range, const TCODColor &color, float patternDelay, const char *pattern) {
	ItemFeature *ret=new ItemFeature();
	ret->id = ITEM_FEAT_LIGHT;
	ret->light.range=range;
	ret->light.color=color;
	ret->light.patternDelay=patternDelay;
	ret->light.pattern=strdup(pattern);
	return ret;
}

ItemFeature *ItemFeature::getAttack(AttackWieldType wield, float minCastDelay, float maxCastDelay, float minReloadDelay, 
		float maxReloadDelay, float minDamagesCoef, float maxDamagesCoef, int flags ) {
	ItemFeature *ret=new ItemFeature();
	ret->id = ITEM_FEAT_ATTACK;
	ret->attack.wield=wield;
	ret->attack.minCastDelay=minCastDelay;
	ret->attack.maxCastDelay=maxCastDelay;
	ret->attack.minReloadDelay=minReloadDelay;
	ret->attack.maxReloadDelay=maxReloadDelay;
	ret->attack.minDamagesCoef=minDamagesCoef;
	ret->attack.maxDamagesCoef=maxDamagesCoef;
	ret->attack.flags=flags;
	return ret;
}

ItemFeature *ItemFeature::getHeat(float intensity, float radius) {
	ItemFeature *ret=new ItemFeature();
	ret->id = ITEM_FEAT_HEAT;
	ret->heat.intensity=intensity;
	ret->heat.radius=radius;
	return ret;
}

ItemFeature *ItemFeature::getContainer(int size) {
	ItemFeature *ret=new ItemFeature();
	ret->id = ITEM_FEAT_CONTAINER;
	ret->container.size=size;
	return ret;
}

ItemFeature *ItemType::getFeature(ItemFeatureId id) const {
	for (ItemFeature **it=features.begin(); it!=features.end(); it++) {
		if ( (*it)->id == id ) return *it;
	}
	return NULL;
}

void Item::addFeature(ItemTypeId id, ItemFeature *feat) {
	Item::getType(id)->features.push(feat);
}

bool Item::init() {
	static ItemType internalType[NB_ITEMS] = {
		{NB_ITEMS,INV_MISC,"oak twig", TCODColor::darkAmber, '/', true, ITEM_STACKABLE, ITEM_WOOD },
		{NB_ITEMS,INV_MISC,"pine twig", TCODColor::darkYellow, '/', false, ITEM_STACKABLE, ITEM_WOOD },
		{NB_ITEMS,INV_MISC,"campfire", TCODColor::lightYellow, '^', false, ITEM_NOT_PICKABLE,0 },
 		{NB_ITEMS,INV_MISC,"scroll", TCODColor(255,127,127), '#', false, ITEM_DELETE_ON_USE|ITEM_USE_WHEN_PICKED, 0},

		{NB_ITEMS,INV_MISC,"door", TCODColor::darkYellow, '+', false, ITEM_NOT_PICKABLE|ITEM_ACTIVATE_ON_BUMP|ITEM_NOT_WALKABLE|ITEM_NOT_TRANSPARENT, ITEM_TAG_DOOR|ITEM_BUILD_NOT_BLOCK},
 		{NB_ITEMS,INV_MISC,"wall", TCODColor::grey, '-', false, ITEM_NOT_PICKABLE|ITEM_NOT_WALKABLE|ITEM_NOT_TRANSPARENT, 0},
 		{NB_ITEMS,INV_MISC,"window", TCODColor::lightBlue, TCOD_CHAR_HLINE, false, ITEM_NOT_PICKABLE|ITEM_NOT_WALKABLE, ITEM_BUILD_NOT_BLOCK},

 		{NB_ITEMS,INV_MISC,"chest", TCODColor::darkerYellow, ']', false, ITEM_CONTAINER|ITEM_ACTIVATE_ON_BUMP|ITEM_NOT_PICKABLE|ITEM_NOT_WALKABLE, 0},
 		{NB_ITEMS,INV_MISC,"bag", TCODColor::desaturatedYellow, ']', false, ITEM_CONTAINER, 0},

 		{NB_ITEMS,INV_MISC,"wooden handle", TCODColor::darkAmber, ')', false, ITEM_STACKABLE, ITEM_HANDLE},
 		{NB_ITEMS,INV_MISC,"small bronze blade", TCODColor::brass, ')', false, ITEM_STACKABLE, ITEM_SMALL_BLADE},

		{NB_ITEMS,INV_MISC,"pine", TCODColor::darkAmber, 'T', false, ITEM_NOT_PICKABLE|ITEM_NOT_WALKABLE|ITEM_NOT_TRANSPARENT,ITEM_TREE}, 
		{NB_ITEMS,INV_MISC,"oak", TCODColor::darkAmber, 'T', true, ITEM_NOT_PICKABLE|ITEM_NOT_WALKABLE|ITEM_NOT_TRANSPARENT,ITEM_TREE }, 
		{NB_ITEMS,INV_MISC,"apple tree", TCODColor::darkAmber, 'T', true, ITEM_NOT_PICKABLE|ITEM_NOT_WALKABLE|ITEM_NOT_TRANSPARENT,ITEM_TREE}, 

		{NB_ITEMS,INV_FOOD,"health potion",TCODColor(127,127,255),'!', false, ITEM_DELETE_ON_USE|ITEM_STACKABLE,0},
		{NB_ITEMS,INV_FOOD,"apple",TCODColor(255,92,92),'a', true, ITEM_DELETE_ON_USE|ITEM_SOFT_STACKABLE,0},
		{NB_ITEMS,INV_FOOD,"rotten apple",TCODColor(127,31,0),'%', false, ITEM_DELETE_ON_USE|ITEM_SOFT_STACKABLE,0},
		{NB_ITEMS,INV_FOOD,"living fish",TCODColor::desaturatedSky,'f', false, ITEM_DELETE_ON_USE|ITEM_SOFT_STACKABLE,0},
		{NB_ITEMS,INV_FOOD,"uncooked fish",TCODColor::desaturatedSky,'f', true, ITEM_DELETE_ON_USE|ITEM_SOFT_STACKABLE,0},
		{NB_ITEMS,INV_FOOD,"cooked fish",TCODColor::desaturatedSky,'f', false, ITEM_DELETE_ON_USE|ITEM_SOFT_STACKABLE,0},
		{NB_ITEMS,INV_FOOD,"smoked fish",TCODColor::desaturatedSky,'f', false, ITEM_DELETE_ON_USE|ITEM_SOFT_STACKABLE,0},
		{NB_ITEMS,INV_FOOD,"rotten fish",TCODColor(63,127,95),'%', false, ITEM_DELETE_ON_USE|ITEM_SOFT_STACKABLE,0},

		{NB_ITEMS, INV_WEAPON,"knife",TCODColor::white,'/', false, 0,ITEM_CUT},
		{NB_ITEMS, INV_WEAPON,"stone",TCODColor::lightGrey,'.', false, ITEM_STACKABLE,0},
		{NB_ITEMS, INV_WEAPON,"wand",TCODColor::white,'/', false, 0,0},
		{NB_ITEMS, INV_WEAPON,"staff",TCODColor::white,'/', false, 0,0},
	};
	// initialize item types list
	for (int i=0; i < NB_ITEMS; i++) {
		internalType[i].id=(ItemTypeId)i;
		types.push(&internalType[i]);
	}
	// add features to item types
	addFeature(ITEM_OAK_TWIG,ItemFeature::getFireEffect(1.0f,getType(ITEM_CAMPFIRE)));
	addFeature(ITEM_PINE_TWIG,ItemFeature::getFireEffect(1.0f,getType(ITEM_CAMPFIRE)));
	addFeature(ITEM_CAMPFIRE,ItemFeature::getHeat(1.0f,1.5f));
	addFeature(ITEM_SCROLL,ItemFeature::getLight(4.0f,TCODColor(76,38,38),1.0f,"56789876"));
	addFeature(ITEM_PINE,ItemFeature::getProduce(600,0.2f,getType(ITEM_PINE_TWIG)));
	addFeature(ITEM_OAK,ItemFeature::getProduce(600,0.2f,getType(ITEM_OAK_TWIG)));
	addFeature(ITEM_APPLE_TREE,ItemFeature::getProduce(600,0.2f,getType(ITEM_APPLE)));
	addFeature(ITEM_HEALTH_POTION,ItemFeature::getFood(15));
	addFeature(ITEM_HEALTH_POTION,ItemFeature::getLight(4.0f,TCODColor(38,38,76),1.0f,"56789876"));
	addFeature(ITEM_APPLE,ItemFeature::getFood(3));
	addFeature(ITEM_APPLE,ItemFeature::getAgeEffect(600,getType(ITEM_ROTTEN_APPLE)));
	addFeature(ITEM_ROTTEN_APPLE,ItemFeature::getFood(1));
	addFeature(ITEM_ROTTEN_APPLE,ItemFeature::getAgeEffect(600,NULL));
	addFeature(ITEM_LIVING_FISH,ItemFeature::getFood(1));
	addFeature(ITEM_LIVING_FISH,ItemFeature::getAgeEffect(20,getType(ITEM_UNCOOKED_FISH)));
	addFeature(ITEM_LIVING_FISH,ItemFeature::getFireEffect(5.0f,getType(ITEM_COOKED_FISH)));
	addFeature(ITEM_UNCOOKED_FISH,ItemFeature::getFood(2));
	addFeature(ITEM_UNCOOKED_FISH,ItemFeature::getAgeEffect(300,getType(ITEM_ROTTEN_FISH)));
	addFeature(ITEM_UNCOOKED_FISH,ItemFeature::getFireEffect(5.0f,getType(ITEM_COOKED_FISH)));
	addFeature(ITEM_COOKED_FISH,ItemFeature::getFood(10));
	addFeature(ITEM_COOKED_FISH,ItemFeature::getAgeEffect(600,getType(ITEM_ROTTEN_FISH)));
	addFeature(ITEM_COOKED_FISH,ItemFeature::getFireEffect(15.0f,NULL));
	addFeature(ITEM_SMOKED_FISH,ItemFeature::getFood(10));
	addFeature(ITEM_SMOKED_FISH,ItemFeature::getAgeEffect(1200,getType(ITEM_ROTTEN_FISH)));
	addFeature(ITEM_SMOKED_FISH,ItemFeature::getFireEffect(15.0f,NULL));
	addFeature(ITEM_ROTTEN_FISH,ItemFeature::getFood(2));
	addFeature(ITEM_ROTTEN_FISH,ItemFeature::getAgeEffect(600,NULL));
	addFeature(ITEM_ROTTEN_FISH,ItemFeature::getFireEffect(3.0f,NULL));
	addFeature(ITEM_KNIFE,ItemFeature::getAttack(WIELD_ONE_HAND, 0.1f,0.2f, 0.2f,0.3f, 0.1f,0.3f,0));
	addFeature(ITEM_STONE,ItemFeature::getAttack(WIELD_MAIN_HAND, 1.0f,1.0f, 0.5f,2.0f, 0.2f,0.4f,WEAPON_PROJECTILE));
	addFeature(ITEM_WAND,ItemFeature::getAttack(WIELD_ONE_HAND, 0.2f,0.3f, 0.6f,0.8f, 0.8f,1.2f, 0));
	addFeature(ITEM_WAND,ItemFeature::getLight(4.0f,TCODColor::white,1.0f,"56789876"));
	addFeature(ITEM_STAFF,ItemFeature::getAttack(WIELD_TWO_HANDS, 0.25f,1.0f, 0.5f,2.0f, 0.8f,1.2f, 0));
	addFeature(ITEM_STAFF,ItemFeature::getLight(4.0f,TCODColor::white,1.0f,"56789876"));
	addFeature(ITEM_CHEST,ItemFeature::getContainer(10));
	addFeature(ITEM_BAG,ItemFeature::getContainer(5));
	// define available action on each item type
	for (int i=0; i < NB_ITEMS; i++) {
		internalType[i].computeActions();
	}
	return true;
}

void ItemType::computeActions() {
	if ( (flags & ITEM_NOT_PICKABLE) == 0 ) actions.push(ITEM_ACTION_TAKE);
	if ( getFeature(ITEM_FEAT_FOOD) || getFeature(ITEM_FEAT_ATTACK) ) actions.push(ITEM_ACTION_USE);
	if ( (flags & ITEM_NOT_PICKABLE) == 0 ) actions.push(ITEM_ACTION_DROP);
	if ( (flags & ITEM_NOT_PICKABLE) == 0 ) actions.push(ITEM_ACTION_THROW);
	if ( hasComponents() ) actions.push(ITEM_ACTION_DISASSEMBLE);
}

ItemType *Item::getType(const char *name) {
	if ( types.size() == 0 ) {
		if (! init()) exit(0); // fatal error. cannot load items configuration
	}
	for (ItemType **it=types.begin(); it!=types.end(); it++) {
		if ( strcmp((*it)->name,name) == 0 ) return *it;
	}
	return NULL;
}

ItemType *Item::getType(ItemTypeId id) { 
	if ( types.size() == 0 ) {
		if (! init()) exit(0); // fatal error. cannot load items configuration
	}
	return types.get(id); 
}

bool ItemType::hasComponents() const {
	ItemCombination *cur=Item::combinations;
	while (cur->resultType != NB_ITEMS) {
		if (cur->resultType == id) {
			// check if ingredients can be reverted
			for (int i=0; i < cur->nbIngredients; i++) {
				if ( cur->ingredients[i].revert ) return true;
			}
		}
		cur++;
	}
	return false;
}

ItemType *Item::getTypeFromTag(unsigned long long tag) {
	int i = rng->getInt(0,types.size()-1);
	int count=types.size();
	while ( count > 0 && !types.get(i)->isA(tag) ) {
		i = (i+1)%types.size();
		count --;
	}
	if ( count == 0 ) return NULL;
	return types.get(i);
}

ItemCombination *ItemType::getCombination() const {
	ItemCombination *cur=Item::combinations;
	while (cur->resultType != NB_ITEMS) {
		if (cur->resultType == id) return cur;
		cur++;
	}
	return NULL;
}

Item::Item(float x,float y, const ItemType &type):
	active(false)  {
	if (descCon == NULL) {
		descCon=new TCODConsole(CON_W/2,CON_H/2);
		descCon->setAlignment(TCOD_CENTER);
		descCon->setDefaultBackground(guiBackground);
	}
	setPos(x,y);
	light=NULL;
	count=1;
	owner=NULL;
	container=NULL;
	asCreature=NULL;
	typeName=strdup(type.name);
	name=NULL;
	typeData=&type;
	toDelete=false;
	ItemFeature *feat=getFeature(ITEM_FEAT_LIGHT);
	if ( feat ) {
		initLight();
		light->range=feat->light.range;
		light->color=TCODColor(feat->light.color.r,feat->light.color.g,feat->light.color.b);
		light->setup(light->color,feat->light.patternDelay,feat->light.pattern,NULL);
	}
	col=type.color;
	ch=type.character;
	itemClass=ITEM_CLASS_STANDARD;
	feat=typeData->getFeature(ITEM_FEAT_AGE_EFFECT);
	if ( feat ) life = feat->ageEffect.delay;
	else life=0.0f;
	feat=typeData->getFeature(ITEM_FEAT_FIRE_EFFECT);
	if ( feat ) fireResistance = feat->fireEffect.resistance;
	else fireResistance=0.0f;
	feat=typeData->getFeature(ITEM_FEAT_ATTACK);
	castDelay=reloadDelay=damages=0.0f;
	if ( feat ) {
		castDelay = rng->getFloat(feat->attack.minCastDelay,feat->attack.maxCastDelay);
		reloadDelay = rng->getFloat(feat->attack.minReloadDelay,feat->attack.maxReloadDelay);
		damages = 15 * (reloadDelay + castDelay ) * rng->getFloat(feat->attack.minDamagesCoef,feat->attack.maxDamagesCoef);
	}

	phase=IDLE;
	phaseTimer=0.0f;
	targetx=targety=-1;
	heatTimer=0.0f;
	onoff=false;
	an=false;
}

Item *Item::getItem(const char *typeName, float x, float y, bool createComponents) {
	ItemType *type=Item::getType(typeName);
	if (! type ) {
		printf("FATAL : unknown item type '%s'\n",typeName);
		return NULL;
	}
	return Item::getItem(type,x,y,createComponents);
}

Item *Item::getItem(const ItemType *type, float x, float y, bool createComponents) {
	Item *ret=new Item(x,y,*type);
	if (createComponents) ret->generateComponents();
	return ret;
}

Item *Item::getItem(ItemTypeId typeId, float x, float y, bool createComponents) {
	ItemType *type = Item::getType(typeId);
	return getItem(type,x,y,createComponents);
}

#define MAX_RELOAD_BONUS 0.2f
#define MAX_CAST_BONUS 0.2f
Item *Item::getRandomWeapon(ItemTypeId id,ItemClass itemClass) {
	if (! textgen ) {
		textgen=new TextGenerator("data/cfg/weapon.txg",rng);
		textgen->parseFile();
	}
	Item * weapon = Item::getItem(id,-1,-1,false);
	weapon->itemClass=itemClass;
	weapon->col = Item::classColor[itemClass];
	if ( itemClass > ITEM_CLASS_STANDARD ) {
		int goatKey = 6 * GameEngine::instance->player.school.type + id;
		weapon->name = strdup( textgen->generate("weapon","${%s}",
			textgen->generators.peek()->rules.get(goatKey)->name ));
	}
	enum { MOD_RELOAD, MOD_CAST, MOD_MODIFIER };
	for (int i=0; i < itemClass; i++) {
        int modType = rng->getInt(MOD_RELOAD,MOD_MODIFIER);
        switch(modType) {
            case MOD_RELOAD :
                weapon->reloadDelay -= rng->getFloat(0.05f, MAX_RELOAD_BONUS);
                weapon->reloadDelay = MAX(0.1f,weapon->reloadDelay);
            break;
            case MOD_CAST :
                weapon->castDelay -= rng->getFloat(0.05f, MAX_CAST_BONUS);
                weapon->castDelay = MAX(0.1f,weapon->reloadDelay);
            break;
            case MOD_MODIFIER :
                ItemModifierId id=(ItemModifierId)0;
                switch (GameEngine::instance->player.school.type ) {
                    case SCHOOL_FIRE :
                        id=(ItemModifierId)rng->getInt(ITEM_MOD_FIRE_BEGIN,ITEM_MOD_FIRE_END);
                    break;
                    case SCHOOL_WATER :
                        id=(ItemModifierId)rng->getInt(ITEM_MOD_WATER_BEGIN,ITEM_MOD_WATER_END);
                    break;
                    default:break;
                }
                weapon->addModifier(id,rng->getFloat(0.1f,0.2f));
            break;
        }
	}
	weapon->damages += weapon->damages * (int)(itemClass)*0.2f; // 20% increase per color level
	weapon->damages = MIN(1.0f,weapon->damages);
	// build components
	weapon->generateComponents();
    return weapon;
}

void Item::addComponent(Item *component) {
	components.push(component);
	if ( component->name && ! name ) {
		// inherit name from component
		char *end=strchr(component->name,' ');
		if ( end ) {
			*end=0;
			char buf[128];
			sprintf(buf,"%s %s",component->name,typeName);
			name=strdup(buf);
			*end=' ';
		}
	}
}

void Item::generateComponents() {
	// check if this item type has components
	ItemCombination *combination=getCombination();
	if (! combination) return;
	int maxOptionals=itemClass-ITEM_CLASS_STANDARD;
	int i=rng->getInt(0,combination->nbIngredients-1);
	for (int count=combination->nbIngredients; count > 0 ; count--) {
		if (combination->ingredients[i].revert && 
			(!combination->ingredients[i].optional || maxOptionals > 0 )) {
			if ( combination->ingredients[i].optional ) maxOptionals--;
			ItemType *componentType=NULL; 
			if (combination->ingredients[i].tag) {
				componentType=getTypeFromTag(combination->ingredients[i].tag);
			} else {
				componentType=getType(combination->ingredients[i].type);
			}
			if ( componentType ) {
				Item *component=Item::getItem(componentType,x,y);
				addComponent(component);
			}
		}
		i = (i+1)%combination->nbIngredients;
	}
}

void Item::initLight() {
	light = new ExtendedLight();
	light->x=x*2;
	light->y=y*2;
	light->randomRad=false;
}

Item::~Item() {
	if ( light ) delete light;
	free(typeName);
}

// look for a 2 items recipe
ItemCombination *Item::getCombination(const Item *it1, const Item *it2) {
	ItemCombination *cur=combinations;
	while (cur->resultType != NB_ITEMS) {
		if ( cur->nbIngredients == 1 && (cur->tool.tag || cur->tool.type != NB_ITEMS)) {
			// tool + 1 ingredient
			if ( (it1->isA(cur->tool.tag) || it1->isA(cur->tool.type)) 
				&& (it2->isA(cur->ingredients[0].tag) || it2->isA(cur->ingredients[0].type) ) ) return cur;
			if ( (it2->isA(cur->tool.tag) || it2->isA(cur->tool.type))
				&& (it1->isA(cur->ingredients[0].tag) || it1->isA(cur->ingredients[0].type) ) ) return cur;
		} else if ( cur->nbIngredients == 2 && cur->tool.tag == 0 && cur->tool.type == NB_ITEMS ) {
			// 2 ingredients (no tool)
			if ( (it1->isA(cur->ingredients[0].tag) || it1->isA(cur->ingredients[0].type)) 
				&& (it2->isA(cur->ingredients[1].tag) || it2->isA(cur->ingredients[1].type) ) ) return cur;
			if ( (it2->isA(cur->ingredients[0].tag) || it2->isA(cur->ingredients[0].type))
				&& (it1->isA(cur->ingredients[1].tag) || it1->isA(cur->ingredients[1].type) ) ) return cur;
		}
		cur++;
	}
	return NULL;
}

bool Item::hasComponents() const {
	return typeData->hasComponents();
}

ItemCombination *Item::getCombination() const {
	return typeData->getCombination();
}

// put it in this container (false if no more room)
bool Item::putInside(Item *it) {
	ItemFeature *cont=getFeature(ITEM_FEAT_CONTAINER);
	if ( ! cont ) return false; // this is not a container
	if ( stack.size() >= cont->container.size ) return false; // no more room
	stack.push(it);
	it->container=this;
	it->x=x;
	it->y=y;
	return true;
}
 
// remove it from this container (false if not inside)
bool Item::pullOutside(Item *it) {
	ItemFeature *cont=getFeature(ITEM_FEAT_CONTAINER);
	if ( ! cont ) return false; // this is not a container
	if ( ! stack.contains(it) ) return false;
	stack.remove(it);
	it->container=NULL;
	return true;
} 


// add to the list, posibly stacking
Item * Item::addToList(TCODList<Item *> *list) {
	if (isStackable()) {
		// if already in the list, increase the count
		for (Item **it=list->begin(); it != list->end(); it++) {
			if ( (*it)->typeData == typeData && ! (*it)->name ) {
				(*it)->count ++;
				toDelete=true;
				return *it;
			}
		}
	} else if ( isSoftStackable() ) {
		// if already in the list, add to soft stack
		for (Item **it=list->begin(); it != list->end(); it++) {
			if ( (*it)->typeData == typeData && ! (*it)->name ) {
				(*it)->stack.push(this);
				return *it;
			}
		}
	}
	// add new item in the list
	list->push(this);
	return this;
}
// remove one item, possibly unstacking
Item * Item::removeFromList(TCODList<Item *> *list, bool fast) {
	if (isStackable() && count > 1) {
		Item *newItem=Item::getItem(typeData, x, y);
		count --;
		return newItem;
	} else if ( isSoftStackable() ) {
		if ( stack.size() > 0 ) {
			// this item is the stack owner. rebuild the stack
			Item *newStackOwner=stack.get(0);
			for (int i=1; i < stack.size(); i++) newStackOwner->stack.push(stack.get(i));
			stack.clear();
			// remove before adding to avoid list reallocation
			if (fast) list->removeFast(this);
			else list->remove(this);
			newStackOwner->addToList(list);
			return this;
		} else {
			// this item may be in a stack. find the stack owner
			for (Item **stackOwner=list->begin(); stackOwner!=list->end(); stackOwner++) {
				if ((*stackOwner) != this && (*stackOwner)->typeData == typeData ) {
					(*stackOwner)->stack.removeFast(this);
					return this;
				}
			}
		}
	} else if ( container && list->contains(container)) {
		// item is inside a container
		container->pullOutside(this);		
	}
	if (fast) list->removeFast(this);
	else list->remove(this);
	return this;
}


void Item::render(LightMap *lightMap) {
	int conx=(int)(x-GameEngine::instance->xOffset);
	int cony=(int)(y-GameEngine::instance->yOffset);
	if ( !IN_RECTANGLE(conx,cony,CON_W,CON_H) ) return;
	Dungeon *dungeon=GameEngine::instance->dungeon;
	TCODColor lightColor=lightMap->getColor(conx,cony);
	float shadow = dungeon->getShadow(x*2,y*2);
	float clouds = dungeon->getCloudCoef(x*2,y*2);
	shadow = MIN(shadow,clouds); 
	lightColor = lightColor * shadow; 
	TCODConsole::root->setChar(conx,cony,ch);
	TCODConsole::root->setCharForeground(conx,cony,col*lightColor);
	TCODConsole::root->setCharBackground(conx,cony,dungeon->getShadedGroundColor(getSubX(),getSubY()));

}

void Item::renderDescription(int x, int y, bool below) {
	int cy=0;
	descCon->clear();
	descCon->setDefaultForeground(Item::classColor[itemClass]);
	if (name) {
		descCon->print(CON_W/4,cy++,name);
		descCon->setDefaultForeground(guiText);
	}
	descCon->print(CON_W/4,cy++,typeName);
	descCon->setDefaultForeground(guiText);
	ItemFeature *feat=getFeature(ITEM_FEAT_FOOD);
	if ( feat ) descCon->print(CON_W/4,cy++,"Health:+%d",feat->food.health);
	feat = getFeature(ITEM_FEAT_ATTACK);
	if ( feat ) {
		static const char *wieldname[] = {
			NULL, "One hand", "Main hand", "Off hand", "Two hands"
		};
		if ( feat->attack.wield ) {
			descCon->print(CON_W/4,cy++,wieldname[feat->attack.wield]);
		}
		float rate=1.0f / (castDelay + reloadDelay);
		int dmgPerSec = (int)(damages *rate + 0.5f);
		descCon->print(CON_W/4,cy++,"%d damages/sec", dmgPerSec);
		descCon->print(CON_W/4,cy++,"Attack rate:%s",getRateName(rate));
		ItemModifier::renderDescription(descCon,2,cy,modifiers);
	}

/*
	y--;
	if ( y < 0 ) y = 2;
	TCODConsole::root->setDefaultForeground(Item::classColor[itemClass]);
	TCODConsole::root->printEx(x,y,TCOD_BKGND_NONE,TCOD_CENTER,typeName);
*/
	renderDescriptionFrame(x,y,below);
}

const char *Item::getRateName(float rate) const {
	static const char *ratename[] = {
		"Very slow",
		"Slow",
		"Average",
		"Fast",
		"Very fast"
	};
	int rateIdx=0;
	if ( rate <= 0.5f ) rateIdx = 0;
	else if ( rate <= 1.0f ) rateIdx = 1;
	else if ( rate <= 3.0f ) rateIdx = 2;
	else if ( rate <= 5.0f ) rateIdx = 3;
	else rateIdx = 4;
	return ratename[rateIdx];	
}

void Item::renderGenericDescription(int x, int y, bool below) {
	int cy=0;
	descCon->clear();
	descCon->setDefaultForeground(Item::classColor[itemClass]);
	if (name) {
		descCon->print(CON_W/4,cy++,name);
		descCon->setDefaultForeground(guiText);
	}
	descCon->print(CON_W/4,cy++,typeName);
	descCon->setDefaultForeground(guiText);
	ItemFeature *feat=getFeature(ITEM_FEAT_FOOD);
	if ( feat ) descCon->print(CON_W/4,cy++,"Health:+%d",feat->food.health);
	feat = getFeature(ITEM_FEAT_ATTACK);
	if ( feat ) {
		static const char *wieldname[] = {
			NULL, "One hand", "Main hand", "Off hand", "Two hands"
		};
		if ( feat->attack.wield ) {
			descCon->print(CON_W/4,cy++,wieldname[feat->attack.wield]);
		}
		float minCast=feat->attack.minCastDelay - itemClass*MAX_CAST_BONUS;
		float minReload=feat->attack.minReloadDelay - itemClass*MAX_RELOAD_BONUS;
		float minDamages = 15 * (minCast + minReload ) * feat->attack.minDamagesCoef;
		float maxDamages = 15 * (feat->attack.maxCastDelay + feat->attack.maxReloadDelay ) * feat->attack.maxDamagesCoef;
    	minDamages += minDamages * (int)(itemClass)*0.2f;
    	maxDamages += maxDamages * (int)(itemClass)*0.2f;
    	minDamages=(int)MIN(1.0f,minDamages);
    	maxDamages=(int)MIN(1.0f,maxDamages);

		if ( minDamages != maxDamages ) {		
			descCon->print(CON_W/4,cy++,"%d-%d damages/hit", (int)minDamages,(int)maxDamages);
		} else {
			descCon->print(CON_W/4,cy++,"%d damages/hit", (int)minDamages);
		}

		float minRate=1.0f / (feat->attack.maxCastDelay + feat->attack.maxReloadDelay);
		float maxRate=1.0f / (minCast + minReload);

		const char *rate1=getRateName(minRate);
		const char *rate2=getRateName(maxRate);
		if ( rate1 == rate2 ) {
			descCon->print(CON_W/4,cy++,"Attack rate:%s",rate1);
		} else {
			descCon->print(CON_W/4,cy++,"Attack rate:%s-%s",rate1,rate2);
		}
		//ItemModifier::renderDescription(descCon,2,cy,modifiers);
	}

	renderDescriptionFrame(x,y,below);
}

void Item::convertTo(ItemType *newType) {
	// create the new item
	Item *newItem=Item::getItem(newType, x, y);
	newItem->speed = speed;
	newItem->dx = dx;
	newItem->dy = dy;
	if ( owner ) {
		if ( owner->isPlayer() ) {
			if ( asCreature ) GameEngine::instance->log.info("%s died.",TheName());
			else {
				if ( strncmp(newType->name,"rotten",6) == 0 ) {
					GameEngine::instance->log.info("%s has rotted.",TheName());
				} else {
					GameEngine::instance->log.info("%s turned into %s.",TheName(), newItem->aName());
				}
			}
		}
		owner->addToInventory(newItem);
	} else GameEngine::instance->dungeon->addItem(newItem);	
}

bool Item::age(float elapsed) {
	ItemFeature *feat=typeData->getFeature(ITEM_FEAT_AGE_EFFECT);
	if ( feat ) {
		life -= elapsed;
		if (life <= 0.0f ) {
			if ( feat->ageEffect.type ) {
				convertTo(feat->ageEffect.type);
			}
			// destroy this item
			if (! owner ) {
				if ( asCreature ) GameEngine::instance->dungeon->removeCreature(asCreature,false);
			} else {
				if ( asCreature ) asCreature->toDelete=true;
			}
			return false;
		}
	}
	return true;
}

bool Item::update(float elapsed, TCOD_key_t key, TCOD_mouse_t *mouse) {
	Dungeon *dungeon=GameEngine::instance->dungeon;
	if (! isOnScreen() ) {
		// when not on screen, update only once per second
		cumulatedElapsed += elapsed;
		if (cumulatedElapsed < 1.0f ) return true;
		elapsed=cumulatedElapsed;
		cumulatedElapsed=0.0f;
	}
	if (!age(elapsed)) return false;
	if ( speed > 0.0f ) {
		float oldx=x;
		float oldy=y;
		x += speed*dx*elapsed;
		y += speed*dy*elapsed;
		if (! dungeon->isCellWalkable(x,y)) {
			// bounce against a wall
			float newx=x;
			float newy=y;
			int cdx=(int)(x-oldx);
			int cdy=(int)(y-oldy);
			x=oldx;y=oldy;
			speed *= 0.5f;
			if ( cdx == 0 ) {
				// hit horizontal wall
				dy=-dy;
			} else if (cdy == 0 ) {
				// hit vertical wall
				dx=-dx;
			} else {
				bool xwalk=dungeon->isCellWalkable(newx,oldy);
				bool ywalk=dungeon->isCellWalkable(oldx,newy);
				if ( xwalk && ywalk ) {
					// outer corner bounce. detect which side of the cell is hit
					//  ##
					//  ##
					// .
					float fdx=ABS(dx);
					float fdy=ABS(dy);
					if ( fdx >= fdy ) dy=-dy;
					if ( fdy >= fdx ) dx=-dx;
				} else if (! xwalk ) {
					if ( ywalk ) {
						// vertical wall bounce
						dx=-dx;
					} else {
						// inner corner bounce
						// ##
						// .#
						dx=-dx;dy=-dy;
					}
				} else {
					// horizontal wall bounce
					dy=-dy;
				}
			}			
		}
		if ( x != oldx || y != oldy ) {
			dungeon->getCell(oldx,oldy)->items.removeFast(this);
			dungeon->getCell(x,y)->items.push(this);
		}
		duration -= elapsed;
		if ( duration < 0.0f ) {
			speed=0.0f;
			if ( dungeon->hasRipples(x,y) ) {
				GameEngine::instance->startRipple(x,y);
			}
		}
	}
	ItemFeature *feat=getFeature(ITEM_FEAT_ATTACK);
	if (feat) {
		switch (phase) {
		case CAST :
			phaseTimer -= elapsed;
			if ( phaseTimer <= 0.0f && (feat->attack.flags & WEAPON_PROJECTILE) == 0 ) {
				phase=RELOAD;
				phaseTimer=reloadDelay;
				FireBall *fb=new FireBall(owner->x,owner->y,targetx,targety,FB_STANDARD);
				((Game *)GameEngine::instance)->addFireball(fb);
				GameEngine::instance->stats.nbSpellStandard++;
			} else {
				if ( feat->attack.flags & WEAPON_PROJECTILE ) {
					// keep targetting while the mouse button is pressed
					int dx=mouse->cx+GameEngine::instance->xOffset;
					int dy=mouse->cy+GameEngine::instance->yOffset;
					targetx=dx;
					targety=dy;
					if ( !mouse->lbutton ) {
						// fire when mouse button released
						phaseTimer=MAX(phaseTimer,0.0f);
						float speed=(castDelay-phaseTimer)/castDelay;
						speed=MIN(speed,1.0f);
						phase=RELOAD;
						phaseTimer=reloadDelay;
						if ( (int)targetx == (int)owner->x && (int)targety == (int)owner->y ) return true;
						x=owner->x;
						y=owner->y;
						Item *it=owner->removeFromInventory(this);
						it->dx = targetx - x;
						it->dy = targety - y;
						float l=fastInvSqrt(it->dx*it->dx+it->dy*it->dy);
						it->dx*=l;
						it->dy*=l;
						it->x = x;
						it->y = y;
						it->speed=speed*12;
						it->duration=1.5f;
						GameEngine::instance->dungeon->addItem(it);
					}
				}
			}
			break;
		case RELOAD :
			phaseTimer -= elapsed;
			if ( phaseTimer <= 0.0f ) {
				phase=IDLE;
			}
			break;
		case IDLE:
			if ( owner->isPlayer() && mouse->lbutton && isEquiped() ) {
				phaseTimer=castDelay;
				phase=CAST;
				int dx=mouse->cx+GameEngine::instance->xOffset;
				int dy=mouse->cy+GameEngine::instance->yOffset;
				targetx=dx;
				targety=dy;
			}
		break;	
		}
	}
	feat=getFeature(ITEM_FEAT_HEAT);
	if (feat) {	
		heatTimer += elapsed;
		if ( heatTimer > 1.0f) {
			// warm up adjacent items
			heatTimer = 0.0f;
			float radius=feat->heat.radius;
			for (int tx=-(int)floor(radius); tx <= (int)ceil(radius); tx++) {
				if ( (int)(x)+tx >= 0 && (int)(x)+tx < dungeon->size ) {
					int dy=(int)(sqrtf(radius*radius - tx*tx));
					for (int ty=-dy; ty <= dy; ty++) {
						if ( (int)(y)+ty >= 0 && (int)(y)+ty < dungeon->size ) {
							TCODList<Item *> *items=dungeon->getItems((int)(x)+tx,(int)(y)+ty);
							for ( Item **it=items->begin(); it!=items->end(); it++) {
								// found an adjacent item
								ItemFeature *fireFeat=(*it)->getFeature(ITEM_FEAT_FIRE_EFFECT);
								if ( fireFeat ) {
									// item is affected by fire
									(*it)->fireResistance -= feat->heat.intensity;
								}
							}
						}
					}
				}
			}
		}
	}
	feat=getFeature(ITEM_FEAT_FIRE_EFFECT);
	if (feat && fireResistance <= 0.0f ) {
		if ( feat->fireEffect.type ) {
			convertTo(feat->fireEffect.type);
		}
		// destroy this item
		if (! owner ) {
			if ( asCreature ) GameEngine::instance->dungeon->removeCreature(asCreature,false);
		} else {
			if ( asCreature ) asCreature->toDelete=true;
		}
		return false;
	}
	return true;
}

bool Item::isEquiped() {
	return ( owner && (owner->mainHand == this || owner->offHand == this) );
}

void Item::use() {
	active=true;
	ItemFeature *feat=getFeature(ITEM_FEAT_PRODUCES);
	if (feat) {
		if ( TCODRandom::getInstance()->getFloat(0.0f,1.0f) < feat->produce.chance ) {
			int fx=(int)x,fy=(int)y;
			GameEngine::instance->dungeon->getClosestWalkable(&fx,&fy,false,false);
			if ( ABS(fx-x) < 2 && ABS(fy-y) < 2 ) {
				// produce some item
				Item *it = Item::getItem(feat->produce.type,fx,fy);
				GameEngine::instance->dungeon->addItem(it);
				GameEngine::instance->log.info("You kick %s. %s falls on the ground.",
					theName(),
					it->AName());				
			}
		} else {
			if ( TCODRandom::getInstance()->getInt(0,2) == 0 ) {
				GameEngine::instance->log.info("You kick %s.",theName());				
				GameEngine::instance->log.warn("You feel a sharp pain in the foot.");
				GameEngine::instance->player.takeDamage(2);
			} else {
				GameEngine::instance->log.info("You kick %s. Nothing happens.",theName());
			}
		}
	}
	feat=getFeature(ITEM_FEAT_FOOD);
	if ( feat ) {
		GameEngine::instance->player.heal(feat->food.health);
		GameEngine::instance->stats.nbEaten[ typeData->id ] ++;
	}
	feat=getFeature(ITEM_FEAT_ATTACK);
	if ( feat && owner ) {
		if ( isEquiped() ) owner->unwield(this);
		else owner->wield(this);
	}
	feat=getFeature(ITEM_FEAT_CONTAINER);
	if ( feat ) {
		GameEngine::instance->openCloseLoot(this);
	}
	if ( isA( ITEM_TAG_DOOR ) ) {
		onoff=!onoff;
		GameEngine::instance->log.info("You %s %s", onoff ? "open":"close", theName());
		ch = onoff ? '/':'+';
		GameEngine::instance->dungeon->setProperties((int)x,(int)y,onoff,onoff);
	}
	if ( isDeletedOnUse() ) {
		if ( count > 1 ) count --;
		else {
			if (! owner ) {
				GameEngine::instance->dungeon->removeItem(this,true);
			} else {
				owner->removeFromInventory(this,true);
			}
		}
	}
}

Item * Item::pickup(Creature *owner, const char *verb) {
	GameEngine::instance->dungeon->removeItem(this,false);
	if ( verb != NULL ) GameEngine::instance->log.info("You %s %s.", verb,aName());
	this->owner=owner;
	Item *ret=owner->addToInventory(this);
	ItemFeature *feat=getFeature(ITEM_FEAT_ATTACK);
	if ( feat ) {
		// auto equip weapon if hand is empty
		if (owner->mainHand == NULL && 
			(feat->attack.wield == WIELD_ONE_HAND || feat->attack.wield == WIELD_MAIN_HAND ) ) {
			owner->mainHand=ret;
		} else if ( owner->offHand == NULL && 
			(feat->attack.wield == WIELD_ONE_HAND || feat->attack.wield == WIELD_OFF_HAND ) ) {
			owner->offHand=ret;
		} else if ( owner->mainHand == NULL && owner->offHand == NULL && feat->attack.wield == WIELD_TWO_HANDS ) {
			owner->mainHand = owner->offHand = ret;
		}
	}

	if ( isUsedWhenPicked() ) {
		ret->use();
	}
	return ret;
}

Item * Item::drop() {
	if (! owner ) return this;
	Creature *ownerBackup=owner; // keep the owner for once the item is removed from inventory
	Item *newItem = owner->removeFromInventory(this);
	newItem->x=ownerBackup->x;
	newItem->y=ownerBackup->y;
	if ( asCreature ) {
		// convert item back to creature
		GameEngine::instance->dungeon->addCreature(asCreature);
	} else {
		GameEngine::instance->dungeon->addItem(newItem);
	}
	GameEngine::instance->log.info("You drop %s %s.", newItem->theName(),
		GameEngine::instance->dungeon->hasRipples(newItem->x,newItem->y) ? "in the water" : "on the ground"
	);
	return newItem;
}

void Item::addModifier(ItemModifierId id, float value) {
    for ( ItemModifier **mod = modifiers.begin(); mod != modifiers.end(); mod ++) {
        if ( (*mod)->id == id ) {
            (*mod)->value += value;
            return;
        }
    }
    modifiers.push(new ItemModifier(id,value));
}

void Item::renderDescriptionFrame(int x, int y, bool below) {
	int cx=0,cy=0,cw=CON_W/2,ch=CON_H/2;
	bool stop=false;

	// find the right border
	for ( cw = CON_W/2; cw > cx && ! stop; cw --) {
		for (int ty=0; ty < ch; ty++) {
			if (descCon->getChar(cx+cw-1,ty) != ' ') {
				stop=true;
				break;
			}
		}
	}
	// find the left border
	stop=false;
	for (cx=0; cx < CON_W/2 && ! stop; cx++,cw--) {
		for (int ty=0; ty < ch; ty++) {
			if (descCon->getChar(cx,ty) != ' ') {
				stop=true;
				break;
			}
		}
	}
	// find the bottom border
	stop=false;
	for (ch = CON_H/2; ch > 0 && ! stop; ch --) {
		for (int tx=cx; tx < cx+cw; tx++) {
			if (descCon->getChar(tx,cy+ch-1) != ' ') {
				stop=true;
				break;
			}
		}
	}
	cx-=2;cw+=4;ch+=2;
	// drawn the frame
	descCon->setDefaultForeground(guiText);
	descCon->putChar(cx,cy,TCOD_CHAR_NW, TCOD_BKGND_NONE);
	descCon->putChar(cx+cw-1,cy,TCOD_CHAR_NE, TCOD_BKGND_NONE);
	descCon->putChar(cx,cy+ch-1,TCOD_CHAR_SW, TCOD_BKGND_NONE);
	descCon->putChar(cx+cw-1,cy+ch-1,TCOD_CHAR_SE, TCOD_BKGND_NONE);
	for (int tx=cx+1; tx < cx+cw -1; tx ++) {
		if ( descCon->getChar(tx,cy) == ' ' ) descCon->setChar(tx,cy,TCOD_CHAR_HLINE);
	}
	descCon->hline(cx+1,cy+ch-1,cw-2,TCOD_BKGND_NONE);
	descCon->vline(cx,cy+1,ch-2,TCOD_BKGND_NONE);
	descCon->vline(cx+cw-1,cy+1,ch-2,TCOD_BKGND_NONE);
	if ( ! below ) y = y -ch+1;
	if ( x-cw/2 < 0 ) x = cw/2;
	else if ( x+cw/2 > CON_W ) x = CON_W-cw/2;
	if ( y + ch > CON_H ) y = CON_H-ch;
	TCODConsole::blit(descCon,cx,cy,cw,ch,TCODConsole::root,x-cw/2,y,1.0f,0.7f);
}

const char *Item::AName() const {
	static char buf[64];
	if ( count == 1 && (!isSoftStackable() || stack.size() == 0) ) {
		if ( name ) {
			sprintf(buf,"%s %s", an ? "An" : "A", name);
		} else {
			sprintf(buf,"%s %s", typeData->an ? "An" : "A", typeName);
		}
	} else {
		int cnt=count > 1 ? count : stack.size()+1;
		const char *nameToUse = name ? name : typeName;
		bool es = (nameToUse[strlen(nameToUse)-1] == 's');
		sprintf(buf,"%d %s%s",cnt,nameToUse, es ? "es":"s");
	}
	return buf;                                   
}

const char *Item::aName() const {
	static char buf[64];
	if ( count == 1 && (!isSoftStackable() || stack.size() == 0) ) {
		if ( name ) {
			sprintf(buf,"%s %s", an ? "an" : "a", name);
		} else {
			sprintf(buf,"%s %s", typeData->an ? "an" : "a", typeName);
		}
	} else {
		int cnt=count > 1 ? count : stack.size()+1;
		const char *nameToUse = name ? name : typeName;
		bool es = (nameToUse[strlen(nameToUse)-1] == 's');
		sprintf(buf,"%d %s%s",cnt,nameToUse, es ? "es":"s");
	}
	return buf;                                   
}

const char *Item::TheName() const {
	static char buf[64];
	if ( count == 1 && (!isSoftStackable() || stack.size() == 0) ) {
		sprintf(buf,"The %s", name ? name : typeName);
	} else {
		int cnt=count > 1 ? count : stack.size()+1;
		const char *nameToUse = name ? name : typeName;
		bool es = (nameToUse[strlen(nameToUse)-1] == 's');
		sprintf(buf,"The %d %s%s",cnt,nameToUse, es ? "es":"s");
	}
	return buf;                                   
}

const char *Item::theName() const {
	static char buf[64];
	if ( count == 1 && (!isSoftStackable() || stack.size() == 0) ) {
		sprintf(buf,"the %s", name ? name : typeName);
	} else {
		int cnt=count > 1 ? count : stack.size()+1;
		const char *nameToUse = name ? name : typeName;
		bool es = (nameToUse[strlen(nameToUse)-1] == 's');
		sprintf(buf,"the %d %s%s",cnt,nameToUse, es ? "es":"s");
	}
	return buf;                                   
}

#define ITEM_CHUNK_VERSION 5

bool Item::loadData(uint32 chunkId, uint32 chunkVersion, TCODZip *zip) {
	if ( chunkVersion != ITEM_CHUNK_VERSION ) return false;
	x=zip->getFloat();
	y=zip->getFloat();
	itemClass=(ItemClass)zip->getInt();
	if ( itemClass < 0 || itemClass >= NB_ITEM_CLASSES ) return false;
	col=zip->getColor();
	typeName=strdup(zip->getString());
	const char *fname=zip->getString();
	if ( fname ) name=strdup(fname);
	count=zip->getInt();
	ch = zip->getInt();
	an = zip->getChar()==1;
	life=zip->getFloat();
	ItemFeature *feat=getFeature(ITEM_FEAT_ATTACK);
	if (feat ) {
		castDelay=zip->getFloat();
		reloadDelay=zip->getFloat();
		damages=zip->getFloat();
	}

	int nbModifiers=zip->getInt();
	while ( nbModifiers > 0) {
		ItemModifierId id = (ItemModifierId)zip->getInt();
		if ( id < 0 || id >= ITEM_MOD_NUMBER ) return false;
		float value=zip->getFloat();
		ItemModifier *mod=new ItemModifier(id,value);
		modifiers.push(mod);
	}
	return true;
}

void Item::saveData(uint32 chunkId, TCODZip *zip) {
	saveGame.saveChunk(ITEM_CHUNK_ID,ITEM_CHUNK_VERSION);
	zip->putFloat(x);
	zip->putFloat(y);
	zip->putInt(itemClass);
	zip->putColor(&col);
	zip->putString(typeName);
	zip->putString(name);
	zip->putInt(count);
	zip->putInt(ch);
	zip->putChar(an ? 1:0);
	zip->putFloat(life);
	ItemFeature *feat=getFeature(ITEM_FEAT_ATTACK);
	if (feat ) {
		zip->putFloat(castDelay);
		zip->putFloat(reloadDelay);
		zip->putFloat(damages);
	}

	zip->putInt(modifiers.size());
	for ( ItemModifier ** it = modifiers.begin(); it != modifiers.end(); it++) {
		zip->putInt((*it)->id);
		zip->putFloat((*it)->value);
	}
}


