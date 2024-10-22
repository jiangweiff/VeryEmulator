#include "Renderer.h"

bool Renderer::Initialize()
{
   	// m_vertexBuffer = Render::ArrayBuffer::Create<Vertex>( Render::BufferUsage::StreamDraw, VertexBufferSize );
    m_noAttributeVAO = Render::VertexArrayObject::Create();

    m_displayTexture = Render::Texture2D::Create(Render::InternalFormat::RGB, 240, 240, Render::PixelFormat::RGB, Render::PixelType::UByte);
   	m_vramViewShader = Render::Shader::Compile( VRamViewVertexShader, VRamViewFragmentShader );
	dbAssert( m_vramViewShader.Valid() );

    m_pixels = new uint8_t[240*240*3];
    return true;
}

void Renderer::DisplayFrame()
{
    int texsize = 240*240;
    for(int i = 0; i < texsize; ++i) {
        uint8_t c = rand() % 255;
        m_pixels[i*3] = m_pixels[i*3+1] = m_pixels[i*3+2] = c;
    }
    
    // m_displayTexture.UpdateImage(Render::InternalFormat::RGB, 240, 240, Render::PixelFormat::RGB, Render::PixelType::UByte, m_pixels);
    m_displayTexture.SubImage(0,0,240,240, Render::PixelFormat::RGB, Render::PixelType::UByte, m_pixels);

	glDisable( GL_SCISSOR_TEST );
	glDisable( GL_BLEND );
	glDisable( GL_DEPTH_TEST );

	glViewport( 0, 0, 240, 240 );
	glClearColor( 0.0f, 1.0f, 0.0f, 1.0f );
	glClear( GL_COLOR_BUFFER_BIT );

	// render to window
    GLenum error = glGetError();
    m_noAttributeVAO.Bind();
	m_vramViewShader.Bind();
	m_displayTexture.Bind();
    glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );

    error = glGetError();
}