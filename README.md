### La Salle - Universitat Ramon Llull : Master in Advanced Videogame Development
## Engine Programming - 03-Lights_Camera

### TODO

#### in Components.h
 - have a look at the light component (already defined)
 - add camera component and properties (see comments in file)

#### in EntityComponentStore
 - note the 'main_camera' property - (index into array of camera components) 

#### in here in Game::init:
 - create entity for light
 - add light component
 - set transform component properties 
 - set light component properties

 - create entity for camera
 - add camera component
 - set transform component properties
 - set camera component properties
 - set ECS.main_camera

#### in GraphicsSystem
 - replace hard coded view and project matrices with camera component (ECS.main_camera)
 - look at fragment shader to understand new lighting code
 - basic: change rendering code to use Light component instead of hard-coded values
 - advanced: look at modified shader, change rendering code upload multiple lights