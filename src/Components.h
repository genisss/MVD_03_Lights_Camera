//
//  Copyright � 2018 Alun Evans. All rights reserved.
//
//  This file contains the definitions of an entity, and all the different types of component
//  At the end is a struct called the EntityComponentManager (ECM) which contains an std::vector for
//  each of the different component types, stored in an std::tuple. The advantage of this system is that
//  if a system wishes to interact/use/update all components of a certain type (e.g. draw all meshes),
//  then these components are stored in contiguous memory, which the various levels of caching can use
//  to improve performance
//
//    TO ADD A NEW COMPONENT TYPE:
//    - define it as a sub-class of Component
//    - add it to the ComponentArrays tuple
//    - add it as a subtemplate of typetoint() and increment 'result' variable
//    - increment NUM_TYPE_COMPONENTS
//
#pragma once
#include "includes.h"
#include <vector>

/**** COMPONENTS ****/

//Component (base class)
// - owner: id of Entity which owns the instance of the component
struct Component {
    int owner;
};

// Transform Component
// - inherits a mat4 which represents a model matrix
// - parent: id of Transform which represents this component's parent
struct Transform : public Component, public lm::mat4 {
    int parent = -1;
    lm::mat4 getGlobalMatrix(std::vector<Transform>& transforms) {
        if (parent != - 1){
            return transforms.at(parent).getGlobalMatrix(transforms) * *this;
        }
        else return *this;
    }
};

// Mesh Component
// - geometry - name of geometry resource
// - material - name of material resource
struct Mesh : public Component {
    int geometry;
    int material;
};


//TODO:
// - Create camera component
// - should have position, target and up vectors (lm::vec3) for rapid access
// - view, projection, and view_projection matrices (lm::mat4) for camera
// - a constructor to create default values
// - general update function (e.g. update view and view_projection matrix when camera changes)
struct Camera : public Component {
	
	lm::vec3 position;
	lm::vec3 up;
	lm::vec3 forward;
	lm::mat4 view_matrix;
	lm::mat4 projection_matrix;
	lm::mat4 view_projection;

	Camera(){
		position = lm::vec3(0.0f, 0.0f, 01.0f);
		forward = lm::vec3(0.0f, 0.0f, -1.0f);
		up = lm::vec3(0.0f, 1.0f, 0.0f);
		lm::vec3 target = position + forward;

		view_matrix.lookAt(position, target, up);
		projection_matrix.perspective(60.0f * DEG2RAD, 1, 0.01f, 100.0f);


	}

	void update() {
		//update viewMatrix based on current position and target
		updateViewMatrix();
		view_projection = projection_matrix * view_matrix;
	}

	void updateViewMatrix() {
		forward.normalize();
		lm::vec3 target = position + forward;
		view_matrix.lookAt(position, target, up);
	}

	void setPerspective(float fov_rad, float aspect, float near, float far) {

		projection_matrix.perspective(fov_rad , aspect, near, far);
	}

};

//Light Component
// - color of light
//Later will be developed extensively
struct Light : public Component {
    lm::vec3 color;
};


/**** COMPONENT STORAGE ****/

//add new component type vectors here to store them in *ECS*
typedef std::tuple<
std::vector<Transform>,
std::vector<Mesh>,
std::vector<Camera>,
std::vector<Light>
> ComponentArrays;

//way of mapping different types to an integer value i.e.
//the index within ComponentArrays
template< typename T >
struct type2int{};
template<> struct type2int<Transform> { enum { result = 0 }; };
template<> struct type2int<Mesh> { enum { result = 1 }; };
template<> struct type2int<Camera> { enum { result = 2 }; };
template<> struct type2int<Light> { enum { result = 3 }; };
//UPDATE THIS!
const int NUM_TYPE_COMPONENTS = 4;

/**** ENTITY ****/

struct Entity {
    //name is used to store entity
    std::string name;
    //array of handles into ECM component arrays
    int components[NUM_TYPE_COMPONENTS];
    //sets active or not
    bool active = true;

    Entity() {
        for (int i = 0; i < NUM_TYPE_COMPONENTS; i++) { components[i] = -1;}
    }
    Entity(std::string a_name) : name(a_name) {
        for (int i = 0; i < NUM_TYPE_COMPONENTS; i++) { components[i] = -1;}
    }
};

