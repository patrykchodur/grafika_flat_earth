uniform float time;
varying vec2 vTexCoord;

void main()
{
	vTexCoord = gl_MultiTexCoord0.xy;
	vec4 myVertexLoc = gl_Vertex;

	myVertexLoc.z = 0.001*sin((gl_Vertex.x*100.0) - time)*cos((gl_Vertex.y*100.0) + time);

	gl_Position = gl_ModelViewProjectionMatrix * myVertexLoc;
}
