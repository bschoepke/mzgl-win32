//
//  Layer.cpp
//
//
//

#include "Layer.h"
#include "Graphics.h"
#include "util.h"
#include <iostream>

using namespace std;

Layer::Layer(Graphics &g, string name) : g(g), name(name) {}
Layer::Layer(Graphics &g, string name, glm::vec4 c) : g(g), name(name), color(c) {}
Layer::Layer(Graphics &g, string name, float x, float y, float w, float h) : Rectf(x, y, w, h), g(g), name(name) {}

Layer::~Layer() {
	clear();
}

void Layer::draw() {

	if(width==0 || height==0) return;
	if(color.a>0) {
		g.setColor(color);
		g.drawRect(*this);
	}
}

string Layer::toString() const {
	char c[512];
	sprintf(c, "name: %s  (xy: %.0f,%.0f   %.0f x %.0f)", name.c_str(), x, y, width, height);
	return string(c);
}


void Layer::maskOn() {
	glEnable(GL_SCISSOR_TEST);
	auto r = getAbsoluteRect();
	r.y = g.height - (r.y+r.height);
	
	// my suspicion is the way mac and ios set up pixel scaling
	// means that this works for them but not others. I should make others work
	// with this.
#if defined(__APPLE__) || defined(__ANDROID__)
	glScissor(r.x, r.y, r.width, r.height);
#else
    glScissor(r.x/g.pixelScale, r.y/g.pixelScale, r.width/g.pixelScale, r.height/g.pixelScale);
#endif
//	glScissor(r.x*fctr, g.height*fctr-(r.y*fctr+r.height*fctr), r.width*fctr, r.height*fctr);
	//g.clear({0, 1, 0, 1});
}

void Layer::maskOff() {
	glDisable(GL_SCISSOR_TEST);
	
}

void Layer::pushMask() {
	scopedMask.startMask(g, getAbsoluteRect());
}


void Layer::popMask() {
	scopedMask.stopMask();
}


void Layer::_draw() {
	if(!visible) return;
	
	if(clipToBounds) {
		pushMask();
	}
	
	if(x!=0 || y!=0) {
		g.pushMatrix();
		draw();
		g.translate(x, y);
		for(auto *c : children) c->_draw();
		g.popMatrix();
	} else {
		draw();
		for(auto *c : children) c->_draw();
	}
	if(clipToBounds) {
		popMask();
	}
}

void Layer::layoutSelfAndChildren() {
	doLayout();
	for(auto *c : children) {
		c->layoutSelfAndChildren();
	}
}


























bool Layer::getRectRelativeTo(const Layer *l, Rectf &r) {
	if(parent==nullptr) return false;
	if(parent==l) {
		r = *this;
		return true;
	}
	
//	printf("=================================\n");
//	printf("Not direct parent\n");
	Layer *curr = parent;
	Rectf out = *this;
	
//	int i = 1;
	while(1) {
//		printf("run %d\n", i++);
		if(curr==nullptr) {
			return false;
		}
		
		out.x += curr->x;
		out.y += curr->y;
//		printf("new xy: %.0f %.0f\n", out.x, out.y);
		curr = curr->parent;
		
		if(curr==l) {
			r = out;
			return true;
		}
	}
	return false;
}









Layer *Layer::addChild(Layer *layer) {
	layer->parent = this;
	children.push_back(layer);
	return layer;
}

bool Layer::removeFromParent() {
	if(getParent()!=nullptr) {
		bool res = getParent()->removeChild(this);
		parent = nullptr;
		return res;
	}
	return false;
}

bool Layer::removeChild(Layer *layer) {
	for(int i = 0; i < children.size(); i++) {
		if(children[i]==layer) {
			children[i]->parent = nullptr;
			children.erase(children.begin() + i);
			return true;
		}
	}
	return false;
}

void Layer::addChildren(vector<Layer*> layers) {
	for(auto *l : layers) {
		addChild(l);
	}
}


void Layer::_mouseScrolled(float x, float y, float scrollX, float scrollY ) {
	if(!visible) return;

	float xx = x;
	float yy = y;
	transformMouse(xx, yy);
	for(auto it = children.rbegin(); it != children.rend(); it++) {
		(*it)->_mouseScrolled(xx, yy, scrollX, scrollY);
	}

	if(interactive && inside(x, y)) {
		mouseScrolled(x, y, scrollX, scrollY);
	}
}

