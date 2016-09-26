"                                                       \n\
#ifdef GL_ES                                            \n\
precision lowp float;                                   \n\
#endif                                                  \n\
varying vec4 v_fragmentColor;                           \n\
varying vec2 v_texCoord;                                \n\
uniform sampler2D u_texture;                            \n\
void main()                                             \n\
{                                                       \n\
    float ratio=0.0;                                    \n\
    vec4 texColor = texture2D(u_texture, v_texCoord);   \n\
    ratio = texColor[0] > texColor[1]?(texColor[0] > texColor[2] ? texColor[0] : texColor[2]) :(texColor[1] > texColor[2]? texColor[1] : texColor[2]);                                      \n\
    if (ratio != 0.0)                                          \n\
    {   if(ratio < 0.1){                                                      \n\
texColor[0] = texColor[0] /  ratio;                    \n\
texColor[1] = texColor[1] /  ratio;                    \n\
texColor[2] = texColor[2] /  ratio;                    \n\
texColor[3] = ratio;                                   \n\                               \n\
        }                                   \n\
    }                                                          \n\
    else                                                       \n\
    {                                                          \n\
        texColor[3] = 0.0;                                     \n\
    }                                                          \n\
    gl_FragColor = v_fragmentColor*texColor;                   \n\
}";