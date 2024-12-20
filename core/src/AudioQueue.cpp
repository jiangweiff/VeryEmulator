#include "AudioQueue.h"

AudioQueue::BatchWriter::BatchWriter( AudioQueue& queue ) : m_queue{ queue }, m_lock{ queue.m_queueMutex }
{
	m_start = m_pos = m_queue.m_queue.get() + m_queue.m_last;
	m_batchSize = std::min( m_queue.m_bufferSize - m_queue.m_last, m_queue.m_bufferSize - m_queue.m_size );
}

AudioQueue::BatchWriter::~BatchWriter()
{
	if ( !m_queue.m_paused )
	{
		const size_t count = GetCount();

		// dbAssert( count <= m_batchSize );
		// dbAssert( m_queue.m_size + count <= m_queue.m_bufferSize );

		m_queue.m_last = ( m_queue.m_last + count ) % m_queue.m_bufferSize;
		m_queue.m_size += count;

		m_queue.CheckFullBuffer();
	}

	// release lock
}

void AudioQueue::Destroy()
{
	if ( m_deviceId != 0 )
	{
		SDL_CloseAudioDevice( m_deviceId );
		m_deviceId = 0;
	}
}

bool AudioQueue::Initialize( int frequency, uint8_t channels, uint16_t bufferSize )
{
	if ( channels != 1 && channels != 2 )
	{
		dbLogError( "AudioQueue::AudioQueue -- Invalid number of channels [%u]", (uint32_t)channels );
		return false;
	}

	SDL_AudioSpec request;
	request.freq = frequency;
	request.format = AUDIO_S16;
	request.channels = channels;
	request.samples = bufferSize;
	request.callback = &StaticFillAudioDeviceBuffer;
	request.userdata = this;

	SDL_AudioSpec obtained;
	const auto deviceId = SDL_OpenAudioDevice( nullptr, 0, &request, &obtained, 0 );

	if ( deviceId == 0 )
	{
		dbLogError( "AudioQueue::AudioQueue -- Cannot open audio device [%s]", SDL_GetError() );
		return false;
	}

	if ( request.freq != obtained.freq || request.format != obtained.format || request.channels != obtained.channels )
	{
		dbLogError( "AudioQueue::AudioQueue -- Obtained audio settings do not match requested settings" );
		return false;
	}

	Log( "audio buffer size: %u", (uint32_t)obtained.samples );

	m_deviceId = deviceId;
	m_settings = obtained;

	m_bufferSize = static_cast<size_t>( m_settings.freq * m_settings.channels );
	m_queue = std::make_unique<int16_t[]>( m_bufferSize );

	ClearInternal();

	SDL_PauseAudioDevice( m_deviceId, true );

	return true;
}

void AudioQueue::SetPaused( bool pause )
{
	dbAssert( m_deviceId > 0 );
	if ( m_paused != pause )
	{
		std::unique_lock lock{ m_queueMutex };
		ClearInternal();
		m_paused = pause;
	}
}

template <typename DestType>
inline void AudioQueue::ReadSamples( DestType* samples, size_t count )
{
	std::unique_lock lock{ m_queueMutex };

	if ( m_paused )
	{
		std::fill_n( samples, count, DestType( 0 ) );
		return;
	}

	const size_t available = std::min( count, m_size );
	PopSamples<DestType>( samples, available );

	const size_t remaining = count - available;
	if ( remaining > 0 )
	{
		// dbLogWarning( "AudioQueue::ReadSamples -- Starving audio device [%u]", remaining );
		// std::fill_n( samples + available, remaining, DestType( 0 ) );
		// std::fill_n( samples + available, remaining, DestType( *(m_queue.get() + m_last-1) ) );
		

		// repeat old samples to fill audio gap. Hopefully this sounds better than filling with 0s
		UnpopSamples( remaining );
		PopSamples( samples + available, remaining );
	}
}

