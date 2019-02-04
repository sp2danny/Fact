
#include <vector>
#include <string>
#include <stdexcept>

using namespace std::literals;

#include "makemovie.h"

extern "C" {
	#include <libavutil/imgutils.h>
	#include <libavutil/opt.h>
	#include <libavcodec/avcodec.h>
	#include <libavformat/avformat.h>
	#include <libswresample/swresample.h>
	#include <libavformat/avio.h>
}

namespace mm
{

#define OUTPUT_CODEC AV_CODEC_ID_H264
//Input pix fmt is set to BGR24
#define OUTPUT_PIX_FMT AV_PIX_FMT_YUV420P

AVFormatContext *fmt_ctx;
AVCodecContext  *codec_ctx; //a shortcut to st->codec
AVStream        *st;
//AVFrame         *tmp_frame;
int              pts=0;

/*
Encoder::~Encoder(){
  int err;
  std::cout<<"cleaning Encoder"<<std::endl;
  //Write pending packets
  while((err = write((AVFrame*)NULL)) == 1){};
  if(err < 0 ){
    std::cout <<"error writing delayed frame"<<std::endl;
  }
  //Write file trailer before exit
  av_write_trailer(this->fmt_ctx);
  //close file
  avio_close(fmt_ctx->pb);
  avformat_free_context(this->fmt_ctx);
  //avcodec_free_context(&this->codec_ctx);

  av_freep(&this->tmp_frame->data[0]);
  av_frame_free(&this->tmp_frame);
}

//Return 1 if a packet was written. 0 if nothing was done.
// return error_code < 0 if there was an error.


int Encoder::write(AVFrame *frame){
  int err;
  int got_output = 1;
  AVPacket pkt = {0};
  av_init_packet(&pkt);

  //Set frame pts, monotonically increasing, starting from 0
  if(frame != NULL) frame->pts = pts++; //we use frame == NULL to write delayed packets in destructor
  err = avcodec_encode_video2(this->codec_ctx, &pkt, frame, &got_output);
  if (err < 0) {
    std::cout <<AVException(err,"encode frame").what()<<std::endl;
    return err;
  }
  if(got_output){
    av_packet_rescale_ts(&pkt, this->codec_ctx->time_base, this->st->time_base);
    pkt.stream_index = this->st->index;
    // write the frame 
    //printf("Write packet %03d of size : %05d\n",pkt.pts,pkt.size);
    //write_frame will take care of freeing the packet.
    err = av_interleaved_write_frame(this->fmt_ctx,&pkt);
    if(err < 0){
      std::cout <<AVException(err,"write frame").what()<<std::endl;
      return err;
    }
    return 1;
  }else{
    return 0;
  }
}
*/	
	
struct AVException : std::exception
{
	AVException() = default;
	AVException(int e, std::string m)
		: m_errno(e), m_message(m) {}

