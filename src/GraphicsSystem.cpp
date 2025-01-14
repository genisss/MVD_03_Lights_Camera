//
//  Copyright 2018 Alun Evans. All rights reserved.
//
#include "GraphicsSystem.h"
#include "Parsers.h"
#include "extern.h"

//destructor
GraphicsSystem::~GraphicsSystem() {
	//delete shader pointers
	for (auto shader_pair : shaders_) {
		if (shader_pair.second)
			delete shader_pair.second;
	}
}

//set initial state of graphics system
void GraphicsSystem::init() {
	//set 'background' colour of framebuffer
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

	//enable culling and depth test
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	time = 0.0f;
}

void GraphicsSystem::update(float dt) {

	time += dt;

	if (time > 100) {
		if (ECS.main_camera == 0) {
			ECS.main_camera = 1;
		}
		else {
			ECS.main_camera = 0;
		}
		time = 0;
	}

	//set initial OpenGL state
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	//TODO:
	// - update camera matrices here


	current_program_ = -1; //reset graphics component program

	//render all components using methods in graphicssystem

	//get reference to mesh components first
	auto& mesh_components = ECS.getAllComponents<Mesh>();

	//loop over mesh components by reference (&)
	for (auto& curr_comp : mesh_components) {
		//get shader id from material. if same, don't change
		if (current_program_ != materials_[curr_comp.material].shader_id) {
			current_program_ = materials_[curr_comp.material].shader_id;
			glUseProgram(current_program_);
		}
		//pass ref to render
		renderMeshComponent_(curr_comp);
	}


}

//renders a given mesh component
void GraphicsSystem::renderMeshComponent_(Mesh& comp) {

	//get transform of components entity
	Transform& transform = ECS.getComponentFromEntity<Transform>(comp.owner);

	//get Geometry, material and textures
	Geometry& geom = geometries_[comp.geometry];
	Material& mat = materials_[comp.material];

	//TODO:
	// - get a camera component using ECS.main_camera
	// - replace hard coded matrix data below, with matrices of camera component
	// - change camera position to send to u_cam_pos uniform

	//lm::vec3 cam_position(0.0f, 0.0f, 3.0f);
	//lm::vec3 cam_target(0.0f, 0.0f, 0.0f);
	//lm::vec3 cam_up(0.0f, 1.0f, 0.0f);
	//lm::mat4 view_matrix, projection_matrix, view_projection;
	//view_matrix.lookAt(cam_position, cam_target, cam_up);
	//projection_matrix.perspective(60.0f * DEG2RAD, 1, 0.01f, 100.0f);
	//view_projection = projection_matrix * view_matrix;


	Camera& cam = ECS.getAllComponents<Camera>()[ECS.main_camera];
	cam.update();

	//model matrix
	lm::mat4 model_matrix = transform.getGlobalMatrix(ECS.getAllComponents<Transform>());

	//normal matrix
	lm::mat4 normal_matrix = model_matrix;
	normal_matrix.inverse();
	normal_matrix.transpose();

	//Model view projection matrix
	lm::mat4 mvp_matrix = cam.view_projection * model_matrix;

	//transform uniforms
	GLint u_mvp = glGetUniformLocation(current_program_, "u_mvp");
	if (u_mvp != -1) glUniformMatrix4fv(u_mvp, 1, GL_FALSE, mvp_matrix.m);

	GLint u_model = glGetUniformLocation(current_program_, "u_model");
	if (u_model != -1) glUniformMatrix4fv(u_model, 1, GL_FALSE, model_matrix.m);

	GLint u_normal_matrix = glGetUniformLocation(current_program_, "u_normal_matrix");
	if (u_normal_matrix != -1) glUniformMatrix4fv(u_normal_matrix, 1, GL_FALSE, normal_matrix.m);

	//TODO: 
	GLint u_cam_pos = glGetUniformLocation(current_program_, "u_cam_pos");
	if (u_cam_pos != -1) glUniform3fv(u_cam_pos, 1, cam.position.value_); // ...3fv - is array of 3 floats

	//material uniforms
	GLint u_diffuse = glGetUniformLocation(current_program_, "u_diffuse");
	if (u_diffuse != -1) glUniform3fv(u_diffuse, 1, mat.diffuse.value_);

	GLint u_specular = glGetUniformLocation(current_program_, "u_specular");
	if (u_specular != -1) glUniform3fv(u_specular, 1, mat.specular.value_);

	GLint u_specular_gloss = glGetUniformLocation(current_program_, "u_specular_gloss");
	if (u_specular_gloss != -1) glUniform1f(u_specular_gloss, 200.0f); //...1f - for float

	//texture uniforms
	GLint u_diffuse_map = glGetUniformLocation(current_program_, "u_diffuse_map");
	if (u_diffuse_map != -1) glUniform1i(u_diffuse_map, 0); // ...1i - is integer
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mat.diffuse_map);


	//TODO
	// - look at fragment shader to understand new lighting code
	// - modify light code to read light information from Light components in ECS
	auto& all_lights = ECS.getAllComponents<Light>();

	//tell shader how many lights there
	GLint u_num_lights = glGetUniformLocation(current_program_, "u_num_lights"); //get/set uniform in shader
	if (u_num_lights != -1) glUniform1i(u_num_lights, (int)all_lights.size());
	
	for (size_t i = 0; i < all_lights.size(); i++) {
		Transform& light_i_transform = ECS.getComponentFromEntity<Transform>(all_lights[i].owner);
		//position
		std::string light_position_name = "lights[" + std::to_string(i) + "].position"; // uniform name
		GLint u_light_pos = glGetUniformLocation(current_program_, light_position_name.c_str()); //find it
		if (u_light_pos != -1) glUniform3fv(u_light_pos, 1, light_i_transform.position().value_); // light position
		//color
		std::string light_color_name = "lights[" + std::to_string(i) + "].color";
		GLint u_light_col = glGetUniformLocation(current_program_, light_color_name.c_str());
		if (u_light_col != -1) glUniform3fv(u_light_col, 1, all_lights[i].color.value_);
	}




	//***** drawing code *****//

	//tell OpenGL we want to the the vao_ container with our buffers
	glBindVertexArray(geom.vao);
	//draw our geometry
	glDrawElements(GL_TRIANGLES, geom.num_tris * 3, GL_UNSIGNED_INT, 0);
	//tell OpenGL we don't want to use our container anymore
	glBindVertexArray(0);

}

