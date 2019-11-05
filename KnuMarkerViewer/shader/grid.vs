#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexColor;

out vec3 outColor;

// Values that stay constant for the whole mesh.
uniform mat4 MVP;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
	// Output position of the vertex, in clip space : MVP * position
	//gl_Position =  MVP * vec4(vertexPosition, 1);
	gl_Position =  projection * view * model * vec4(vertexPosition, 1);
	outColor = vertexColor;
}

