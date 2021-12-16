//
//  EventDispatcher.h
//  MZGL
//
//  Created by Marek Bereza on 24/01/2018.
//  Copyright © 2018 Marek Bereza. All rights reserved.
//

#pragma once
#include "WebViewApp.h"


class EventDispatcher {

public:
	EventDispatcher(App *app) {
		this->app = app;
	}
	
	virtual ~EventDispatcher() {
		printf("Bye bye event dispatcher\n");
	}
	
	void androidDrawLoading() { app->androidDrawLoading(); }

	// KEYS
	void keyDown(int key) {
		app->keyDown(key);
	}
	
	void keyUp(int key) {
		app->keyUp(key);
	}
	
	virtual bool canOpenFiles(const std::vector<std::string> &paths) {
		return app->canOpenFiles(paths);
	}
	
//	virtual void fileDragBegin(float x, float y, int touchId, int numItems) {
//		app->fileDragBegin(x, y, touchId, numItems);
//	}
	
	virtual void fileDragUpdate(float x, float y, int touchId, int numItems) {
		app->fileDragUpdate(x, y, touchId, numItems);
	}
	
	void filesDropped(const std::vector<std::string> &paths, int touchId, std::function<void()> completionHandler) {
		app->filesDropped(paths,touchId, completionHandler);
	}
	
	void fileDragCancelled(int touchId) {
		return app->fileDragCancelled(touchId);
	}
	
	bool openUrl(std::string url) {
		return app->openUrl(url);
	}
	
	// TOUCHES
	void touchOver(float x, float y) {
		mouse = glm::vec2(x, y);
		app->touchOver(x, y);
		app->root->_touchOver(x, y);
	}
	void touchDown(float x, float y, int id) {
		mouse = glm::vec2(x, y);
		downTouches[id] = true;
		app->touchDown(x, y, id);
		app->root->_touchDown(x, y, id);
		
	}
	void touchMoved(float x, float y, int id) {
		mouse = glm::vec2(x, y);
		downTouches[id] = true;
		app->touchMoved(x, y, id);
		app->root->_touchMoved(x, y, id);
	}
	void touchUp(float x, float y, int id) {
		mouse = glm::vec2(x, y);
		downTouches[id] = false;
		
		app->touchUp(x, y, id);
		app->root->_touchUp(x, y, id);
	}
	
	void mouseScrolled(float x, float y, float dx, float dy) {
		app->mouseScrolled(x, y, dx, dy);
		app->root->_mouseScrolled(x, y, dx, dy);
	}
	
	void mouseZoomed(float x, float y, float zoom) {
		app->mouseZoomed(x, y, zoom);
		app->root->_mouseZoomed(x, y, zoom);
	}
	
	void didEnterBackground() {
		app->didEnterBackground();
	}
	
	void willEnterForeground() {
		app->willEnterForeground();
	}

	void memoryWarning() {
		app->memoryWarning();
	}

	void exit() {
		app->exit();
	}
	
	// EVENTS
	void resized() {
		app->resized();
	}
	
	// LIFE-CYCLE
	void setup() {
		app->setup();
        hasCalledSetup = true;
	}
    
    bool hasSetup() {
        return hasCalledSetup;
    }
    
	void update() {
		app->update();
		app->root->_update();
	}
	
	void draw() {
		app->draw();
		app->root->_draw();
		app->drawAfterUI();
	}
	
//	bool isReady() {return app!=nullptr;}
	App *app = nullptr;
	glm::vec2 mouse;
	std::map<int, bool> downTouches;
    bool hasCalledSetup = false;
	
	
	void runFrame() {
	#ifdef DO_DRAW_STATS
		Vbo::resetDrawStats();
	#endif
		app->g.setupView();
		updateInternal();
		update();
		callUpdateListeners();
		draw();
		callDrawListeners();
	}
    
    virtual void receivedJSMessage(const std::string &key, const std::string &value) {
        ((WebViewApp*)app)->receivedJSMessage(key, value);
    }
};
