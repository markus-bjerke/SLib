#include "../../../inc/slib/media/opensl_es.h"

#ifdef SLIB_AUDIO_SUPPORT_OPENSL_ES

#include "../../../inc/slib/core/log.h"
#include "../../../inc/slib/core/pointer.h"

#if defined(SLIB_PLATFORM_IS_ANDROID)
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <SLES/OpenSLES_AndroidConfiguration.h>
#endif

SLIB_MEDIA_NAMESPACE_BEGIN
class _OpenSLES_AudioPlayerImpl : public AudioPlayer
{
private:
	_OpenSLES_AudioPlayerImpl() {}

public:
	SLObjectItf m_engineObject;
	SLEngineItf m_engineInterface;
	SLObjectItf m_mixerObject;

	~_OpenSLES_AudioPlayerImpl()
	{
		(*m_mixerObject)->Destroy(m_mixerObject);
		(*m_engineObject)->Destroy(m_engineObject);
	}

	static void logError(String text)
	{
		SLIB_LOG_ERROR("OpenSL_ES", text);
	}

	static Ref<_OpenSLES_AudioPlayerImpl> create(const AudioPlayerParam& param)
	{
		Ref<_OpenSLES_AudioPlayerImpl> ret;

		SLObjectItf engineObject;
		SLEngineItf engineInterface;
		SLObjectItf mixerObject;

		SLEngineOption options[] = {
			{SL_ENGINEOPTION_THREADSAFE, 1},
		};
		if (slCreateEngine(&engineObject, 1, options, 0, sl_null, sl_null) == SL_RESULT_SUCCESS) {
			if ((*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE) == SL_RESULT_SUCCESS) {
				if ((*engineObject)->GetInterface(engineObject, SL_IID_ENGINE,  &engineInterface) == SL_RESULT_SUCCESS) {
					if ((*engineInterface)->CreateOutputMix(engineInterface, &mixerObject, 0, sl_null, sl_null) == SL_RESULT_SUCCESS) {
						if ((*mixerObject)->Realize(mixerObject, SL_BOOLEAN_FALSE) == SL_RESULT_SUCCESS) {
							ret = new _OpenSLES_AudioPlayerImpl();
							if (ret.isNotNull()) {
								ret->m_engineObject = engineObject;
								ret->m_engineInterface = engineInterface;
								ret->m_mixerObject = mixerObject;
								return ret;
							}
						} else {
							logError("Failed to realize output mixer");
						}
						(*mixerObject)->Destroy(mixerObject);
					} else {
						logError("Failed to create output mixer");
					}
				} else {
					logError("Failed to get engine interface");
				}
			} else {
				logError("Failed to realize engine");
			}
			(*engineObject)->Destroy(engineObject);
		} else {
			logError("Failed to create engine");
		}
		return ret;
	}

	Ref<AudioPlayerBuffer> createBuffer(const AudioPlayerBufferParam& param);
};

class _OpenSLES_AudioPlayerBufferImpl : public AudioPlayerBuffer
{
private:
	_OpenSLES_AudioPlayerBufferImpl()
	{
		m_flagOpened = sl_true;
		m_flagRunning = sl_false;
		m_indexBuffer = 0;
	}

public:
	Ref<_OpenSLES_AudioPlayerImpl> m_engine;
	SLObjectItf m_playerObject;
	SLPlayItf m_playerInterface;
	SLAndroidSimpleBufferQueueItf m_bufferQueue;

	sl_bool m_flagOpened;
	sl_bool m_flagRunning;
	
	sl_int16* m_bufFrame;
	sl_uint32 m_indexBuffer;
	sl_uint32 m_nSamplesFrame;

	~_OpenSLES_AudioPlayerBufferImpl()
	{
		release();
	}

	static void logError(String text)
	{
		SLIB_LOG_ERROR("OpenSL_ES_Buffer", text);
	}

