#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavutil/channel_layout.h>
    #include <libavutil/common.h>
    #include <libavutil/frame.h>
    #include <libavutil/samplefmt.h>
}

#include "audio_encoder_core.h"
#include "io_data.h"

static const AVCodec *codec = nullptr;
static AVCodecContext *codec_ctx = nullptr;
static AVFrame *frame = nullptr;
static AVPacket *pkt = nullptr;

static enum AVCodecID audio_codec_id;

int32_t init_audio_encoder(const char *codec_name) {
    if(strcasecmp(codec_name, "MP3") == 0) {
        audio_codec_id = AV_CODEC_ID_MP3;
        std::cout << "Select codec id: MP3" << std::endl;
    }
    else if(strcasecmp(codec_name, "AAC") == 0) {
        audio_codec_id = AV_CODEC_ID_AAC;
        std::cout << "Select codec id: AAC" << std::endl;
    }
    else {
        std::cerr << "Error: invalid audio format." << std::endl;
        return -1;
    }

    codec = avcodec_find_encoder(audio_codec_id);
    if(!codec) {
        std::cerr << "Error: could not find codec." << std::endl;
        return -1;
    }

    codec_ctx = avcodec_alloc_context3(codec);
    if(!codec_ctx) {
        std::cerr << "Error: could not alloc codec." << std::endl;
        return -1;
    }

    // 设置音频编码器的参数
    codec_ctx->bit_rate = 128000;                     // 设置输出码率为128Kbps
    codec_ctx->sample_fmt = AV_SAMPLE_FMT_FLTP;       // 音频采样格式为fltp，单精度浮点数，planar
    codec_ctx->sample_rate = 48000;                   // 音频采样率为44.1kHz
    codec_ctx->channel_layout = AV_CH_LAYOUT_STEREO;  //声道布局为立体声
    codec_ctx->channels = av_get_channel_layout_nb_channels(codec_ctx->channel_layout);      // 声道数为双声道

    int32_t result = avcodec_open2(codec_ctx, codec, nullptr);
    if(result < 0) {
        std::cerr << "Error: could not open codec." << std::endl;
        return -1;
    }

    frame = av_frame_alloc();
    if(!frame) {
        std::cerr << "Error: could not alloc frame." << std::endl;
        return -1;
    }

    frame->nb_samples = codec_ctx->frame_size;
    frame->format = codec_ctx->sample_fmt;
    frame->channel_layout = codec_ctx->channel_layout;
    result = av_frame_get_buffer(frame, 0);
    if(result < 0) {
        std::cerr << "Error: AVFrame could not get buffer." << std::endl;
        return -1;
    }

    pkt = av_packet_alloc();
    if(!pkt) {
        std::cerr << "Error: could not alloc packet." << std::endl;
        return -1;
    }

    return 0;
}

// 写入ADTS头
static void get_adts_header(AVCodecContext* ctx, uint8_t* adts_header, int aac_length)
{
    uint8_t freq_idx = 0;    //0: 96000 Hz  3: 48000 Hz 4: 44100 Hz
    switch (ctx->sample_rate) {
    case 96000: freq_idx = 0; break;
    case 88200: freq_idx = 1; break;
    case 64000: freq_idx = 2; break;
    case 48000: freq_idx = 3; break;
    case 44100: freq_idx = 4; break;
    case 32000: freq_idx = 5; break;
    case 24000: freq_idx = 6; break;
    case 22050: freq_idx = 7; break;
    case 16000: freq_idx = 8; break;
    case 12000: freq_idx = 9; break;
    case 11025: freq_idx = 10; break;
    case 8000: freq_idx = 11; break;
    case 7350: freq_idx = 12; break;
    default: freq_idx = 4; break;
    }
    uint8_t chanCfg = ctx->channels;
    uint32_t frame_length = aac_length + 7;
    adts_header[0] = 0xFF;
    adts_header[1] = 0xF1;
    adts_header[2] = ((ctx->profile) << 6) + (freq_idx << 2) + (chanCfg >> 2);
    adts_header[3] = (((chanCfg & 3) << 6) + (frame_length >> 11));
    adts_header[4] = ((frame_length & 0x7FF) >> 3);
    adts_header[5] = (((frame_length & 7) << 5) + 0x1F);
    adts_header[6] = 0xFC;
}

static int32_t encode_frame(bool flushing) {
    int32_t result = 0;
    result = avcodec_send_frame(codec_ctx, flushing ? nullptr : frame);
    if(result < 0) {
        std::cerr << "Error: avcodec_send_frame failed." << std::endl;
        return result;
    }

    while(result >= 0) {
        result = avcodec_receive_packet(codec_ctx, pkt);
        if(result == AVERROR(EAGAIN) || result == AVERROR_EOF) {
            return 1;
        }
        else if(result < 0) {
            std::cerr << "Error: avcodec_receive_packet failed." << std::endl;
            return result;
        }

        uint8_t aac_header[7];
        get_adts_header(codec_ctx, aac_header, pkt->size);

        write_header_to_file(aac_header, 7);
        write_pkt_to_file(pkt);
    }
    return 0;
}

int32_t audio_encoding() {
    int32_t result = 0;
    while(!end_of_input_file()) {
        result = read_pcm_to_frame(frame, codec_ctx);
        if(result < 0) {
            std::cerr << "Error: read_pcm_to_frame failed." << std::endl;
            return -1;
        }

        result = encode_frame(false);
        if(result < 0) {
            std::cerr << "Error: encode_frame failed." << std::endl;
            return result;
        }
    }

    result = encode_frame(true);
    if(result < 0) {
        std::cerr << "Error: flushing failed." << std::endl;
        return result;
    }

    return 0;
}

void destroy_audio_encoder() {
    av_frame_free(&frame);
    av_packet_free(&pkt);
    avcodec_free_context(&codec_ctx);
}