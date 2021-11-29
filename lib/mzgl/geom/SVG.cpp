//
//  SVG.cpp
//  roundedtexrect
//
//  Created by Marek Bereza on 02/03/2018.
//  Copyright © 2018 Marek Bereza. All rights reserved.
//

#include "SVG.h"
#include "util.h"
#include "Triangulator.h"
#include "MitredLine.h"
#include <algorithm>
#include "log.h"
#include "RoundedRect.h"
#include <glm/gtc/type_ptr.hpp>
#include "pugixml.hpp"

// TODO: optimization - only transform verts if there's a transformation
using namespace std;




bool isNumeric(int c) {
	if(c-'0' <=9 && c-'0'>=0) return true;
	if(c=='.' || c=='-') return true;
	return false;
}

glm::vec4 parseColor(const string &hex) {
	if(hex[0]!='#') {
		
		if(hex=="none") {
			return {0,0,0,0};
		} else {
			Log::e() << "ERROR: color does not have a # '"<<hex<<"'";
		}
	}
	string s = hex.substr(1);
	int c = (int)strtol(s.c_str(), NULL, 16);
	return hexColor(c);
}

// TODO: replace with glm version
void doBezierCubic (glm::vec2 p0, glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, vector<glm::vec2> &outVerts) {
	
	float dist = distance(p0,p3);
	float numSteps = dist / SVG_CUBIC_RESOLUTION;
	float increment = 1.f/numSteps;
	//printf("cubic numSteps: %.0f\n", numSteps);
	for(float t = increment; t < 1; t += increment) {
		float u = 1.f - t;
		float t2 = t * t;
		float u2 = u * u;
		float u3 = u2 * u;
		float t3 = t2 * t;
		
		outVerts.push_back((u3) * p0 + (3.f * u2 * t) * p1 + (3.f * u * t2) * p2 + (t3) * p3);
	}
	outVerts.push_back(p3);
}

glm::mat3 parseTransform(string tr) {
	glm::mat3 transform = {1,0,0,0,1,0,0,0,1};
	auto t = split(tr, ")");
	for(auto &a : t) {
		if(a.find("translate")!=-1) {
			
			auto parts = split(a, "(");
			auto vals = split(parts[1], ",");
			glm::vec2 translation(stof(vals[0]), stof(vals[1]));
			
			glm::mat3 m = {
				1, 0, 0,
				0, 1, 0,
				translation.x, translation.y, 1
			};
			
			transform *= m;
			
		} else if(a.find("rotate")!=-1) {
			auto parts = split(a, "(");
			float rotation = stof(parts[1]);
			float theta = rotation * M_PI / 180.f;
			
			
			glm::mat3 m = {
				
				cos(theta),sin(theta),0,
				-sin(theta),cos(theta),0,
				0,           0,          1
			};
			
			transform *= m;
		}
	}
	
	// this bit removes any very small numbers
	// in the resulting transform, because
	// they can cause slightly whacky things
	// in the rendering.
	float *v = glm::value_ptr(transform);
	for(int i = 0; i < 9; i++) {
		if(abs(v[i])<0.0001) v[i] = 0;
	}
	
	return transform;
}



class SVGShape: public SVGNode {
public:
	
	static SVGShapeRef create(pugi::xml_node &n) {
		return SVGShapeRef(new SVGShape(n));
	}
	
	string id = "";
	vector<vector<glm::vec2>> verts;
	glm::vec4 strokeColor;
	glm::vec4 fillColor;
	glm::mat3 transform;
	
	float strokeWeight = 1;
	bool strokeWeightSet = false;
	bool filled = true;
	bool stroked = false;
	bool closed = false;
	
	void getOutlines(vector<glm::vec2> &outlines) override {
		if(verts.size()==0) {
			Log::e() << "ERROR: shape '"<< id <<"' with no points";
			return;
		}
		for(auto &v : verts) {
			for(int i = 0; i < v.size()-1; i++) {
				outlines.push_back(v[i]);
				outlines.push_back(v[i+1]);
			}
		}
	}
	
	
	
