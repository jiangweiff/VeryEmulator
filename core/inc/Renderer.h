#pragma once

#include <Render/VertexArrayObject.h>
#include <Render/Shader.h>
#include <Render/Texture.h>
#include <Render/Buffer.h>

const char* const VRamViewVertexShader = R"glsl(
#version 330 core

const vec2 positions[4] = vec2[]( vec2(-1.0, -1.0), vec2(1.0, -1.0), vec2(-1.0, 1.0), vec2(1.0, 1.0) );
const vec2 texCoords[4] = vec2[]( vec2(0.0, 1.0), vec2(1.0, 1.0), vec2(0.0, 0.0), vec2(1.0, 0.0) );

out vec2 TexCoord;

void main()
{
	TexCoord = texCoords[ gl_VertexID ];
	gl_Position = vec4( positions[ gl_VertexID ], 0.0, 1.0 );
}

)glsl";

const char* const VRamViewFragmentShader = R"glsl(
#version 330 core

in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D tex;

void main()
{
	FragColor = texture( tex, TexCoord );
}

)glsl";

class Renderer
{
public:
    bool Initialize();
    void DisplayFrame();
    void UpdateDisplayFrame(GLsizei width, GLsizei height, const void* pixels);
    
private:
    uint8_t* m_pixels;
    Render::VertexArrayObject m_noAttributeVAO;
    Render::Shader m_vramViewShader;
  	Render::Texture2D m_displayTexture;

};