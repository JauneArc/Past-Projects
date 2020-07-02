#version 330 core

// interpolated values from the vertex shaders
in vec3 vColor;

// uniform input data
//uniform float uAlpha;


// output data
//out vec4 fColor;
out vec3 fColor;

void main()
{
	// set output color
	//fColor = vec4(vColor, uAlpha);
	fColor = vColor;
}