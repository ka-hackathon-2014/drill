#include <AL/al.h>    // for ALuint, AL_FALSE, AL_NONE, etc
#include <AL/alc.h>   // for ALC_INVALID_CONTEXT, etc
#include <AL/alure.h> // for alureStopSource, etc
#include <cassert>
#include <cstddef>
#include <functional>
#include <stdexcept>
#include <string>

namespace Audioxx {

class Utils final {
public:
  /**
   * 1) Identify the error code.
   * 2) Return the error as a string.
   */
  static std::string getALCErrorString(int err)
  {
    switch (err) {
      case AL_NO_ERROR:
        return "AL_NO_ERROR";
      case ALC_INVALID_DEVICE:
        return "ALC_INVALID_DEVICE";
      case ALC_INVALID_CONTEXT:
        return "ALC_INVALID_CONTEXT";
      case ALC_INVALID_ENUM:
        return "ALC_INVALID_ENUM";
      case ALC_INVALID_VALUE:
        return "ALC_INVALID_VALUE";
      case ALC_OUT_OF_MEMORY:
        return "ALC_OUT_OF_MEMORY";
      default:
        return "no such error code";
    }
  }
};

/**
 * Manages the mapping between a file and a memory buffer.
 *
 * Note: The whole file gets mapped into memory.
 */
class Buffer final {

public:
  Buffer(const std::string& filename) : buffer(alureCreateBufferFromFile(filename.c_str()))
  {

    if (buffer == AL_NONE)
      throw std::runtime_error("Error: Unable to create buffer: " + std::string(alureGetErrorString()));
  }

  Buffer(const Buffer&) = delete;

  Buffer(Buffer&& o) : buffer(o.buffer)
  {
    o.buffer = AL_NONE;
  }

  ~Buffer()
  {
    if (buffer != AL_NONE)
      alDeleteBuffers(1, &buffer);
  }

  ALuint get() const noexcept
  {
    return buffer;
  }


private:
  ALuint buffer;
};

/**
 * Manages the device and context creation, essential for each application.
 */
class Device final {

public:
  Device()
  {
    if (alureInitDevice(nullptr, nullptr) == AL_FALSE)
      throw std::runtime_error("Error: Unable to create device: " + std::string(alureGetErrorString()));
  }

  ~Device()
  {
    alureShutdownDevice();
  }
};

/**
 * Manages the link between a source and a buffer.
 */
class Link final {

public:
  Link(ALuint source_, ALuint buffer_) : source(source_), buffer(buffer_)
  {

    assert(source != 0 && "Error: Source not valid");
    assert(buffer != 0 && "Error: Buffer not valid");

    alSourcei(source, AL_BUFFER, buffer);

    auto e = alGetError();
    if (e)
      throw std::runtime_error("Unable to create Link: " + Utils::getALCErrorString(e));
  }

  ~Link()
  {
    alSourcei(source, AL_BUFFER, AL_NONE);
    auto e = alGetError();
    if (e)
      throw std::runtime_error("Unable to create Link: " + Utils::getALCErrorString(e));
  }


private:
  const ALuint source;
  const ALuint buffer;
};

/**
 * Manages a source, on which various audio functions are invoked.
 */
class Source final {

public:
  Source() : isdone(false)
  {
    alGenSources(1, &source);

    if (alGetError() != AL_NO_ERROR)
      throw std::runtime_error("Error: Unable to create source: " + std::string(alureGetErrorString()));
  }

  ~Source()
  {
    alureStopSource(source, AL_FALSE);
    alDeleteSources(1, &source);
  }

  ALuint get() const noexcept
  {
    return source;
  }


  void play(const Buffer& buffer, std::function<bool()> end)
  {
    Link link(this->get(), buffer.get());

    if (alurePlaySource(source, callback_wrapper, this) == AL_FALSE)
      throw std::runtime_error("Error: Unable to play buffer: " + std::string(alureGetErrorString()));

    eventloop(end);
    alureStopSource(source, AL_FALSE);
  }


private:
  ALuint source;

  volatile bool isdone;

  // XXX: static callback wrapper required, pass pointer as userdata to modify object's isdone state
  static void callback_wrapper(void* source, ALuint) noexcept
  {
    static_cast<Source*>(source)->isdone = true;
  }

  void eventloop(std::function<bool()> end, const float interval = 0.125f) const noexcept
  {
    while (not isdone and not end()) {
      alureSleep(interval);
      alureUpdate();
    }
  }
};

/**
 * Offers high level audio functionality.
 */
class Player final {

public:
  explicit Player() try : device()
  {
  }
  catch (const std::runtime_error&)
  {
    // XXX: simply rethrow for now, in order to preserve error details
    throw;
  }

  void play(const Buffer& buffer, std::function<bool()> end = []()->bool { return false; })
  {
    Source source;
    source.play(buffer, end);
  }


private:
  Device device;
};
}
