//
//  Layer.h
//
//
//

#pragma once

#include "Graphics.h"
#include <string>
#include "Texture.h"
#include "RoundedRect.h"
#include <map>
#include "Font.h"
#include <functional>



class Layer : public Rectf {
public:
	
	std::string name;
	glm::vec4 color { 1.f, 1.f, 1.f, 1.f };
	
	bool interactive = false;
	bool visible = true;
	
	
	Layer(Graphics &g, std::string name = "");
	Layer(Graphics &g, std::string name, glm::vec4 c);
	virtual ~Layer();
	Layer(Graphics &g, std::string name, float x, float y, float w, float h);
	
	virtual void draw();
	
	Layer *addChild(Layer *layer);
	bool removeChild(Layer *layer);
	void addChildren(std::vector<Layer*> layers);
	bool removeFromParent();
	
	
	
	virtual void clear();
	
	std::string toString() const;
	
	bool clipToBounds = false;

	
	
	// reorder layers - if the parameter is null, it's referring to this
	// layer.
	void sendToFront(Layer *child = NULL);
	void sendToBack(Layer *child = NULL);
	
	// override for when ui needs to be rebuilt
	virtual void doLayout() {}
	
	// override for touch events
	virtual void touchOver(float x, float y) {}
	virtual bool touchDown(float x, float y, int id) { return false; }
	virtual void touchMoved(float x, float y, int id) {}
	virtual void touchUp(float x, float y, int id) {}
	
	virtual void mouseScrolled(float x, float y, float scrollX, float scrollY ) {}
	virtual void mouseZoomed(float x, float y, float zoom) {}

	// override to have something to do before the draw
	virtual void update() {}

	int getNumChildren() const;
	Layer *getChild(int index);
	Layer *getFirstChild();
	Layer *getLastChild();
	
	
	// called by api, do not use
	virtual void _draw();
	void layoutSelfAndChildren();
	void _touchOver(float x, float y);
	void _touchUp(float x, float y, int id);
	void _touchMoved(float x, float y, int id);
	bool _touchDown(float x, float y, int id);
	void _mouseScrolled(float x, float y, float scrollX, float scrollY );
	void _mouseZoomed(float x, float y, float zoom);
	
	void _update();
	

	Layer *getParent();
	Layer *getRoot();
	
	glm::vec2 getAbsolutePosition();
	Rectf getAbsoluteRect();
	
	glm::vec2 getAbsolutePosition(glm::vec2 p);
	Rectf getAbsoluteRect(const Rectf &r);
	
	glm::vec2 getLocalPosition(glm::vec2 pp);

	// rather than absolute coords, this tries to give you the rect
	// relative to an ancestor. If it can't find the ancestor, it will return false
	bool getRectRelativeTo(const Layer *l, Rectf &r);
	
	// same functionality as getLocalPosition()
	void absoluteToLocalCoords(float &xx, float &yy);
	
	// same functionality as getAbsolutePosition()
	void localToAbsoluteCoords(float &xx, float &yy);
	
	void setBottomCenter(float x, float y) { set(x - this->width/2, y - this->height, this->width, this->height); }
	
	// attempt to pass focus from this layer to another
	
	void transferFocus(Layer *otherLayer, int touchId);
	bool hasFocus() const;
	void removeFocus();
	
	void positionAbove(Layer *l, float padding = 0);
	void positionUnder(Layer *l, float padding = 0);
	void positionLeftOf(Layer *l, float padding = 0);
	void positionRightOf(Layer *l, float padding = 0);
	void layoutChildrenAsGrid(int cols, int rows, float padding = 0);
	void alignChildrenToPixels();
	
	// used for hacky things, don't use
	Graphics &getGraphics() { return g; }
	
protected:

	void transformMouse(float &x, float &y);
	
	// vbo for drawing rounded rects
	RoundedRect roundedRect;
	
	void maskOn();
	void maskOff();
	
	Graphics &g;
	
private:
	Layer *parent = nullptr;
	std::vector<Layer*> children;

	void pushMask();
	void popMask();
	// this gets abused because its not
	// really scoped usage, but it helps
	// to not reuse some code.
	ScopedMask scopedMask;
	
	
	//static void transformFocusedMouse(float &x, float &y);
	//static glm::vec2 focusedMouseTransform;
};


