#include "AudioBufferDataSource.h"

#include <algorithm>
#include <utility>
#include <memory>

extern "C" {
#include <libavformat/avformat.h> // NOLINT: not a C system header.
}

#include "Library/Logger/Logger.h"

AudioBufferDataSource::AudioBufferDataSource(Blob buffer) : _ioContext(std::move(buffer)) {}

bool AudioBufferDataSource::Open() {
    if (bOpened) {
        return true;
    }

    pFormatContext = avformat_alloc_context();
    if (pFormatContext == nullptr) {
        return false;
    }

    // A fresh context for every playback, e.g. of looping music. Seeking the old one back to the start keeps demuxer
    // state such as a running checksum.
    _ioContext.reset(Blob::share(_ioContext.blob()));

    pFormatContext->pb = _ioContext.avioContext();

    // Open audio file
    if (avformat_open_input(&pFormatContext, _ioContext.blob().displayPath().c_str(), nullptr, nullptr) < 0) {
        MM_WARNING("ffmpeg: Unable to open input buffer");
        return false;
    }

    // Dump information about buffer onto standard error
    av_dump_format(pFormatContext, 0, nullptr, 0);

    return AudioBaseDataSource::Open();
}

PAudioDataSource CreateAudioBufferDataSource(Blob buffer) {
    return std::make_shared<AudioBufferDataSource>(std::move(buffer));
}
