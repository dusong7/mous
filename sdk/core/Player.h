#pragma once

#include <inttypes.h>
#include <memory>
#include <string>
#include <vector>

#include <util/Plugin.h>

#include <util/AudioMode.h>
#include <util/ErrorCode.h>

#include <scx/Signal.h>

namespace mous {

enum class PlayerStatus : uint8_t {
  Closed,
  Playing,
  Paused,
  Stopped
};

class Player {
  class Impl;

 public:
  Player();
  ~Player();

 public:
  PlayerStatus Status() const;

  void LoadFormatProbePlugin(const std::shared_ptr<Plugin>& plugin);
  void LoadDecoderPlugin(const std::shared_ptr<Plugin>& plugin);
  void LoadOutputPlugin(const std::shared_ptr<Plugin>& plugin);
  void UnloadPlugin(const std::string& path);
  void UnloadPlugin();

  std::vector<std::string> SupportedSuffixes() const;

  int BufferCount() const;
  void SetBufferCount(int count);

  int Volume() const;
  void SetVolume(int level);

  ErrorCode Open(const std::string& path);
  void Close();
  std::string FileName() const;

  void Play();
  void Play(uint64_t msBegin, uint64_t msEnd);
  void Pause();
  void Resume();
  void SeekTime(uint64_t msPos);
  void SeekPercent(double percent);

  void PauseDecoder();
  void ResumeDecoder();

  int32_t BitRate() const;
  int32_t SamleRate() const;
  uint64_t Duration() const;
  uint64_t RangeBegin() const;
  uint64_t RangeEnd() const;
  uint64_t RangeDuration() const;
  uint64_t OffsetMs() const;
  uint64_t CurrentMs() const;
  enum AudioMode AudioMode() const;

 public:
  scx::Signal<void(void)>* SigFinished();

 private:
  std::unique_ptr<Impl> impl;
};

}  // namespace mous