	int m_errno;
	std::string m_message;
	const char* what() const noexcept override { return (std::to_string(m_errno) + " ("s + m_message + ")"s).c_str(); }
};
	
void SetupMovie(int width,int height, std::string name)
{
	av_register_all();
	avcodec_register_all();

	int err;
	AVOutputFormat  *fmt;
	AVCodec         *codec;
	AVDictionary    *fmt_opts = nullptr;
	fmt_ctx = avformat_alloc_context();
	if (!fmt_ctx)
	{
		throw AVException(ENOMEM, "can not alloc av context");
	}
	//init encoding format
	fmt = av_guess_format("mp4", nullptr, nullptr);
	if (!fmt)
	{
		throw AVException(1, "could not guess format");
	}
	//std::cout <<fmt->long_name<<std::endl;
	//Set format header infos
	fmt_ctx->oformat = fmt;
	auto target = name.c_str();
	snprintf(fmt_ctx->filename, sizeof(fmt_ctx->filename), "%s", target);
	//Reference for AvFormatContext options : https://ffmpeg.org/doxygen/2.8/movenc_8c_source.html
	//Set format's privater options, to be passed to avformat_write_header()
	err = av_dict_set(&fmt_opts, "movflags", "faststart", 0);
	if (err < 0)
	{
		throw AVException(err, "av_dict_set movflags");
	}
	//default brand is "isom", which fails on some devices
	av_dict_set(&fmt_opts, "brand", "mp42", 0);
	if (err < 0)
	{
		throw AVException(err, "av_dict_set brand");
	}
	codec = avcodec_find_encoder(OUTPUT_CODEC);
	//codec = avcodec_find_encoder(fmt->video_codec);
	if (!codec)
	{
		throw AVException(1,"can't find encoder");
	}
	st = avformat_new_stream(fmt_ctx, codec);
	if (!st)
	{
		throw AVException(1,"can't create new stream");
	}
	//set stream time_base
	/* frames per second FIXME use input fps? */
	st->time_base = AVRational{1, 50};

	//Set codec_ctx to stream's codec structure
	codec_ctx = st->codec;
	/* put sample parameters */
	codec_ctx->sample_fmt = AV_SAMPLE_FMT_NONE;
	//sample_fmt = codec->sample_fmts ? codec->sample_fmts[0] : AV_SAMPLE_FMT_S16;
	codec_ctx->width = width;
	codec_ctx->height = height;
	codec_ctx->time_base = st->time_base;
	codec_ctx->pix_fmt = OUTPUT_PIX_FMT;
	// Apparently it's in the example in master but does not work in V11
	if (fmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
	  codec_ctx->flags |= CODEC_FLAG_GLOBAL_HEADER;
	/**/
	//H.264 specific options
	
	codec_ctx->gop_size = 25;
	codec_ctx->level = 31;
	err = av_opt_set(codec_ctx->priv_data, "crf", "12", 0);
	if (err < 0)
	{
		throw AVException(err, "av_opt_set crf");
	}
	err = av_opt_set(codec_ctx->priv_data, "profile", "main", 0);
	if (err < 0)
	{
		throw AVException(err, "av_opt_set profile");
	}
	err = av_opt_set(codec_ctx->priv_data, "preset", "slow", 0);
	if (err < 0)
	{
		throw AVException(err, "av_opt_set preset");
	}
	// disable b-pyramid. CLI options for this is "-b-pyramid 0"
	//Because Quicktime (ie. iOS) doesn't support this option
	err = av_opt_set(codec_ctx->priv_data, "b-pyramid", "0", 0);
	if (err < 0)
	{
		throw AVException(err, "av_opt_set b-pyramid");
	}
	//It's necessary to open stream codec to link it to "codec" (the encoder).
	err = avcodec_open2(codec_ctx, codec, NULL);
	if (err < 0)
	{
		throw AVException(err, "avcodec_open2");
	}

	//* dump av format informations
	av_dump_format(fmt_ctx, 0, target, 1);
	//*/
	err = avio_open(&fmt_ctx->pb, target, AVIO_FLAG_WRITE);
	if(err < 0)
	{
		throw AVException(err,"avio_open");
	}

	//Write file header if necessary
	err = avformat_write_header(fmt_ctx, &fmt_opts);
	if(err < 0)
	{
		throw AVException(err, "avformat_write_header");
	}

	/* Alloc tmp frame once and for all*/
	/*
	tmp_frame = av_frame_alloc();
	if(!tmp_frame)
	{
		throw AVException(ENOMEM, "alloc frame");
	}
	//Make sure the encoder doesn't keep ref to this frame as we'll modify it.
	av_frame_make_writable(tmp_frame);
	tmp_frame->format = codec_ctx->pix_fmt;
	tmp_frame->width  = codec_ctx->width;
	tmp_frame->height = codec_ctx->height;
	err = av_image_alloc(tmp_frame->data, tmp_frame->linesize, codec_ctx->width, codec_ctx->height, codec_ctx->pix_fmt, 32);
	if (err < 0)
	{
		throw AVException(ENOMEM, "can't alloc output frame buffer");
	}
    */
}

void AddFrame(const Image& img)
{
	AVFrame* frame = av_frame_alloc();
	if (!frame)
	{
		throw AVException(ENOMEM, "alloc frame");
	}

	av_frame_make_writable(frame);
	int w = img.Width();
	int h = img.Height();
	frame->width = w;
	frame->height = h;
    frame->format = AV_PIX_FMT_RGB24;
	
	//pic->img.i_stride[0] = width;

    auto ret = av_frame_get_buffer(frame, 32);
    if (ret < 0) {
		throw AVException(ret, "could not alloc the frame data");
    }
	
	frame->linesize[0] = w*3;

	int idx = 0;
	for(int y=0; y<h ; ++y)
	{
		for(int x=0; x<w; ++x)
		{
			auto pix = img.GetPixel(x,y);
			frame->data[0][idx++] = pix.r;
			frame->data[0][idx++] = pix.g;
			frame->data[0][idx++] = pix.b;
		}
	}

	int err;
	int got_output = 1;

	AVPacket pkt;
	pkt.data = nullptr;
    pkt.size = 0;
	av_init_packet(&pkt);
    //av_new_packet(&pkt, 10000);

	//Set frame pts, monotonically increasing, starting from 0
	if (frame) frame->pts = pts++; //we use frame == NULL to write delayed packets in destructor
	
	codec_ctx->height = h;
	codec_ctx->width = w;

	//err = avcodec_send_frame(codec_ctx, frame);
	err = avcodec_encode_video2 (codec_ctx, &pkt, frame, &got_output);
	
	
	//err = avcodec_send_frame((AVCodecContext*)0,frame);
	
	if (err < 0)
	{
		throw AVException(err, "encode frame");
	}
	if (got_output)
	{
		av_packet_rescale_ts(&pkt, codec_ctx->time_base, st->time_base);
		pkt.stream_index = st->index;
		// write the frame 
		//printf("Write packet %03d of size : %05d\n",pkt.pts,pkt.size);
		//write_frame will take care of freeing the packet.
		err = av_interleaved_write_frame(fmt_ctx, &pkt);
		if (err < 0)
		{
			throw AVException(err, "write frame");
		}
	} else {
		//throw AVException(1, "got no output");
	}
	
	av_frame_free(&frame);
}

void Encode()
{
	//int err;
	std::cout<<"cleaning Encoder"<<std::endl;
	//Write pending packets
	//while((err = write((AVFrame*)NULL)) == 1){};
	//if(err < 0 ){
	//std::cout <<"error writing delayed frame"<<std::endl;
	//}
	//Write file trailer before exit
	av_write_trailer(fmt_ctx);
	//close file
	avio_close(fmt_ctx->pb);
	avformat_free_context(fmt_ctx);
	avcodec_free_context(&codec_ctx);

	//av_freep(&tmp_frame->data[0]);
	//av_frame_free(&tmp_frame);
}

DataT GetData()
{
	return {};
}


}