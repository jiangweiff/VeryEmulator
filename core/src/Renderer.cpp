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

void Renderer::UpdateDisplayFrame(GLsizei width, GLsizei height, const void* pixels)
{
    m_displayTexture.UpdateImage(Render::InternalFormat::RGB, width, height, Render::PixelFormat::RGB, Render::PixelType::UByte, pixels);
    // m_displayTexture.SubImage(0,0,240,240, Render::PixelFormat::RGB, Render::PixelType::UByte, m_pixels);
}

void Renderer::DisplayFrame()
{
	glDisable( GL_SCISSOR_TEST );
	glDisable( GL_BLEND );
	glDisable( GL_DEPTH_TEST );

	glViewport( 0, 0, m_displayTexture.GetWidth(), m_displayTexture.GetHeight() );
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