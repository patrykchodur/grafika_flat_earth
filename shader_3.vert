uniform float time;
varying vec2 vTexCoord;

void main()
{
	vTexCoord = gl_MultiTexCoord0.xy;
	vec4 myVertexLoc = gl_Vertex;

	myVertexLoc.y = 0.2*sin((gl_Vertex.x*2.0) - time)*cos((gl_Vertex.z*2.0) + time);

	gl_Position = gl_ModelViewProjectionMatrix * myVertexLoc;
}
