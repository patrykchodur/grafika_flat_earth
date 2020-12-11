varying vec2 vTexCoord;

uniform sampler2D myTexture;

void main() 
{ 
	gl_FragColor = texture(myTexture, vTexCoord);
}
