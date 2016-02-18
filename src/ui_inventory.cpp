/*
* Copyright (c) 2010 Jice
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
#include "main.hpp"

#define INV_WIDTH 70
#define INV_HEIGHT 40

// skin
TCODColor guiBackground(32,16,0);
TCODColor guiHighlightedBackground(128,128,0);
TCODColor guiDisabledText(95,95,95);
TCODColor guiText(255,180,0);
TCODColor guiHighlightedText(114,114,255);


static const char *tabNames[NB_INV_TABS] = {
	"All","Arm","Wea","Foo","Mis"
};

Inventory::Inventory() : curTabId(INV_ARMOR), mouseTabId(NB_INV_TABS), selectedItem(-1),closeOn(false) {
	con=new TCODConsole(INV_WIDTH,INV_HEIGHT);
	con->setKeyColor(TCODColor::black);
	rect.x=rect.y=5;
	rect.w=INV_WIDTH/2;
	rect.h=INV_HEIGHT;
	firstOpen=true;
	isDragging=false;
	dragOut=false;
	cmenuon=false;
	cmenudrag=false;
	flags=DIALOG_MODAL|DIALOG_CLOSABLE;
	closeButton.set(rect.w-1,0);
}

void Inventory::initialize(Creature *owner) {
	for (int i=0; i< NB_INV_TABS; i++) {
		tabs[i].items.clear();
		tabs[i].offset=0;
	}
	for (Item **it=owner->inventoryBegin(); it !=owner->inventoryEnd(); it++) {
		tabs[ (*it)->typeData->inventoryTab ].items.push(*it);
		tabs[INV_ALL].items.push(*it);
	}
	for (; curTabId < NB_INV_TABS && tabs[curTabId].items.size() == 0; curTabId = (InventoryTabId)(curTabId+1)) {}
	if ( curTabId == NB_INV_TABS ) curTabId=(InventoryTabId)0;
	if ( selectedItem >= tabs[curTabId].items.size() ) selectedItem  = tabs[curTabId].items.size()-1;
	this->owner=owner;
	container=NULL;
	itemToCombine=itemToCombine2=combinationResult=NULL;
	cmenuon=false;
	cmenudrag=false;
	rect.x=rect.y=5;
	isDragging=dragOut=false;
}

void Inventory::initialize(Item *container) {
	for (int i=0; i< NB_INV_TABS; i++) {
		tabs[i].items.clear();
		tabs[i].offset=0;
	}
	for (Item **it=container->stack.begin(); it !=container->stack.end(); it++) {
		tabs[ (*it)->typeData->inventoryTab ].items.push(*it);
		tabs[INV_ALL].items.push(*it);
	}
	for (; curTabId < NB_INV_TABS && tabs[curTabId].items.size() == 0; curTabId = (InventoryTabId)(curTabId+1)) {}
	if ( curTabId == NB_INV_TABS ) curTabId=(InventoryTabId)0;
	if ( selectedItem >= tabs[curTabId].items.size() ) selectedItem  = tabs[curTabId].items.size()-1;
	this->container=container;
	owner=NULL;
	itemToCombine=itemToCombine2=combinationResult=NULL;
	cmenuon=false;
	cmenudrag=false;
	rect.x=6+INV_WIDTH/2;
	rect.y=5;
	isDragging=dragOut=false;
}

void Inventory::checkDefaultAction(Item *item) {
	// when looting, drop item is the default action instead of use
	// check that the default action is coherent with current mode (inventory or loot)
	if ( container ) return; // concerns only inventory
	ItemActionId *dropAction=NULL;
	ItemActionId *useAction=NULL;
	bool dropFirst=false;
	for( ItemActionId *id=item->typeData->actions.begin(); id != item->typeData->actions.end(); id++) {
		ItemAction *action=ItemAction::getFromId(*id);
		if (( owner && action->onInventory() )
			|| (container && action->onLoot() ) ) {
			if ( *id == ITEM_ACTION_USE ) useAction=id;
			if ( *id == ITEM_ACTION_DROP ) {
				dropAction=id;
				if ( useAction != NULL ) dropFirst=false;
				else dropFirst=true;
			}
		}
	}
	if ( (GameEngine::instance->guiLoot.isActive() && !dropFirst ) 
		|| (!GameEngine::instance->guiLoot.isActive() && dropFirst )){
		// bad default action. swap use and drop
		if ( useAction && dropAction ) {
			ItemActionId tmp=*useAction;
			*useAction=*dropAction;
			*dropAction=tmp;				
		}
	}
}

void Inventory::render() {
	int itemx=isDragging ? dragx : rect.x+INV_WIDTH/4;
	int itemy=isDragging ? dragy+1 : rect.y+selectedItem+4;
	InventoryTab *curTab=&tabs[curTabId];
	if ( ! dragOut ) {
		// render the inventory frame
		con->setDefaultBackground(guiBackground);
		con->setDefaultForeground(guiText);
		int w2=INV_WIDTH/2;
		con->printFrame(0,0,w2,rect.h,true,TCOD_BKGND_SET,owner ? "Inventory" 
			: container->name ? container->name : container->typeName);
		con->setChar(w2-1,0,'x');
		con->setCharForeground(w2-1,0,closeOn ? guiHighlightedText : guiText);

		// render the tabs
		int tx=2;
		for (int i=0; i< NB_INV_TABS; i++) {
			if ( i > 0 ) {
				con->setDefaultForeground(guiText);
				con->print(tx++,1,"\xB3");
			}
			tabs[i].x=tx;
			int numItems = tabs[i].items.size();
			con->setDefaultBackground(mouseTabId==i && curTabId != i ? guiHighlightedBackground : guiBackground );
			con->setDefaultForeground(TCODColor::white);
			con->printEx(tx++,1,TCOD_BKGND_SET,TCOD_LEFT,"%c",tabNames[i][0]);
			con->setDefaultForeground(curTabId==i ? guiHighlightedText : guiText);
			if ( numItems > 0 ) {
				char buf[128];
				sprintf(buf,"%s:%d",&tabNames[i][1],numItems);
				con->printEx(tx,1,TCOD_BKGND_SET,TCOD_LEFT,buf);
				tabs[i].len=strlen(buf)+1;
			} else {
				con->printEx(tx,1,TCOD_BKGND_SET,TCOD_LEFT,&tabNames[i][1]);
				tabs[i].len= strlen(tabNames[i]);
			}
			tx += tabs[i].len-1;
		}

		// render the items list
		int ty=3;
		int skip=curTab->offset;
		int num=0;
		con->setDefaultForeground(guiText);
		for (Item **it=curTab->items.begin(); it != curTab->items.end() && ty < rect.y+rect.h-2; it++,skip--) {
			if ( skip <= 0 ) {
				con->setDefaultBackground(num == selectedItem ? guiHighlightedBackground : guiBackground );
				con->rect(2,ty,w2-4,1,false,TCOD_BKGND_SET);
				con->setDefaultForeground(Item::classColor[(*it)->itemClass]);
				if ( (*it)->isEquiped() ) con->print(2,ty++,"%s (equiped)",(*it)->aName());
				else con->print(2,ty++,(*it)->aName());
				num++;
			}
		}

		blitSemiTransparent(con,0,0,INV_WIDTH/2,INV_HEIGHT,TCODConsole::root,rect.x,rect.y,0.8f,1.0f);
	}
	if ( cmenuon ) {
		// render the context menu
		if (selectedItem >= 0 ) {
			Item *item=curTab->items.get(selectedItem);
			checkDefaultAction(item);
			cmenuheight = 0;
			cmenuwidth=0;
			// compute context menu size
			for( ItemActionId *id=item->typeData->actions.begin(); id != item->typeData->actions.end(); id++) {
				ItemAction *action=ItemAction::getFromId(*id);
				if (( owner && action->onInventory() )
					|| (container && action->onLoot() ) ) {
					cmenuheight++;
					cmenuwidth = MAX(cmenuwidth,(int)strlen(action->name)+2);
				}
			}
			int num=0;
			if ( cmenuwidth > 0 ) {
				TCODConsole::root->rect(cmenux,cmenuy,cmenuwidth,cmenuheight,true);
			}
			// print the actions names
			bool first=true;
			for( ItemActionId *id=item->typeData->actions.begin(); id != item->typeData->actions.end(); id++) {
				ItemAction *action=ItemAction::getFromId(*id);
				if (( owner && action->onInventory() )
					|| (container && action->onLoot() ) ) {
					TCODConsole::root->setDefaultForeground(num == cmenuitem ? TCODColor::white : first ? guiHighlightedText : guiText);
					TCODConsole::root->setDefaultBackground(num == cmenuitem ? guiHighlightedBackground : guiBackground );
					TCODConsole::root->printEx(cmenux,cmenuy+num,TCOD_BKGND_SET, TCOD_LEFT," %s%*s",action->name,cmenuwidth-1-strlen(action->name),"");
					first=false;
					num++;
				}
			}
		}
	} else {
		// render the item description
		if ( dragOut && isDragging && dragItem ) {
			TCODConsole::root->setChar(itemx,itemy-2, dragItem->ch);
			TCODConsole::root->setCharForeground(itemx,itemy-2, dragItem->col);
		} else if ((! isDragging && itemToCombine == NULL ) || (isDragging && ! dragOut && selectedItem == -1 ) ) {
			Item *item=isDragging ? dragItem : selectedItem >= 0 ? curTab->items.get(selectedItem) : NULL;
			if ( item ) item->renderDescription(itemx,itemy);
		} else if ( selectedItem >= 0 ) {
			if ( itemToCombine2 == curTab->items.get(selectedItem) ) {
				combinationResult->renderGenericDescription(itemx,itemy);
			} else if ( itemToCombine || dragItem ) {
				if ( combinationResult ) delete combinationResult;
				combinationResult=itemToCombine2=NULL;
				combination = Item::getCombination(itemToCombine ? itemToCombine : dragItem, curTab->items.get(selectedItem));
				if ( combination ) {
					itemToCombine2=curTab->items.get(selectedItem);
					combinationResult=Item::getItem(combination->resultType, 0,0, false);
					combinationResult->addComponent(itemToCombine ? itemToCombine : dragItem);
					combinationResult->addComponent(curTab->items.get(selectedItem));
				}
			}
		}
	}
}

void Inventory::activateItem() {
	Item *item=tabs[curTabId].items.get(selectedItem);
	checkDefaultAction(item);
	if ( combinationResult ) {
		GameEngine::instance->log.info("You created %s",combinationResult->aName());
		GameEngine::instance->player.addToInventory(combinationResult);
		// switch to the tab of the generated item
		if ( curTabId != INV_ALL ) curTabId=combinationResult->typeData->inventoryTab;
		combinationResult=NULL;
		bool destroy1=false;
		// destroy items if needed
		TCODList<Item *>toDestroy;
		if ( combination->ingredients[0].destroy ) {
			if ( itemToCombine->isA(combination->ingredients[0].tag) || itemToCombine->isA(combination->ingredients[0].type) ) {
				toDestroy.push(itemToCombine);
				destroy1=true;
			} else if ( itemToCombine2->isA(combination->ingredients[0].tag) || itemToCombine2->isA(combination->ingredients[0].type) ) {
				toDestroy.push(itemToCombine2);
			}
		}
		if ( combination->nbIngredients == 2 && combination->ingredients[1].destroy ) {
			if ( itemToCombine->isA(combination->ingredients[1].tag) || itemToCombine->isA(combination->ingredients[1].type) ) {
				toDestroy.push(itemToCombine);
				destroy1=true;
			} else if ( itemToCombine2->isA(combination->ingredients[1].tag) || itemToCombine2->isA(combination->ingredients[1].type) ) {
				toDestroy.push(itemToCombine2);
			}
		}
		for ( Item **it=toDestroy.begin(); it != toDestroy.end(); it++) {
			if ((*it)->count > 1 ) (*it)->count--;
			else {
				GameEngine::instance->player.removeFromInventory((*it),true);
			}
		}
		// recompute inventory
		if ( owner ) initialize(owner);
		else initialize(container);
		itemToCombine2=NULL;
		if ( destroy1 ) {
			// itemToCombine was destroyed.
			itemToCombine=NULL;
		}
	} else {
		// check the first valid action
		ItemActionId *id=NULL;
		for( id=item->typeData->actions.begin(); id != item->typeData->actions.end(); id++) {
			ItemAction *action=ItemAction::getFromId(*id);
			if (( owner && action->onInventory() )
				|| (container && action->onLoot() ) ) break;
		}
		runActionOnItem(*id,item);
	}
}

void Inventory::activate() {
	Dialog::activate();
	if ( firstOpen && ! GameEngine::instance->guiLoot.isActive()) {
		firstOpen=false;
		tutorial->startLiveTuto(TUTO_INVENTORY);
	}
}

void Inventory::deactivate() {
	Dialog::deactivate();
	if ( combinationResult ) delete combinationResult;
	combinationResult=NULL;
}

void Inventory::runActionOnItem(ItemActionId id, Item *item) {
	switch(id) {
		case ITEM_ACTION_USE : 
			item->use();
			if ( owner ) initialize(owner);
			else initialize(container);
		break;
		case ITEM_ACTION_DROP : 
			if ( GameEngine::instance->guiLoot.isActive() ) {
				Item *newItem=owner->removeFromInventory(item);
				GameEngine::instance->guiLoot.container->putInside(newItem);
				GameEngine::instance->guiLoot.initialize(GameEngine::instance->guiLoot.container);
			} else {
				item->drop();
			} 
			if ( owner ) initialize(owner);
			else initialize(container);
		break;
		case ITEM_ACTION_TAKE : 
			item->pickup(&GameEngine::instance->player); 
			initialize(container);
			if ( GameEngine::instance->guiInventory.isActive() ) {
				GameEngine::instance->guiInventory.initialize(&GameEngine::instance->player);
			}
		break;
		case ITEM_ACTION_THROW :
			isDraggingStart=isDragging=true;
			dragItem=item;
			dragOut=true;
			cmenuon=false;
			cmenudrag=true;
		break;
		case ITEM_ACTION_DISASSEMBLE :
		{
			// create components
			for (Item **it=item->components.begin(); it != item->components.end(); it++) {
				if (owner) owner->addToInventory(*it);
				else container->putInside(*it);
			}
			// destroy item
			Item *newItem=NULL;
			if ( owner ) newItem=owner->removeFromInventory(item);
			else {
				Cell *cell=GameEngine::instance->dungeon->getCell((int)item->x,(int)item->y);
				newItem = item->removeFromList(&cell->items,false);
			}
			delete newItem;
			if ( owner ) initialize(owner);
			else initialize(container);
		}
		break;
		default:break;
	}
}

bool Inventory::update(float elapsed, TCOD_key_t &k, TCOD_mouse_t &mouse) {
	int mx=mouse.cx-rect.x;
	int my=mouse.cy-rect.y;
	closeOn = ( mx == rect.w/2 - 1 && my == 0 );
	mouseTabId = NB_INV_TABS;
	if ( my == 1 ) {
		for (int i=0; i< NB_INV_TABS; i++) {
			if ( mx >= tabs[i].x && mx < tabs[i].x + tabs[i].len) {
				mouseTabId=(InventoryTabId)i;
				if ( mouse.lbutton_pressed || isDragging ) curTabId=mouseTabId;
				break;
			}
		}
	}
	if (!isDragging && !cmenuon && mouse.rbutton) {
		cmenuon=true;
		cmenux=mouse.cx;
		cmenuy=mouse.cy+1;
		cmenuitem=-1;
		cmenuwidth=cmenuheight=0; // computed in render
	} else if ( cmenuon && ! mouse.rbutton ) {
		cmenuon=false;
		if ( cmenuitem >= 0 && selectedItem >= 0) {
			Item *item=tabs[curTabId].items.get(selectedItem);
			ItemActionId *id=NULL;
			int count=cmenuitem;
			for( id=item->typeData->actions.begin(); id != item->typeData->actions.end(); id++) {
				ItemAction *action=ItemAction::getFromId(*id);
				if (( owner && action->onInventory() )
					|| (container && action->onLoot() ) ) {
					if ( count == 0 ) break;
					count--;
				}
			}
			
			runActionOnItem(*id,item);
			if ( *id == ITEM_ACTION_THROW ) {
				dragx=dragStartX=mouse.cx;
				dragy=dragStartY=mouse.cy;
			}
		}
	} else if (cmenuon) {
		if ( mouse.cx >= cmenux && mouse.cx <= cmenux+cmenuwidth
			&& mouse.cy >= cmenuy && mouse.cy <= cmenuy+cmenuheight) {
			cmenuitem=mouse.cy-cmenuy;
		} else {
			cmenuitem=-1;
		}
	} else {
		selectedItem = -1;
		if ( my >= 3 && my < 3+tabs[curTabId].items.size() && mx > 1 && mx < rect.x+rect.w/2 ) {
			selectedItem = my-3;
			if ( (! isDragging || combinationResult ) && mouse.lbutton_pressed ) {
				activateItem();
			}
		}
	}
	if (!cmenuon && ! isDraggingStart && mouse.lbutton && selectedItem >= 0 ) {
		// start dragging. to be confirmed
		isDraggingStart = true;
		dragItem = tabs[curTabId].items.get(selectedItem);
		dragOut=false;
		dragStartX=mouse.cx;
		dragStartY=mouse.cy;
		dragx=mouse.cx;
		dragy=mouse.cy;
	} else if ( isDraggingStart && ! isDragging && mouse.lbutton) {
		dragx=mouse.cx;
		dragy=mouse.cy;
		if ( dragx != dragStartX || dragy != dragStartY ) {
			// dragging confirmed
			isDragging=true;
			itemToCombine=dragItem;
		}
	} else if ( (isDragging || isDraggingStart)
		&& ((! cmenudrag && ! mouse.lbutton) || (cmenudrag && mouse.lbutton_pressed)) ) {
		// drop
		if ( isDragging ) {
			itemToCombine=NULL;
		}
		isDragging=isDraggingStart=false;
		if ( dragOut ) {
			int dungeonx = mouse.cx + GameEngine::instance->xOffset;
			int dungeony = mouse.cy -1 + GameEngine::instance->yOffset;
			Dungeon *dungeon=GameEngine::instance->dungeon;
			dragOut=false;
			if ( ! IN_RECTANGLE(dungeonx,dungeony,dungeon->size,dungeon->size) || mouse.rbutton ) {
				// out of map or right click : cancel
			} else {
				dragItem->x=owner ? owner->x : container->x;
				dragItem->y=owner ? owner->y : container->y;
				if ( dungeonx == GameEngine::instance->player.x && dungeony == GameEngine::instance->player.y ) {
					if ( owner ) {
						// drag from inventory and drop on player cell = drop
						dragItem->drop();
						initialize(owner);
					} else {
						// drag from loot screen and drop on player cell = take
						dragItem->pickup(&GameEngine::instance->player);
						initialize(container);
					}
				} else {
					// drag and drop on another cell = throw
					float dx = dungeonx - (owner ? owner->x : container->x);
					float dy = dungeony - (owner ? owner->y : container->y);
					float invLength = Entity::fastInvSqrt(dx*dx+dy*dy);
					dx *= invLength;
					dy *= invLength;
					Item *newItem=NULL;
					if ( owner ) newItem = owner->removeFromInventory(dragItem);
					else newItem = dragItem->removeFromList(&container->stack);
					newItem->dx=dx;
					newItem->dy=dy;
					newItem->speed=1.0f/(invLength*1.5f);
					newItem->speed=MIN(12.0f,newItem->speed);
					newItem->duration = 1.5f;
					dungeon->addItem(newItem);
					if ( owner ) initialize(owner);
					else initialize(container);
				}
				return false;
			}
		}
		dragItem=NULL;
	} else if ( isDragging ) {
		if(  mouse.rbutton ) {
			// cancel dragging
			isDragging=isDraggingStart = false;
			dragOut=cmenudrag=false;
			dragItem=NULL;
		} else if (mouse.lbutton || cmenudrag) {
			if (! dragOut && isDragging && 
				(dragx < rect.x ||dragx >= rect.x+INV_WIDTH/2 || dragy < rect.y 
				|| dragy >= rect.y+INV_HEIGHT ) ) {
				// dragging out of inventory frame
				dragOut=true;
			}
			dragx=mouse.cx;
			dragy=mouse.cy;
		}
	}
	if ( ! k.pressed) {
		if (k.c == 'A' || k.c=='a') curTabId=INV_ARMOR;
		else if (k.c == 'W' || k.c=='w') curTabId=INV_WEAPON;
		else if (k.c == 'F' || k.c=='f') curTabId=INV_FOOD;
		else if (k.c == 'M' || k.c=='m') curTabId=INV_MISC;
		if ( strchr("AWFMudcawfm",k.c)) {
			k.vk=TCODK_NONE;
			k.c=0;
		}
	}
	if ( itemToCombine &&
		(( k.vk == TCODK_ESCAPE && ! k.pressed ) || mouse.rbutton_pressed )) {
		// cancel item combination
		itemToCombine=NULL;
		if ( combinationResult ) delete combinationResult;
		combinationResult=NULL;
		k.vk=TCODK_NONE;
	}
	if ( (k.vk == TCODK_ESCAPE && ! k.pressed) ) {
		return false;
	}
	return true;
}