void Layer::_mouseZoomed(float x, float y, float zoom) {
	if(!visible) return;

	float xx = x;
	float yy = y;
	transformMouse(xx, yy);
	for(auto it = children.rbegin(); it != children.rend(); it++) {
		(*it)->_mouseZoomed(xx, yy, zoom);
	}

	if(interactive && inside(x, y)) {
		mouseZoomed(x, y, zoom);
	}
}



void Layer::_touchOver(float x, float y) {
	if(!visible) return;

	float xx = x;
	float yy = y;
	transformMouse(xx, yy);
	
	for(auto it = children.rbegin(); it != children.rend(); it++) {
		(*it)->_touchOver(xx, yy);
	}
	
	if(interactive) {
		touchOver(x, y);
	}
	

}
				 

void Layer::_touchUp(float x, float y, int id) {
	if(!visible) return;

	float xx = x;
	float yy = y;
	transformMouse(xx, yy);
	
	
	if(parent==NULL && g.focusedLayers.find(id)!=g.focusedLayers.end()) {
		float xxx = x;
		float yyy = y;
		
		g.focusedLayers[id]->absoluteToLocalCoords(xxx, yyy);
		g.focusedLayers[id]->touchUp(xxx, yyy, id);
		
	} else {
		
		for(auto it = children.rbegin(); it != children.rend(); it++) {
			(*it)->_touchUp(xx, yy, id);
		}
		
		// everyone receives a touch up
		//if(interactive && hitTest(x, y)) {
			touchUp(x, y, id);
		//}
	}
	// so if we're the root, we want to clear the focussed layer for this touch
	if(parent==NULL) {
		if(g.focusedLayers.find(id)!=g.focusedLayers.end()) {
			g.focusedLayers.erase(id);
		}
	}
}


void Layer::_touchMoved(float x, float y, int id) {
	if(!visible) return;

	float xx = x;
	float yy = y;
	transformMouse(xx, yy);

	
	if(parent==NULL && g.focusedLayers.find(id)!=g.focusedLayers.end()) {
		float xxx = x;
		float yyy = y;
		
		g.focusedLayers[id]->absoluteToLocalCoords(xxx, yyy);
		g.focusedLayers[id]->touchMoved(xxx, yyy, id);
		return;
	}

	for(auto it = children.rbegin(); it != children.rend(); it++) {
		(*it)->_touchMoved(xx, yy, id);
	}
	
	if(interactive && inside(x, y)) {
		touchMoved(x, y, id);
	}
	

}

void Layer::transformMouse(float &xx, float &yy) {
	xx -= this->x;
	yy -= this->y;
}


//void Layer::transformFocusedMouse(float &x, float &y) {
//	if(focusedLayer!=NULL) {
//		if(focusedLayer->getParent()!=NULL) {
//			glm::vec2 p = focusedLayer->getParent()->getAbsolutePosition();
//			x -= p.x;
//			y -= p.y;
//		}
//	}
//}

bool Layer::_touchDown(float x, float y, int id) {
	
	if(!visible) return false;
	float xx = x;
	float yy = y;
	transformMouse(xx, yy);

	
	// if we're clipping to bounds, reject any touches
	// that aren't inside the bounds
	if(clipToBounds) {
		if(!inside(x, y)) return false;
	}

	for(auto it = children.rbegin(); it != children.rend(); it++) {
		
		if((*it)->_touchDown(xx, yy, id)) {
			return true;
		}
	}

	

	if(interactive && inside(x, y)) {
		g.focusedLayers[id] = this;

		return touchDown(x, y, id);
	}
	return false;
}




void Layer::_update() {
	for(auto *c : children) {
		c->_update();
	}
	update();
}

void Layer::sendToBack(Layer *child) {
	if(child==NULL) {
		Layer *parent = this->getParent();
		if(parent!=NULL) {
			parent->sendToBack(this);
		}
	} else {
		if(children.size()==0) return;
		if(children[0]==child) return;
		for(int i = 0; i < children.size(); i++) {
			if(children[i]==child) {
				children.erase(children.begin()+i);
				children.insert(children.begin(), child);
				return;
			}
		}
		cout << "Couldn't find child in sendLayerToBack " << child->name << endl;
	}
}


void Layer::sendToFront(Layer *child) {
	if(child==NULL) {
		Layer *parent = this->getParent();
		if(parent!=NULL) {
			parent->sendToFront(this);
		}
	} else {
		if(children.size()==0) return;
		if(children.back()==child) return;
		for(int i = 0; i < children.size(); i++) {
			if(children[i]==child) {
				children.erase(children.begin()+i);
				children.push_back(child);
				return;
			}
		}
		std::cout << "Couldn't find child in sendLayerToFront " << child->name << endl;
	}
}