	void getTriangles(vector<glm::vec2> &outVerts, vector<glm::vec4> &outCols, vector<unsigned int> &indices) override {
		
		if(filled) {
            for(int i = 0; i < verts.size(); i++) {
                if(verts[i].size()<3) {
                    Log::e() << "ERROR: shape '"<< id<<"', " << to_string(i) << " with less than 3 points";
                }
            }
			
//				else {
				Triangulator tri;
				int numVerts = tri.triangulate(verts, outVerts, indices);
				outCols.insert(outCols.end(), numVerts, fillColor);
//			}
		}
		if(stroked) {
			MitredLine ml;
			ml.thickness = strokeWeight;
			for(auto &v : verts) {
//				auto &v = verts[0];
				int numVerts = ml.getVerts(v, outVerts, indices, closed);
				outCols.insert(outCols.end(), numVerts, strokeColor);
			}
		}
	}
	
	void applyState(SVGState state = SVGState()) override {
		
		fillColor.a *= state.opacity;
		strokeColor.a *= state.opacity;
		state.transform *= transform;
		
		applyTransformToPoints(state.transform);
		if(!filled) {
			if(state.filling) {
				filled = true;
				fillColor = glm::vec4(
									  state.fillColor.r,
									  state.fillColor.g,
									  state.fillColor.b,
									  state.fillOpacity * state.opacity);
			}
		}
		
		if(!stroked) {
			if(state.stroking) {
				stroked = true;
				strokeColor = glm::vec4(
										state.strokeColor.r,
										state.strokeColor.g,
										state.strokeColor.b,
										state.strokeOpacity * state.opacity);
			}
		}
		if(stroked && !strokeWeightSet) {
			strokeWeight = state.strokeWeight;
		}
	}
	
	void mirrorX() override {
		applyTransformToPoints({
			-1,0,0,
			0,1,0,
			0,0,1});
		for(auto &v : verts) {
			std::reverse(v.begin(), v.end());
		}
	}
	
	void rotate(float theta) override {
		applyTransformToPoints({
			cos(theta),-sin(theta),0,
			sin(theta),cos(theta),0,
			0,0,1});
		
	}
	void mirrorY() override {
		applyTransformToPoints({
			1,0,0,
			0,-1,0,
			0,0,1});
		for(auto &v : verts) {
			std::reverse(v.begin(), v.end());
		}
	}
	
	void scale(float s) override {
		applyTransformToPoints({
			s,0,0,
			0,s,0,
			0,0,1});
		strokeWeight *= s;
	}
	void translate(float x, float y) override {
		applyTransformToPoints({
			1,0,0,
			0,1,0,
			x,y,1});
	}

	
	void setUseInfo(pugi::xml_node &n) {
		if(n.attribute("stroke-width")) {
			strokeWeightSet = true;
			strokeWeight = n.attribute("stroke-width").as_float();
		}
		if(n.attribute("fill")) {
			filled = true;
			fillColor = parseColor(n.attribute("fill").value());
			if(n.attribute("fill-opacity")) {
				fillColor.a = n.attribute("fill-opacity").as_float();
			}
		} else {
			filled = false;
		}
		
		if(n.attribute("stroke")) {
			stroked = true;
			
			strokeColor = parseColor(n.attribute("stroke").value());
			if(n.attribute("stroke-opacity")) {
				strokeColor.a = n.attribute("stroke-opacity").as_float();
			}
		}
		if(n.attribute("transform")) {
			transform = parseTransform(n.attribute("transform").value());
		}
		if(n.attribute("opacity")) {
			strokeColor.a *= n.attribute("opacity").as_float();
			fillColor.a *= n.attribute("opacity").as_float();
		}
	}
	
private:
	void applyTransformToPoints(glm::mat3 transform) {
		for(auto &v : verts) {
			for(int i = 0; i < v.size(); i++) {
				glm::vec3 a(v[i].x, v[i].y, 1);
				v[i] = transform * a;
			}
		}
	}
	
