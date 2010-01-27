uniform sampler2D texture;
uniform sampler2D lightmap;

uniform vec2 WindowDimensions;
uniform vec2 LightPosition;
uniform float LightRange;
uniform vec3 LightColor;

void main(void) {
	
    float d = distance(gl_FragCoord, LightPosition);
    vec4 color = texture2D(texture, gl_TexCoord[0].xy);
    vec4 lightvalue = texture2D(lightmap, gl_TexCoord[0].xy);
    
    float lightIntensity = 0;
    if(d < LightRange)
    {
        lightIntensity = d/LightRange;
        lightIntensity = ((lightIntensity - 0.5) * -1) + 0.5;
       // lightIntensity -= lightvalue.r;
    }
    
    float stencil = ((lightvalue.r - 0.5) * -1) + 0.5;
                    // lightvalue; //
    
    //gl_FragColor = color * stencil;
    
    //gl_FragColor = color * clamp(stencil+(lightIntensity/3),0,1) * lightIntensity * vec4(LightColor,1);
    
    gl_FragColor = color * stencil * lightIntensity * vec4(LightColor,1); //(color * stencil * lightIntensity) * vec4(LightColor,1);
    //gl_FragColor = color * lightIntensity * vec4(LightColor,1) * stencil;
    
    
    //gl_FragColor = vec4(stencil, stencil, stencil, 1);
}