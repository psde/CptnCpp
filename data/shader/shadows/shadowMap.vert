uniform vec2 LightPosition;
uniform float LightRange;
uniform vec2 screenpos;

void main(void)
{   
    vec4 screenPosition = vec4(gl_Vertex[0] + screenpos[0], gl_Vertex[1] + screenpos[1], gl_Vertex[2], gl_Vertex[3]);
    vec4 position = gl_ModelViewProjectionMatrix * screenPosition; //gl_Vertex;
    
    if(gl_Vertex.z == -1)
    {
        float foo = -50;
        float nx = (LightPosition[0] - gl_Vertex.x) * foo;
        float ny = (LightPosition[1] - gl_Vertex.y) * foo;
        
        vec2 lightVec = vec2(nx, ny);
        
        screenPosition[0] += nx;
        screenPosition[1] += ny;
        
        position = gl_ModelViewProjectionMatrix * screenPosition;
    }
    
    gl_Position = position;
    
    gl_TexCoord[0] = gl_MultiTexCoord0;
}