	void parseRect(pugi::xml_node &n) {
		Rectf r(n.attribute("x").as_float(), n.attribute("y").as_float(), n.attribute("width").as_float(), n.attribute("height").as_float());
		float radius = 0;
		if(n.attribute("rx")) {
			radius = n.attribute("rx").as_float();
		}
		verts.push_back(vector<glm::vec2>());
		if(radius==0) {
			verts.back() = {r.tl(), r.tr(), r.br(), r.bl()};
		} else {
			getRoundedRectVerts(r, radius, verts.back());
			verts.back().pop_back();
		}
		closed = true;
	}
	
	void parseCircle(pugi::xml_node &n) {
		glm::vec2 c(n.attribute("cx").as_float(), n.attribute("cy").as_float());
		float radius = n.attribute("r").as_float();
		doEllipse(c, radius, radius);
	}
	
	void parseEllipse(pugi::xml_node &n) {
		glm::vec2 c(n.attribute("cx").as_float(), n.attribute("cy").as_float());
		float rx = n.attribute("rx").as_float();
		float ry = n.attribute("ry").as_float();
		doEllipse(c, rx, ry);
	}
	
	void doEllipse(glm::vec2 c, float rx, float ry) {
		verts.push_back(vector<glm::vec2>());
		float pixelsPerStep = SVG_CIRCLE_RESOLUTION;
		float step = asin(pixelsPerStep / 2.f / ((rx+ry)*0.5)) / 2.f;
		for(float f = 0; f < M_PI * 2.f; f+= step) {
			verts.back().push_back(glm::vec2(c.x + rx*cos(f), c.y + ry*sin(f)));
		}
		closed = true;
	}
	
	void parsePolygon(pugi::xml_node &n) {
		
		vector<string> points = split(n.attribute("points").value(), " ");
		verts.push_back(vector<glm::vec2>(points.size()/2));
		for(int i = 0; i < points.size(); i+=2) {
			verts.back()[i/2] = glm::vec2(stof(points[i]), stof(points[i+1]));
		}
		string tagName = n.name();
		if(tagName=="polygon") {
			closed = true;
		} else if(tagName=="polyline") {
			closed = false;
		}
	}
	
	void parsePath(pugi::xml_node &n) {
		string d = n.attribute("d").value();
		auto s = split(d, " ");
		for(int i = 0; i <s.size(); i++) {
			if(!isNumeric(s[i][0])) {
				char instr = s[i][0];
				s[i] = s[i].substr(1);
				if(instr=='Z') {
					
				} else if(instr=='M') {
					
					verts.push_back(vector<glm::vec2>());
					auto xy = split(s[i],",");
					verts.back().push_back(glm::vec2(stof(xy[0]), stof(xy[1])));
					
				} else if(instr=='C') {
					
					
					auto xy = split(s[i],",");
					auto p1 = glm::vec2(stof(xy[0]), stof(xy[1]));
					i++;
					
					xy = split(s[i],",");
					auto p2 = glm::vec2(stof(xy[0]), stof(xy[1]));
					i++;
					xy = split(s[i],",");
					auto p3 = glm::vec2(stof(xy[0]), stof(xy[1]));
					
					doBezierCubic(verts.back().back(), p1, p2, p3, verts.back());
					
				} else if(instr=='L') {
					auto xy = split(s[i],",");
					verts.back().push_back(glm::vec2(stof(xy[0]), stof(xy[1])));
				}
			}
		}
		if(verts[0][0]==verts[0].back()) {
			verts[0].pop_back();
			closed = true;
		}
	}
	
	// groups can have opacity
	// but shapes have fill-opacity and stroke-opacity
	SVGShape(pugi::xml_node &n) {
		transform = {1,0,0,0,1,0,0,0,1};
		string name = n.name();
		id = n.attribute("id").value();
		
		setUseInfo(n);
		
		if(name=="rect") {
			parseRect(n);
		} else if(name=="circle") {
			parseCircle(n);
		} else if(name=="ellipse") {
			parseEllipse(n);
		} else if(name=="polygon" || name=="polyline") {
			parsePolygon(n);
		} else if(name=="path") {
			parsePath(n);
		} else {
			Log::e() << "ERROR: don't know how to parse " << name;
		}
	}
};


