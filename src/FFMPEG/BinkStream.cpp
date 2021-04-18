#include "BinkStream.h"
#include "Audio/Audio.h"
#include <iostream>
#include <cassert>

using namespace Sourcehold::FFMPEG;
using namespace Sourcehold::OpenAL;

BinkStream::BinkStream(const std::string& path, bool looping)
    : looping_(looping) {
  auto input_format = av_find_input_format("bink");
  if (!input_format) {
    throw std::runtime_error("failed to find input_format");
  }
  decoder_ = std::make_unique<DecoderAll>(path, input_format);
  audio_source_ = std::make_unique<AudioSource<17,float>>(
      decoder_->audio_frame_->channels, AL_FORMAT_STEREO16,
      decoder_->audio_frame_->sample_rate);

  frame_buffer_ = std::vector<uint32_t>(decoder_->video_context_->width *  //
                                        decoder_->video_context_->height);

  buffer_dim = {decoder_->video_context_->width,
                decoder_->video_context_->height};
  sws_context_ = SWSContextUQ(sws_getContext(
      decoder_->video_context_->width, decoder_->video_context_->height,
      decoder_->video_context_->pix_fmt, decoder_->video_context_->width,
      decoder_->video_context_->height, AV_PIX_FMT_RGB32, SWS_BILINEAR, nullptr,
      nullptr, nullptr));

  if (!sws_context_) {
    throw std::runtime_error("failed to get sws context");
  }

  // set bink extra data
  uint8_t bink_extradata[4] = {0};
  decoder_->video_context_->extradata = bink_extradata;
  decoder_->video_context_->extradata_size = sizeof(bink_extradata);

  running_ = true;
  packet_finished_ = true;
}

void BinkStream::Update() {
  if (!running_) {
    return;
  }

  av_init_packet(&packet_);

  if (!Finished()) {
    if (packet_.stream_index == decoder_->video_stream_) {
      UpdateVideo();
      OnUpdateVideo_();
    }
    else if (packet_.stream_index == decoder_->audio_stream_) {
      UpdateAudio();
    }
  }
}

bool BinkStream::Finished() {
  if (av_read_frame(decoder_->av_context_.get(), &packet_) < 0) {
    if (looping_) {
      av_seek_frame(decoder_->av_context_.get(), -1, 0, 0);
      if (av_read_frame(decoder_->av_context_.get(), &packet_) < 0) {
        return true;
      }
    }
    else {
      running_ = false;
      return true;
    }
  }
  return false;
}

void BinkStream::UpdateVideo() {
  if (!VideoFinished()) {
    ProcessVideoFrame();
  }
}

bool BinkStream::VideoFinished() {
  if (packet_finished_) {
    if (avcodec_send_packet(decoder_->video_context_.get(), &packet_) < 0) {
      return true;
    }
  }
  return false;
}

void BinkStream::ProcessVideoFrame() {
  auto ret = avcodec_receive_frame(decoder_->video_context_.get(),
                                   decoder_->video_frame_.get());
  if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
    return;
  }

  uint8_t* slices[3] = {(uint8_t*)frame_buffer_.data(), 0, 0};
  int strides[3] = {decoder_->video_context_->width * 4, 0, 0};

  sws_scale(sws_context_.get(), decoder_->video_frame_->data,
            decoder_->video_frame_->linesize, 0,
            decoder_->video_context_->height, slices, strides);
}

bool BinkStream::AudioState() {
  av_frame_unref(decoder_->audio_frame_.get());
  int ret = 0;
  if (packet_finished_) {
    ret = avcodec_send_packet(decoder_->audio_context_.get(), &packet_);
    if (ret < 0) {
        return false;
    }
  }
  return true;
}

auto BinkStream::UpdateAudio() -> void {
  if(AudioState()){
    auto state = avcodec_receive_frame(decoder_->audio_context_.get(),
                                      decoder_->audio_frame_.get());
    while (state >= 0) {
      state = avcodec_receive_frame(decoder_->audio_context_.get(),
                                  decoder_->audio_frame_.get());

      if (state == AVERROR(EAGAIN) || state == AVERROR_EOF) {
        break;
        }
      else if (state < 0) {
        throw std::runtime_error("failed to recieve frame from audio decoder: " +
                               decoder_->GetAvError(state));
      }
      audio_source_->Update(GetAudioData());
    }
    audio_source_->Play();
  }
}

auto BinkStream::GetAudioData() -> std::vector<float> {
  std::vector<float> result;
  auto size = decoder_->GetAudioFrameSize();
  result.reserve(size);

  std::transform(&decoder_->audio_frame_->data[0][0],
                 &decoder_->audio_frame_->data[0][size],
                 std::back_inserter(result), [](auto val) {
                   return val;
                 });
  return result;
}