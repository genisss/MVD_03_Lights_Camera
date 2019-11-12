//
//  Game.cpp
//
//  Copyright � 2018 Alun Evans. All rights reserved.
//

#include "Game.h"
#include "Shader.h"
#include "extern.h"
#include "Parsers.h"

Game::Game() {

}

//Nothing here yet
void Game::init() {

	//******** CREATE AND ADD RESOURCES **********//

	//init graphics system
	graphics_system_.init();

	//shaders
	graphics_system_.loadShader("phong", "data/shaders/phong.vert", "data/shaders/phong.frag");

	//geometries
	//int teapot_geom_id = graphics_system_.createGeometryFromFile("data/assets/teapot_small.obj");
	int teapot_geom_id = graphics_system_.createPlaneGeometry();

	//materials and textures
	int default_mat_id = graphics_system_.createMaterial();
	graphics_system_.getMaterial(default_mat_id).diffuse_map = Parsers::parseTexture("data/assets/test.tga");
	graphics_system_.getMaterial(default_mat_id).shader_id = graphics_system_.getShaderProgram("phong");

	//******* CREATE ENTITIES AND ADD COMPONENTS *******//
	//don't forget that transforms are automatically created

	int ent_teapot = ECS.createEntity("Teapot");
	Mesh& tmc = ECS.createComponentForEntity<Mesh>(ent_teapot);
	tmc.geometry = teapot_geom_id;
	tmc.material = default_mat_id;
	ECS.getComponentFromEntity<Transform>(ent_teapot).translate(0.0, -0.5, 0.0);
	ECS.getComponentFromEntity<Transform>(ent_teapot).rotateLocal(-0.7f, lm::vec3(0, 1, 0));

	//TODO in Components.h
	// - have a look at the light component (already defined)
	// - add camera component and properties (see comments in file)

	// TODO in EntityComponentStore
	// - note the 'main_camera' property - (index into array of camera components) 

	int ent_light_1 = ECS.createEntity("Light 1");
	Light& light_1_comp = ECS.createComponentForEntity<Light>(ent_light_1);
	light_1_comp.color = lm::vec3(1.0f, 0.0f, 0.0f);
	Transform& light_1_transform = ECS.getComponentFromEntity<Transform>(ent_light_1);
	light_1_transform.translate(1000.0f, 0.0f, 1000.0f);

	int ent_light_2 = ECS.createEntity("Light 1");
	Light& light_2_comp = ECS.createComponentForEntity<Light>(ent_light_1);
	light_2_comp.color = lm::vec3(0.0f, 1.0f, 0.0f);
	Transform& light_2_transform = ECS.getComponentFromEntity<Transform>(ent_light_1);
	light_2_transform.translate(1000.0f, 0.0f, 1000.0f);


	int ent_camera = ECS.createEntity("Camera 1");
	Camera& camera_1_comp = ECS.createComponentForEntity<Camera>(ent_camera);

	ECS.getComponentFromEntity<Transform>(ent_camera).translate(0, 0, 3.0f);
	camera_1_comp.position = lm::vec3(0.0f, 0.0f, 3.0f);
	camera_1_comp.forward = lm::vec3(0.0f, 0.0f, -1.f);
	camera_1_comp.setPerspective(60.0f * DEG2RAD, 1.0f, 0.01f, 100.0f);


	int ent_camera2 = ECS.createEntity("Camera 2");
	Camera& camera_2_comp = ECS.createComponentForEntity<Camera>(ent_camera2);

	ECS.getComponentFromEntity<Transform>(ent_camera2).translate(0, 0, 3.0f);
	camera_2_comp.position = lm::vec3(1.0f, 1.0f, 6.0f);
	camera_2_comp.forward = lm::vec3(0.0f, 0.0f, -1.f);
	camera_2_comp.setPerspective(60.0f * DEG2RAD, 1.0f, 0.01f, 100.0f);

	ECS.main_camera = 1;

	

	// - create entity for camera
	// - add camera component
	// - set transform component properties
	// - set camera component properties
	// - set ECS.main_camera

	//TODO in GraphicsSystem
	// - replace hard coded view and project matrices with camera component (ECS.main_camera)
	// - update camera matrices once per frame
	// - look at fragment shader to understand new lighting code
	// - basic: change rendering code to use Light component instead of hard-coded values
	// - advanced: look at modified shader, change rendering code upload multiple lights
	//			   then create several light entities of different colours to test them out
	// - extra advanced: modify light component and shader to support a Directional light (from infinity)
}

//Entry point for game update code
void Game::update(float dt) {
	//update each system in turn

	//render
	graphics_system_.update(dt);

}