class SVGGroup: public SVGNode {
public:
	
	
	glm::mat3 transform;
	
	
	float opacity = 1;
	bool fillSet = false;
	bool strokeSet = false;
	bool strokeWeightSet = false;
	bool fillOpacitySet = false;
	bool strokeOpacitySet = false;
	
	bool filling = false;
	glm::vec3 fillColor;
	
	bool stroking = false;
	glm::vec3 strokeColor;
	
	float strokeWeight = 1;
	
	float fillOpacity = 1;
	
	float strokeOpacity = 1;
	
	
	static SVGGroupRef create(pugi::xml_node &n, map<string,SVGShapeRef> &defs) {
		return SVGGroupRef(new SVGGroup(n, defs));
	}
	
	void getOutlines(vector<glm::vec2> &outlines) override {
		for(auto c: children) {
			c->getOutlines(outlines);
		}
	}
	
	void getTriangles(vector<glm::vec2> &verts, vector<glm::vec4> &cols, vector<unsigned int> &indices) override {
		for(auto c: children) {
			c->getTriangles(verts, cols, indices);
		}
	}
	
	
	void scale(float s) override {
		for(auto c: children) {
			c->scale(s);
		}
	}
	
	void mirrorX() override {
		for(auto c: children) {
			c->mirrorX();
		}
	}
	
	void mirrorY() override {
		for(auto c: children) {
			c->mirrorY();
		}
	}
	void rotate(float theta) override {
		for(auto c: children) {
			c->rotate(theta);
		}
	}
	
	
	void translate(float x, float y) override {
		for(auto c: children) {
			c->translate(x,y);
		}
	}
	// after the svg tree of groups and shapes has been made,
	// this recursive call propagates all the transforms and opacity values
	void applyState(SVGState state = SVGState()) override {
		
		state.opacity *= opacity;
		state.transform *= transform;
		
		
		if(fillSet) {
			state.filling = filling;
			state.fillColor = fillColor;
		}
		if(fillOpacitySet) {
			state.fillOpacity = fillOpacity;
		}
		
		if(strokeSet) {
			state.stroking = stroking;
			state.strokeColor = strokeColor;
		}
		
		if(strokeOpacitySet) {
			state.strokeOpacity = strokeOpacity;
		}
		
		if(strokeWeightSet) {
			state.strokeWeight = strokeWeight;
		}
		
		for(auto c: children) {
			c->applyState(state);
		}
	}
	
private:
	
	SVGGroup(pugi::xml_node &n, map<string,SVGShapeRef> &defs) {
		
		transform = {1,0,0,0,1,0,0,0,1};
		
		if(n.attribute("transform")) {
			transform = parseTransform(n.attribute("transform").value());
		}
		if(n.attribute("opacity")) {
			opacity = n.attribute("opacity").as_float();
		}
		
		if(n.attribute("fill")) {
			string fillStr = n.attribute("fill").value();
			if(fillStr!="" && fillStr[0]=='#') {
				fillSet = true;
				filling = fillStr!="none";
				if(filling) {
					glm::vec4 c = parseColor(fillStr);
					fillColor = glm::vec3(c.r, c.g, c.b);
				}
			}
		}
		
		if(n.attribute("fill-opacity")) {
			fillOpacity = n.attribute("fill-opacity").as_float();
			fillOpacitySet = true;
		}
		
		if(n.attribute("stroke")) {
			string strokeStr = n.attribute("stroke").value();
			if(strokeStr!="" && strokeStr[0]=='#') {
				strokeSet = true;
				stroking = strokeStr!="none";
				if(stroking) {
					glm::vec4 c = parseColor(strokeStr);
					strokeColor = glm::vec3(c.r, c.g, c.b);
				}
			}
		}
		
		if(n.attribute("stroke-opacity")) {
			strokeOpacity = n.attribute("stroke-opacity").as_float();
			strokeOpacitySet = true;
		}
		
		if(n.attribute("stroke-width")) {
			strokeWeight = n.attribute("stroke-width").as_float();
			strokeWeightSet = true;
		}
		
		for(auto &c: n) {
			string tag = c.name();
			if(tag=="desc" || tag=="defs" || tag=="title") {
				// ignore, already parsed
			} else if(tag=="g") {
				children.push_back(SVGGroup::create(c, defs));
			} else if(tag=="use") {
				if(c.attribute("xlink:href")) {
					string id = c.attribute("xlink:href").value();
					id = id.substr(1); // remove #
					SVGShapeRef use = SVGShapeRef(new SVGShape(*defs.at(id)));
					children.push_back(use);
					use->setUseInfo(c);
				} else {
					Log::e() << "ERROR: use without href";
				}
			} else {
				children.push_back(SVGShape::create(c));
			}
		}
	}
};

