//
//  Rectf.hpp
//  samploid
//
//  Created by Marek Bereza on 16/02/2018.
//  Copyright © 2018 Marek Bereza. All rights reserved.
//

#pragma once

#include <glm/glm.hpp>

// some of this is taken from openframeworks:
// https://github.com/darrenmothersele/openFrameworks/blob/master/types/ofRectangle.cpp
class Rectf {
public:
	float x = 0;
	float y = 0;
	float width = 0;
	float height = 0;
	Rectf() {}
	Rectf(float x, float y, float width, float height) :
	x(x),
	y(y),
	width(width),
	height(height)
	{}
	
	void set(float x, float y, float w, float h);
	
	void set(const Rectf &r);
	
	
	bool inside(float x, float y) const;
	bool inside(glm::vec2 p) const;
	bool inside(const Rectf &r) const;
	bool intersects(const Rectf &r) const;
	void setFromCentre(float cx, float cy, float w, float h);
	void setFromCentre(glm::vec2 c, float w, float h) { setFromCentre(c.x, c.y, w, h); }
	
	glm::vec2 tl() const { return glm::vec2(x, y); }
	glm::vec2 tr() const { return glm::vec2(x + width, y); }
	glm::vec2 bl() const { return glm::vec2(x, y + height); }
	glm::vec2 br() const { return glm::vec2(x + width, y + height); }
	float bottom() const { return y + height; }
	float right()  const { return x + width; }
	glm::vec2 centre() const {
		return glm::vec2(x + width / 2.f, y + height / 2.f);
	}
	glm::vec2 size() const { return glm::vec2(width, height); }
	void size(glm::vec2 s) { width = s.x; height = s.y; }
	
	
	void position(float x, float y) { this->x = x; this->y = y;}
	
	glm::vec2 position() const { return glm::vec2(x, y); }
	void position(glm::vec2 pos) {this->x = pos.x; this->y = pos.y;}
	
	
	// Rectf convenience methods
	void setCentre(float x, float y) { setFromCentre(x, y, this->width, this->height); }
	void setCentre(const glm::vec2 &c) { setCentre(c.x, c.y); }
	
	
	void setRight(float right) { this->x = right - this->width; }
	void setBottom(float bottom) { this->y = bottom - this->height; }
	
	
	void setTopLeft(float x, float y) { position(x, y); }
	void setTopLeft(glm::vec2 p) { position(p); }
	
	void setBottomLeft(float x, float y) { set(x, y-this->height, this->width, this->height); }
	void setBottomLeft(glm::vec2 p) { set(p.x, p.y-this->height, this->width, this->height); }
	
	void setBottomRight(float x, float y) { set(x - this->width, y-this->height, this->width, this->height); }
	void setBottomRight(glm::vec2 p) { set(p.x - this->width, p.y-this->height, this->width, this->height); }
	
	void setTopRight(float x, float y) { set(x - this->width, y, this->width, this->height); }
	void setTopRight(glm::vec2 p) { set(p.x - this->width, p.y, this->width, this->height); }
	
	// if there's a negative dimension, it positivizes it
	void normalize() {
		if(width<0) {
			width = -width;
			x -= width;
		}
		if(height<0) {
			height = -height;
			y -= height;
		}
	}
	
	float getMinX() const { return (width  > 0 ? x : (x+width));};
	float getMinY() const { return (height > 0 ? y : (y+height));};
	float getMaxX() const { return (width  > 0 ? (x+width)  : x);};
	float getMaxY() const { return (height > 0 ? (y+height) : y);};
	
	
	void growToInclude(glm::vec2 p);
	void growToInclude(const Rectf &other);
	void inset(float amt);
	void inset(glm::vec2 amt);
	void inset(float horiz, float vert);
	void inset(float top, float right, float bottom, float left);
	
	Rectf getIntersection(const Rectf &other) const;
	void alignToPixels();
	
	bool operator==(const Rectf &o) const {
		return x == o.x && y == o.y && width == o.width && height == o.height;
	}
	
	bool operator!=(const Rectf &o) const {
		return x != o.x || y != o.y || width != o.width || height != o.height;
	}
	
	Rectf &operator+=(const glm::vec2& offset) {
		x += offset.x;
		y += offset.y;
		return *this;
	}
	
	Rectf& operator*=(const float& rhs){
		
		x *= rhs;
		y *= rhs;
		width *= rhs;
		height *= rhs;
		
		return *this;
	}
		
};
