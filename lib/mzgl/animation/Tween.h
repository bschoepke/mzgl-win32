//
//  Tween.h
//  mySketch
//
//  Created by Marek on 1/25/15.
//
//

#pragma once

#include "animation.h"
#include <functional>
//#include "util.h"
#include "Rectf.h"
#include <vector>

enum EaseType {
 
	EASE_LINEAR,
	EASE_OUT_CUBIC,
	EASE_IN_CUBIC,
	EASE_IN_OUT_CUBIC
};
// base type for different types of animation moves
class Animation {
public:
	virtual bool isRunning() = 0;
	virtual ~Animation() {}
};

template <class T>
class Tween_: public Animation {
public:
	
	T *valuePtr;
	
	std::function<void()> tweenComplete;
	
	void tweenTo(T &valuePtr, T to, float duration, EaseType type = EASE_LINEAR, float delay = 0);
	
	void start(T &valuePtr, T from, T to, float duration, EaseType type = EASE_LINEAR, float delay = 0);

	bool isRunning() override {
		return running;
	}
	
	bool isDone();
	
private:
	float startTime;
	float endTime;
	T from;
	T to;
	bool running = false;
	EaseType type;
	
	void update();
	
	float ease(float v);
};

typedef Tween_<float> Tween;

// not sure if this works
//typedef Tween_<glm::vec2> Tween2f;

class FunctionAnimation: public Animation {
public:
	float startTime;
	float endTime;
	bool running = false;
	
	float duration;
	std::function<void(float)> progressFunc;
	std::function<void()> completionFunc;
	FunctionAnimation(float duration, std::function<void(float)> progressFunc, std::function<void()> completionFunc);
	
	bool isRunning() override {
		return running;
	}
	void update();
	~FunctionAnimation();
};



class AnimationManager {
private:
	AnimationManager() {}
	
public:
	
	static AnimationManager *getInstance() {
		static AnimationManager *instance = NULL;
		if(instance==NULL) {
			instance = new AnimationManager();
		}
		return instance;
	}
	void animate(float duration, std::function<void(float)> progressFunc, std::function<void()> completionFunc);
	Tween &tweenTo(float &val, float to, float duration, EaseType easing = EASE_LINEAR, float delay = 0);
	Tween &tweenTo(glm::vec2 &val, glm::vec2 to, float duration, EaseType easing = EASE_LINEAR, float delay = 0);
	Tween &tweenTo(glm::vec3 &val, glm::vec3 to, float duration, EaseType easing = EASE_LINEAR, float delay = 0);
	Tween &tweenTo(glm::vec4 &val, glm::vec4 to, float duration, EaseType easing = EASE_LINEAR, float delay = 0);
	
	Tween &tweenTo(Rectf &val, Rectf to, float duration, EaseType easing = EASE_LINEAR, float delay = 0);
	
private:
	std::vector<Animation*> animations;
public: // just for now to test functionality
	void cleanUpCompletedAnimations();
};

#define ANIMATE(duration, progressFunc, completionFunc) AnimationManager::getInstance()->animate(duration, progressFunc, completionFunc)
#define TWEEN(var, to, duration, tween) AnimationManager::getInstance()->tweenTo(var, to, duration, tween)
#define EASE_IN(var, to, duration) AnimationManager::getInstance()->tweenTo(var, to, duration, EASE_IN_CUBIC)
#define EASE_OUT(var, to, duration) AnimationManager::getInstance()->tweenTo(var, to, duration, EASE_OUT_CUBIC)
#define EASE_IN_OUT(var, to, duration) AnimationManager::getInstance()->tweenTo(var, to, duration, EASE_IN_OUT_CUBIC)
#define EASE_IN_OUT_FN(var, to, duration, fn) AnimationManager::getInstance()->tweenTo(var, to, duration, EASE_IN_OUT_CUBIC, 0, fn)

#define EASE_IN_OUT_DELAY(var, to, duration, delay) AnimationManager::getInstance()->tweenTo(var, to, duration, EASE_IN_OUT_CUBIC, delay)

