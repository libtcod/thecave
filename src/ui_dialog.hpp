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

enum EDialogFlag {
	DIALOG_MAXIMIZABLE=1, // can be minimized/maximized
	DIALOG_DRAGGABLE=2, // can be dragged at any position
	DIALOG_CLOSABLE=4, // has a close button
	DIALOG_MULTIPOS=8, // can be dragged at specific positions
	DIALOG_MODAL=16, // pause the game when displayed
};

class Dialog : public UmbraWidget {
public :
	Dialog() : flags(0),isMinimized(false),waitRelease(false) {}
	void keyboard (TCOD_key_t &key_) { this->key=key_; UmbraWidget::keyboard(key_); }
	void mouse (TCOD_mouse_t &ms_) { this->ms=ms_; UmbraWidget::mouse(ms_); }
	bool update (void);
	virtual bool update(float elapsed, TCOD_key_t &k, TCOD_mouse_t &mouse) = 0;
	void setMaximized();
	void setMinimized();
	bool isMaximizable() { return (flags & DIALOG_MAXIMIZABLE) != 0 ; }
	bool isDraggable() { return (flags & DIALOG_DRAGGABLE) != 0 ; }
	bool isClosable() { return (flags & DIALOG_CLOSABLE) != 0 ; }
	bool isMultiPos() { return (flags & DIALOG_MULTIPOS) != 0 ; }
	bool isModal() { return (flags & DIALOG_MODAL) != 0 ; }
	void onActivate() override;
	virtual void setPos(int x, int y) { rect.setPos(x,y); }
	void onDeactivate() override;
protected :
	int flags;
	TCOD_key_t key;
	TCOD_mouse_t ms;
	TCODConsole *con;
	bool isMinimized;
	bool waitRelease;
	UmbraRect maximizedRect; // pos on screen when maximized
	UmbraRect minimizedRect; // pos when minimized

	virtual void internalUpdate();
	virtual void renderFrame(float alpha, const char *title);
	void endDragging(int /* mousex */, int /* mousey */) {}
};

class MultiPosDialog : public Dialog {
public :
	TCODList<MultiPosDialog *> set; // set of several dependant dialogs
protected :
	TCODList<UmbraRect *> possiblePos; // list of possible pos when minimized
	int targetx,targety;
	void renderTargetFrame(); // render next pos when dragging
	void internalUpdate();
	void renderFrame(float alpha, const char *title);
	virtual void endDragging(int mousex,int mousey);
};