	static Ref<_OpenSLES_AudioPlayerBufferImpl> create(Ref<_OpenSLES_AudioPlayerImpl> engine, const AudioPlayerBufferParam& param)
	{
		Ref<_OpenSLES_AudioPlayerBufferImpl> ret;
		SLEngineItf engineInterface = engine->m_engineInterface;

		SLDataLocator_AndroidSimpleBufferQueue androidSBQ = {
			SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2
		};
		SLDataFormat_PCM configuration;
		{
			configuration.formatType = SL_DATAFORMAT_PCM;
			configuration.numChannels = param.channelsCount;
			configuration.samplesPerSec = param.samplesPerSecond * 1000;
			configuration.bitsPerSample = SL_PCMSAMPLEFORMAT_FIXED_16;
			configuration.containerSize = SL_PCMSAMPLEFORMAT_FIXED_16;
			if (configuration.numChannels == 2) {
				configuration.channelMask = SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;
			} else {
				configuration.channelMask = SL_SPEAKER_FRONT_CENTER;
			}
			configuration.endianness = SL_BYTEORDER_LITTLEENDIAN;
		}
		SLDataSource slDataSource = { &androidSBQ, &configuration };

		SLDataLocator_OutputMix outputMix;
		outputMix.locatorType = SL_DATALOCATOR_OUTPUTMIX;
		outputMix.outputMix = engine->m_mixerObject;
		SLDataSink slDataSink = { &outputMix, sl_null };

		SLObjectItf playerObject;
		SLPlayItf playerInterface;
		SLAndroidSimpleBufferQueueItf bufferQueue;

		SLInterfaceID ids[3] = { SL_IID_BUFFERQUEUE, SL_IID_VOLUME, SL_IID_ANDROIDCONFIGURATION };
		SLboolean req[3] = { SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE };
		if ((*engineInterface)->CreateAudioPlayer(engineInterface, &playerObject, &slDataSource, &slDataSink, 3, ids, req) == SL_RESULT_SUCCESS) {
			if ((*playerObject)->Realize(playerObject, SL_BOOLEAN_FALSE) == SL_RESULT_SUCCESS) {
				if ((*playerObject)->GetInterface(playerObject, SL_IID_PLAY, &playerInterface) == SL_RESULT_SUCCESS) {
					if ((*playerObject)->GetInterface(playerObject, SL_IID_BUFFERQUEUE, &bufferQueue) == SL_RESULT_SUCCESS) {
						ret = new _OpenSLES_AudioPlayerBufferImpl();
						if (ret.isNotNull()) {
							ret->m_engine = engine;
							ret->m_playerObject = playerObject;
							ret->m_playerInterface = playerInterface;
							ret->m_bufferQueue = bufferQueue;
							ret->m_nSamplesFrame = param.samplesPerSecond * param.frameLengthInMilliseconds / 1000 * param.channelsCount;
							ret->m_queue.setQueueSize(param.samplesPerSecond * param.bufferLengthInMilliseconds / 1000 * param.channelsCount);
							ret->m_bufFrame = new sl_int16[ret->m_nSamplesFrame * 2];
							ret->setListener(param.listener);
							if (ret->m_bufFrame) {
								if ((*bufferQueue)->RegisterCallback(bufferQueue, _OpenSLES_AudioPlayerBufferImpl::callback, ret.get()) == SL_RESULT_SUCCESS) {
									if (param.flagAutoStart) {
										ret->start();
									}
									return ret;
								} else {
									logError("Failed to register callback");
								}
							}
							return Ref<_OpenSLES_AudioPlayerBufferImpl>::null();
						}
					} else {
						logError("Failed to get buffer queue");
					}
				} else {
					logError("Failed to get player interface");
				}
			} else {
				logError("Failed to realize player object");
			}
			(*playerObject)->Destroy(playerObject);
		} else {
			logError("Failed to create player object");
		}
		return ret;
	}

	void release()
	{
		MutexLocker lock(getLocker());
		if (!m_flagOpened) {
			return;
		}
		stop();
		m_flagOpened = sl_false;
		(*m_playerObject)->Destroy(m_playerObject);
		if (m_bufFrame) {
			delete[] m_bufFrame;
		}
	}

	sl_bool isOpened()
	{
		return m_flagOpened;
	}

	void start()
	{
		MutexLocker lock(getLocker());
		if (!m_flagOpened) {
			return;
		}
		if (m_flagRunning) {
			return;
		}
		if (enqueue()) {
			if ((*m_playerInterface)->SetPlayState(m_playerInterface, SL_PLAYSTATE_PLAYING) != SL_RESULT_SUCCESS) {
				logError("Failed to play buffer");
				return;
			}
		}
		m_flagRunning = sl_true;
	}

	void stop()
	{
		MutexLocker lock(getLocker());
		if (!m_flagOpened) {
			return;
		}
		if (!m_flagRunning) {
			return;
		}
		m_flagRunning = sl_false;
		if ((*m_bufferQueue)->Clear(m_bufferQueue) != SL_RESULT_SUCCESS) {
			logError("Failed to clear buffer queue");
			return;
		}
		(*m_playerInterface)->SetPlayState(m_playerInterface, SL_PLAYSTATE_STOPPED);
	}

	sl_bool isRunning()
	{
		return m_flagRunning;
	}

	sl_bool enqueue()
	{
		m_indexBuffer = (m_indexBuffer + 1) % 2;
		sl_int16* s = m_bufFrame + m_indexBuffer * m_nSamplesFrame;
		_processFrame_S16(s, m_nSamplesFrame);
		if ((*m_bufferQueue)->Enqueue(m_bufferQueue, s, m_nSamplesFrame * 2) == SL_RESULT_SUCCESS) {
			return sl_true;
		} else {
			logError("Failed to enqueue buffer");
		}
		return sl_false;
	}

	void onFrame()
	{
		enqueue();
	}

	static void	callback(SLAndroidSimpleBufferQueueItf bufferQueue, void* p_context)
	{
		_OpenSLES_AudioPlayerBufferImpl* object = (_OpenSLES_AudioPlayerBufferImpl*)p_context;
		object->onFrame();
	}
};

Ref<AudioPlayerBuffer> _OpenSLES_AudioPlayerImpl::createBuffer(const AudioPlayerBufferParam& param)
{
	return Ref<AudioPlayerBuffer>::from(_OpenSLES_AudioPlayerBufferImpl::create(this, param));
}

Ref<AudioPlayer> OpenSL_ES::createPlayer(const AudioPlayerParam& param)
{
	return Ref<AudioPlayer>::from(_OpenSLES_AudioPlayerImpl::create(param));
}
SLIB_MEDIA_NAMESPACE_END

#else

SLIB_MEDIA_NAMESPACE_BEGIN
Ref<AudioPlayer> OpenSL_ES::createPlayer(const AudioPlayerParam& param)
{
	return Ref<AudioPlayer>::null();
}
SLIB_MEDIA_NAMESPACE_END

#endif
