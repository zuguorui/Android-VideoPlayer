#!/bin/bash

# To link extern lib, you may need to try for some times to find out how FFmpeg located it. For example, if you 
# want to enable fdk-aac, you should put header files in a folder named "fdk-aac", and pass a c flag as "-IparentFolder",
# the "parentFolder" containes the "fdk-aac" folder, so the FFmpeg can find the header files correctly. But for lame and x264,
# you can put header files wherever you want. If some errors occured during compiling, please see "ffbuild/config.log" for
# details.

# WARNING: Please be sure all configure options are available, or the compile will not success. FFmpeg won't ignore wrong or unknown
# options. And don't put comment in a whole line sentence, it can break the sentence and the following configure options will be ignored.
NDK=/Users/zu/AndroidSDK/ndk/20.1.5948944
HOST_TAG=darwin-x86_64
TOOLCHAIN=$NDK/toolchains/llvm/prebuilt/$HOST_TAG

ANDROID_LIB_PATH="$(pwd)/thin/android"

SYSROOT=$NDK/toolchains/llvm/prebuilt/$HOST_TAG/sysroot

API=26

function build_android_arm
{
echo "Compiling FFmpeg for $CPU"
./configure \
--disable-stripping \
--enable-gpl \
--enable-version3 \
--enable-nonfree \
--disable-static \
--enable-shared \
--enable-small \
--disable-runtime-cpudetect \
--disable-programs \
--disable-doc \
--disable-asm \
--enable-dct \
--enable-dwt \
--disable-avdevice \
--disable-devices \
--disable-encoders \
--enable-encoder=pcm_s16le \
--enable-encoder=aac \
--enable-encoder=mp3 \
--disable-decoders \
--enable-decoder=aac \
--enable-decoder=mp3 \
--enable-decoder=ac3 \
--enable-decoder=flac \
--enable-decoder=pcm_s16le \
--enable-decoder=flv \
--enable-decoder=h264 \
--enable-decoder=mpeg4 \
--enable-decoder=mpegvideo \
--disable-parsers \
--enable-parser=aac \
--enable-parser=h264 \
--enable-parser=mpegaudio \
--enable-parser=mpegvideo \
--enable-parser=mpeg4video \
--enable-parser=flac \
--disable-muxers \
--enable-muxer=ac3 \
--enable-muxer=adts \
--enable-muxer=avi \
--enable-muxer=flv \
--enable-muxer=mp3 \
--enable-muxer=mp4 \
--enable-muxer=pcm_s16le \
--enable-muxer=h264 \
--disable-demuxers \
--enable-demuxer=matroska \
--enable-demuxer=aac \
--enable-demuxer=ac3 \
--enable-demuxer=flv \
--enable-demuxer=h264 \
--enable-demuxer=mp3 \
--enable-demuxer=wav \
--enable-demuxer=flac \
--enable-demuxer=mpegvideo \
--enable-demuxer=pcm_s16le \
--enable-demuxer=webm_dash_manifest \
--disable-hwaccels \
--disable-bsfs \
--enable-bsf=aac_adtstoasc \
--enable-bsf=h264_mp4toannexb \
--disable-protocols \
--enable-protocol=file \
--enable-protocol=http \
--enable-protocol=https \
--enable-protocol=tcp \
--enable-protocol=rtmp \
--disable-filters \
--disable-postproc \
--enable-libfdk-aac \
--enable-libx264 \
--enable-libmp3lame \
--prefix="$PREFIX" \
--cross-prefix="$CROSS_PREFIX" \
--target-os=android \
--arch="$ARCH" \
--cpu="$CPU" \
--cc="$CC" \
--cxx="$CXX" \
--enable-cross-compile \
--sysroot="$SYSROOT" \
--extra-cflags="-Os -fpic $OPTIMIZE_CFLAGS" \
--extra-ldflags="-Lexternal-lib/lame/android/$CPU/ -Lexternal-lib/fdk-aac/android/$CPU/ -Lexternal-lib/x264/android/$CPU/"
make clean
make -j8
make install
echo "The Compilation of FFmpeg for $CPU is completed"
}

#armv8-a
ARCH=arm64
CPU=armv8-a
CC=$TOOLCHAIN/bin/aarch64-linux-android$API-clang
CXX=$TOOLCHAIN/bin/aarch64-linux-android$API-clang++
CROSS_PREFIX=$TOOLCHAIN/bin/aarch64-linux-android-
PREFIX=$ANDROID_LIB_PATH/$CPU
OPTIMIZE_CFLAGS="-march=$CPU -Iexternal-lib/lame/include -Iexternal-lib/fdk-aac/include -Iexternal-lib/x264/include"
build_android_arm

#armv7-a
ARCH=arm
CPU=armv7-a
CC=$TOOLCHAIN/bin/armv7a-linux-androideabi$API-clang
CXX=$TOOLCHAIN/bin/armv7a-linux-androideabi$API-clang++
CROSS_PREFIX=$TOOLCHAIN/bin/arm-linux-androideabi-
PREFIX=$ANDROID_LIB_PATH/$CPU
OPTIMIZE_CFLAGS="-mfloat-abi=softfp -mfpu=vfp -marm -march=$CPU -Iexternal-lib/lame/include -Iexternal-lib/fdk-aac/include -Iexternal-lib/x264/include"
build_android_arm