//********************************************
// Adding and creating functions
//********************************************

//loads a shader and returns its program id
int GraphicsSystem::loadShader(std::string name, std::string vs_path, std::string fs_path) {
	shaders_[name] = new Shader(vs_path, fs_path);
	return shaders_[name]->program;
}

//returns a shader program id
int GraphicsSystem::getShaderProgram(std::string name) {
	return shaders_[name]->program;
}

//create a new material and return pointer to it
int GraphicsSystem::createMaterial() {
	materials_.emplace_back();
	return (int)materials_.size() - 1;
}

//creates a standard plane geometry and return its
int GraphicsSystem::createPlaneGeometry() {

	std::vector<GLfloat> vertices, uvs, normals;
	std::vector<GLuint> indices;
	vertices = { -0.5f, -0.5f, 0.0f,    0.5f, -0.5f, 0.0f, 0.5f,  0.5f, 0.0f, -0.5f, 0.5f, 0.0f };
	uvs = { 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f };
	normals = { 0.0f, 0.0f, 1.0f,    0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,    0.0f, 0.0f, 1.0f };
	indices = { 0, 1, 2, 0, 2, 3 };
	//generate the OpenGL buffers and create geometry
	GLuint vao = generateBuffers_(vertices, uvs, normals, indices);
	geometries_.emplace_back(vao, 2);

	return (int)geometries_.size() - 1;
}

//create geometry from an external file
//returns index in geometry array with stored geometry data
int GraphicsSystem::createGeometryFromFile(std::string filename) {

	//these are the final arrays we will use to send to the GPU
	std::vector<GLfloat> vertices, uvs, normals;
	std::vector<GLuint> indices;

	//check for supported format
	std::string ext = filename.substr(filename.size() - 4, 4);
	if (ext == ".obj" || ext == ".OBJ")
	{
		//fill it with data from object
		if (Parsers::parseOBJ(filename, vertices, uvs, normals, indices)) {

			//generate the OpenGL buffers and create geometry
			GLuint vao = generateBuffers_(vertices, uvs, normals, indices);
			geometries_.emplace_back(vao, (GLuint)indices.size() / 3);
			return (int)geometries_.size() - 1;
		}
		else {
			std::cerr << "ERROR: Could not parse mesh file" << std::endl;
			return -1;
		}
	}
	else {
		std::cerr << "ERROR: Unsupported mesh format when creating geometry" << std::endl;
		return -1;
	}

}

//generates buffers in VRAM and returns VAO handle.
GLuint GraphicsSystem::generateBuffers_(std::vector<float>& vertices, std::vector<float>& uvs, std::vector<float>& normals, std::vector<unsigned int>& indices) {
	//generate and bind vao
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	GLuint vbo;
	//positions
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &(vertices[0]), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	//texture coords
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(float), &(uvs[0]), GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	//normals
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float), &(normals[0]), GL_STATIC_DRAW);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	//indices
	GLuint ibo;
	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &(indices[0]), GL_STATIC_DRAW);
	//unbind
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	return vao;
}

