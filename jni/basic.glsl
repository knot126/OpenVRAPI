#ifdef VERTEX
in vec2 inPos;
in vec2 inTexCoord;
out vec2 fTexCoord;

void main() {
	gl_Position = vec4(inPos, 0, 1);
	fTexCoord = inTexCoord;
}
#endif

#ifdef FRAGMENT
uniform sampler2D uTexture;

in vec2 fTexCoord;
out vec4 fragColor;

void main() {
	fragColor = vec4(texture(uTexture, fTexCoord).rgb, 1.0);
// 	fragColor = texture(uTexture, fTexCoord);
// 	fragColor = vec4(1.0, 0.0, 0.0, 1.0);
}
#endif
