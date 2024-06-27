#! /bin/bash
rm -rf build
mkdir build
cd build

cmake ..
make


# mp4示例的aac流是5.1，6声道，有点问题，这里强行使用stereo，双声道
./demuxer ../SampleVideo_1280x720_1mb.mp4 SampleVideo_1280x720_1mb.yuv SampleVideo_1280x720_1mb.pcm
#./video_decoder ../SampleVideo_1280x720_1mb.h264 SampleVideo_1280x720_1mb.yuv
./audio_decoder ../SampleVideo_1280x720_1mb.aac SampleVideo_1280x720_1mb.pcm AAC

./video_encoder SampleVideo_1280x720_1mb.yuv SampleVideo_1280x720_1mb.h264 libx264
./audio_encoder SampleVideo_1280x720_1mb.pcm SampleVideo_1280x720_1mb.aac AAC
./muxer SampleVideo_1280x720_1mb.h264 ../SampleVideo_1280x720_1mb.aac SampleVideo_1280x720_1mb.mp4

#./demuxer ../SampleVideo_1280x720_1mb.mp4 SampleVideo_1280x720_1mb.yuv SampleVideo_1280x720_1mb.pcm

#./video_decoder ../SampleVideo_1280x720_1mb.h264 SampleVideo_1280x720_1mb.yuv
#./video_encoder SampleVideo_1280x720_1mb.yuv SampleVideo_1280x720_1mb.h264 libx264
#./audio_decoder ../SampleVideo_1280x720_1mb.aac SampleVideo_1280x720_1mb.pcm AAC
#./audio_encoder SampleVideo_1280x720_1mb.pcm SampleVideo_1280x720_1mb.aac AAC
# valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all --undef-value-errors=no --log-file=log ./demo