void AudioQueue::StaticFillAudioDeviceBuffer( void* userData, uint8_t* buffer, int length )
{
	reinterpret_cast<AudioQueue*>( userData )->FillAudioDeviceBuffer( buffer, length );
}

void AudioQueue::FillAudioDeviceBuffer( uint8_t* buffer, int bufferLength )
{
	switch ( m_settings.format )
	{
		case AUDIO_S16:
			ReadSamples( reinterpret_cast<int16_t*>( buffer ), static_cast<size_t>( bufferLength ) / sizeof( int16_t ) );
			break;

		default:
			dbBreak();
			break;
	}
}

void AudioQueue::PushSamples( const int16_t* samples, size_t count )
{
	std::unique_lock lock{ m_queueMutex };

	if ( m_paused )
		return;

	const size_t capacity = m_bufferSize - m_size;
	if ( capacity < count )
	{
		dbLogWarning( "AudioQueue::PushSamples -- Exceeding queue capacity" );
		if ( capacity == 0 )
			return;

		count = capacity;
	}

	dbAssert( m_size + count <= m_bufferSize );

	const size_t seg1Count = std::min( count, m_bufferSize - m_last );
	const size_t seg2Count = count - seg1Count;

	std::copy_n( samples, seg1Count, m_queue.get() + m_last );
	std::copy_n( samples + seg1Count, seg2Count, m_queue.get() );

	m_size += count;
	m_last = ( m_last + count ) % m_bufferSize;

	CheckFullBuffer();
}

void AudioQueue::PushSilenceFrames( size_t count )
{
	std::unique_lock lock{ m_queueMutex };

	if ( m_paused )
		return;

	count *= m_settings.channels;

	const size_t capacity = m_bufferSize - m_size;
	if ( capacity < count )
	{
		dbLogWarning( "AudioQueue::PushSilenceFrames -- Exceeding queue capacity" );
		count = capacity;
	}

	const size_t seg1Count = std::min( count, m_bufferSize - m_last );
	const size_t seg2Count = count - seg1Count;

	std::fill_n( m_queue.get() + m_last, seg1Count, SampleType( 0 ) );
	std::fill_n( m_queue.get(), seg2Count, SampleType( 0 ) );

	m_size += count;
	m_last = ( m_last + count ) % m_bufferSize;

	CheckFullBuffer();
}

void AudioQueue::IgnoreSamples( size_t count )
{
	std::unique_lock lock{ m_queueMutex };

	count = std::min( count, m_size );
	m_size -= count;
	m_first = ( m_first + count ) % m_bufferSize;
}

void AudioQueue::ClearInternal()
{
	m_size = 0;
	m_first = 0;
	m_last = 0;
	m_waitForFullBuffer = true;
	SDL_PauseAudioDevice( m_deviceId, true );
}

void AudioQueue::CheckFullBuffer()
{
	if ( m_waitForFullBuffer && m_size >= static_cast<size_t>( m_settings.samples * m_settings.channels ) )
	{
		m_waitForFullBuffer = false;

		if ( !m_paused )
			SDL_PauseAudioDevice( m_deviceId, false );
	}
}

template <typename T>
void AudioQueue::PopSamples( T* dest, size_t count )
{
	dbExpects( m_size >= count );

	const size_t seg1Size = std::min( count, m_bufferSize - m_first );
	const size_t seg2Size = count - seg1Size;

	if constexpr ( std::is_same_v<T, int16_t> )
	{
		std::copy_n( m_queue.get() + m_first, seg1Size, dest );
		std::copy_n( m_queue.get(), seg2Size, dest + seg1Size );
	}
	else
	{
		dbBreak(); // TODO
	}

	m_size -= count;
	m_first = ( m_first + count ) % m_bufferSize;
}

void AudioQueue::UnpopSamples( size_t count )
{
	dbExpects( m_size + count <= m_bufferSize );

	m_size += count;
	m_first = ( m_first + m_bufferSize - count ) % m_bufferSize;
}