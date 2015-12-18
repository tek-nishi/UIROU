
#pragma once

//
// OpenAL管理
// 

#if defined (_MSC_VER)
#pragma comment (lib, "openal32.lib")
#endif

#if defined (__APPLE__)
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#else
#include <AL/al.h>
#include <AL/alc.h>
#endif
#include <boost/noncopyable.hpp>
#include "co_vector.hpp"


namespace ngs {

class Audio : private boost::noncopyable {
	ALCdevice* const device_;
	ALCcontext* const context_;

public:
	Audio() :
		device_(alcOpenDevice(0)),
		context_(alcCreateContext(device_, 0))
	{
    DOUT << "Audio()" << std::endl;
    
		alcMakeContextCurrent(context_);
    // TODO:エラーは無視しても問題ないが、エラー内容を表示した方がよい
	}

	~Audio() {
    DOUT << "~Audio()" << std::endl;
    
		// TIPS:カレントコンテキストをNULLにしてから解放する
		alcMakeContextCurrent(0);

		alcDestroyContext(context_);
		alcCloseDevice(device_);
	}
    

  void suspend() const {
    alcMakeContextCurrent(0);
    alcSuspendContext(context_);
  }

  void process() const {
    alcMakeContextCurrent(context_);
    alcProcessContext(context_);
  }


  static void listenerPosition(const Vec3f&pos) {
    ALfloat listener_pos[] = { pos.x(), pos.y(), pos.z() };
    alListenerfv(AL_POSITION, listener_pos);
  }
  
  
  // 波形バッファを扱うクラス
  class Buffer : private boost::noncopyable {
    ALuint id_;

  public:
    Buffer() {
       alGenBuffers(1, &id_);
    }

    ~Buffer() {
      DOUT << "~Buffer()" << std::endl;
      alDeleteBuffers(1, &id_);
    }

    void bind(const bool stereo, const void* data, const u_int size, const u_int rate) const {
      alBufferData(id_, stereo ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16, data, size, rate);
    }

    ALuint id() const { return id_; }
  };

  // 再生バッファを扱うクラス
  class Source : private boost::noncopyable {
    ALuint id_;

  public:
    Source() {
      alGenSources(1, &id_);
    }

    ~Source() {
      DOUT << "~Source()" << std::endl;
      alDeleteSources(1, &id_);
    }

    void play(const Buffer& buffer, const float gain, const bool loop) const {
      alSourceStop(id_);
      alSourcei(id_, AL_BUFFER, buffer.id());
      alSourcef(id_, AL_GAIN, gain);
      alSourcei(id_, AL_LOOPING, loop);
      alSourcePlay(id_);
    }

    void play() const {
      alSourcePlay(id_);
    }

    void pause() const {
      alSourceStop(id_);
    }

    void  stop() const {
      alSourceStop(id_);
      alSourcei(id_, AL_BUFFER, 0);
      // TIPS:再生を止めたらバッファの割当を解除しておく
    }
    
    void gain(const float gain) const {
      alSourcef(id_, AL_GAIN, gain);
    }

    void position(const Vec3f& pos) {
      ALfloat source_pos[] = { pos.x(), pos.y(), pos.z() };
      alSourcefv(id_, AL_POSITION, source_pos);
    }

    
    void queueBuffer(const Buffer& buffer) const {
      ALuint buffers = buffer.id();
      alSourceQueueBuffers(id_, 1, &buffers);
    }

    void unqueueBuffer() const {
      ALuint buffer;
      alSourceUnqueueBuffers(id_, 1, &buffer);
    }

    ALuint id() const { return id_; }

    int processed() const {
      int processed;
      alGetSourcei(id_, AL_BUFFERS_PROCESSED, &processed);
      
      return processed;
    }

    bool isPlaying() const {
      ALenum state;
      alGetSourcei(id_, AL_SOURCE_STATE, &state);
      
      return state == AL_PLAYING;
    }
    
  };
  
};

}
