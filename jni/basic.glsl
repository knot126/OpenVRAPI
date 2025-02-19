varying vec2 texCoord;

#ifdef VERTEX
attribute vec2 inPos;
attribute vec2 inTexCoord;

void main() {
	gl_Position = vec4(inPos, 0, 0);
	texCoord = inTexCoord;
}
#endif

#ifdef FRAGMENT
uniform sampler2D uTexture;

void main() {
// 	gl_FragColor = texture2D(uTexture, texCoord);
	gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}
#endif