Layer *Layer::getParent() {
	return parent;
}

Layer *Layer::getRoot() {
	if(parent==nullptr) {
		return this;
	} else {
		return parent->getRoot();
	}
}

int Layer::getNumChildren() const {
	return (int)children.size();
}


Layer *Layer::getChild(int index) {
	return children[index];
}

Layer *Layer::getFirstChild() {
	return children[0];
}
Layer *Layer::getLastChild() {
	return children.back();
}


Rectf Layer::getAbsoluteRect() {
	return getAbsoluteRect(*this);
}

glm::vec2 Layer::getAbsolutePosition() {
	return getAbsolutePosition(position());
}

glm::vec2 Layer::getAbsolutePosition(glm::vec2 pos) {
	localToAbsoluteCoords(pos.x, pos.y);
	return pos;
}

glm::vec2 Layer::getLocalPosition(glm::vec2 pos) {
	absoluteToLocalCoords(pos.x, pos.y);
	return pos;
}

Rectf Layer::getAbsoluteRect(const Rectf &rr) {
	Rectf r;
	glm::vec2 tl = getAbsolutePosition(rr.tl());
	r.x = tl.x;
	r.y = tl.y;
	r.width = rr.width;
	r.height = rr.height;
	return r;
}

void Layer::absoluteToLocalCoords(float &xx, float &yy) {
	Layer *layer = this;
	while((layer = layer->getParent()) != nullptr) {
		xx -= layer->x;
		yy -= layer->y;
	}
}
void Layer::localToAbsoluteCoords(float &xx, float &yy) {
	Layer *layer = this;
	while((layer = layer->getParent()) != nullptr) {
		xx += layer->x;
		yy += layer->y;
	}
}

bool Layer::hasFocus() const {
	for(auto l : g.focusedLayers) {
		if(l.second==this) {
			return true;
		}
	}
	return false;
}


void Layer::removeFocus() {
	for(auto l = g.focusedLayers.begin(); l != g.focusedLayers.end(); l++) {
		if(l->second==this) {
			g.focusedLayers.erase(l);
			return;
		}
	}
}


/**
 * This used to take only the otherLayer as the parameter, but that meant
 * that if a layer had 2 fingers focused on it, there would be no way to
 * distinguish between the 2 fingers - so now you need to pass in a touchId
 */
void Layer::transferFocus(Layer *otherLayer, int touchId) {
	if(g.focusedLayers.find(touchId)!=g.focusedLayers.end() && g.focusedLayers[touchId]==this) {
		g.focusedLayers[touchId] = otherLayer;
	} else {
		cout << "Couldn't find the other layer to focus on" << endl;
	}
}

void Layer::clear() {
	for(int i = 0; i < children.size(); i++) {
		for(auto it = g.focusedLayers.begin(); it != g.focusedLayers.end();) {
			if((*it).second==children[i]) {
				g.focusedLayers.erase(it++);
			} else {
				it++;
			}
		}
	}
	
	
	
	for(auto *c : children) {
		delete c;
	}
	
	children.clear();
	
}



void Layer::positionAbove(Layer *l, float padding) {
	x = l->x;
	setBottom(l->y - padding);
}
void Layer::positionUnder(Layer *l, float padding) {
	x = l->x;
	y = l->bottom() + padding;
}
void Layer::positionLeftOf(Layer *l, float padding) {
	setRight(l->x - padding);
	y = l->y;
}
void Layer::positionRightOf(Layer *l, float padding) {
	x = l->right() + padding;
	y = l->y;
}


void Layer::layoutChildrenAsGrid(int cols, int rows, float padding) {
	float w = (width - (cols-1)*padding) / (float)cols;
	float h = (height - (rows-1)*padding) / (float)rows;
	
	float wSpace = w + padding;
	float hSpace = h + padding;
	int pos = 0;
	for(int i  = 0; i < getNumChildren(); i++) {
		Layer *l = getChild(i);
		if(l->visible) {
			l->width = w;
			l->height = h;
			l->x = (pos % cols) * wSpace;
			l->y = (pos / cols) * hSpace;
			pos++;
		}
		
	}
}

void Layer::alignChildrenToPixels() {
	for(int i  = 0; i < getNumChildren(); i++) {
		getChild(i)->alignToPixels();
	}
}