void SVGDoc::parseViewBox(string s) {
	auto p = split(s, " ");
	viewBox.x 		= stoi(p[0]);
	viewBox.y 		= stoi(p[1]);
	viewBox.width 	= stoi(p[2]);
	viewBox.height 	= stoi(p[3]);
	// printf("%f %f %f %f\n", viewBox.x, viewBox.y, viewBox.width, viewBox.height);
}

void SVGDoc::parseDefs(const pugi::xml_node &root) {
	auto xpathNode = root.select_single_node("defs");
	if (xpathNode) {
		auto defsNode = xpathNode.node();
		for(auto &def : defsNode) {
			SVGShapeRef shape = SVGShape::create(def);
			
			defs[shape->id] = shape;
		}
	}
}
void SVGDoc::parse(const pugi::xml_node &n, int depth) {
	for(auto &c : n) {
		parse(c, depth+1);
	}
}

void SVGDoc::scale(float s) {
	rootGroup->scale(s);
	width *= s;
	height *= s;
}
void SVGDoc::mirrorX() {
	rootGroup->mirrorX();
}
void SVGDoc::mirrorY() {
	rootGroup->mirrorY();
}
void SVGDoc::rotate(float theta) {
	rootGroup->rotate(theta);
}

#ifdef __ANDROID__
#include "androidUtil.h"

#endif
void SVGDoc::translate(float x, float y) {
	rootGroup->translate(x,y);
}

bool SVGDoc::loadFromString(const string &svgData) {
	defs.clear();
	pugi::xml_document doc;
	auto status = doc.load_string(svgData.c_str());
	if(status.status!=pugi::status_ok) {
		
		Log::e() << "ERROR: couldn't load SVG from string - must be a parse error - msg is "<<status.description() << " - at character " << status.offset;
		Log::e() << svgData;
		return false;
	}
	pugi::xml_node root = doc.document_element();
	
	parseViewBox(root.attribute("viewBox").value());
	width = viewBox.width;
	height = viewBox.height;
	parseDefs(root);
	
	rootGroup = SVGGroup::create(root, defs);
	
	rootGroup->applyState();
	return true;
}

bool SVGDoc::load(string path) {
	defs.clear();
	pugi::xml_document doc;

#ifdef __ANDROID__
	vector<unsigned char> data;
	loadAndroidAsset(path, data);
	auto status = doc.load_buffer(data.data(), data.size());

	if(status.status!=pugi::status_ok) {
		Log::e() << "Error: Couldn't load svg from buffer - got " << data.size() << "bytes - message from pugi is " << status.description()  << " - at character " << status.offset;
		return false;
	}
#else
	auto status = doc.load_file(path.c_str());
	if(status.status!=pugi::status_ok) {
		Log::e() << "ERROR: could not load svg - pugi says " << status.description() << " - at character " << status.offset;
		return false;
	}
#endif


	
	pugi::xml_node root = doc.document_element();
	
	parseViewBox(root.attribute("viewBox").value());
	width = viewBox.width;
	height = viewBox.height;
	parseDefs(root);
	
	rootGroup = SVGGroup::create(root, defs);
	
	rootGroup->applyState();
	return true;
}

void SVGDoc::getTriangles(vector<glm::vec2> &verts, vector<glm::vec4> &cols, vector<unsigned int> &indices) {
	rootGroup->getTriangles(verts, cols, indices);
}

void SVGDoc::getOutlines(vector<glm::vec2> &verts) {
	rootGroup->getOutlines(verts);
}

