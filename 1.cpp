#include <iostream>
#include <string>

// اجباری برای استفاده از توابع C در محیط C++
extern "C" {
    #include <libavformat/avformat.h>
    #include <libavcodec/avcodec.h>
    #include <libavutil/imgutils.h> // برای توابع کمکی
}

using namespace std;

void log_error(const string& message, int error_code = 0) {
    char errbuf[AV_ERROR_MAX_STRING_SIZE];
    av_make_error_string(errbuf, AV_ERROR_MAX_STRING_SIZE, error_code);
    cerr << "FATAL ERROR: " << message << " (FFmpeg error: " << errbuf << ")" << endl;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        cerr << "استفاده: " << argv[0] << " <مسیر_فایل.ts>" << endl;
        return 1;
    }
    
    const char* filename = argv[1];
    AVFormatContext *format_context = nullptr;
    AVCodecContext *codec_context = nullptr;
    AVCodec *codec = nullptr;
    int video_stream_index = -1;

    // ۱. باز کردن فایل ورودی
    cout << "در حال باز کردن فایل: " << filename << endl;
    if (avformat_open_input(&format_context, filename, NULL, NULL) < 0) {
        cerr << "خطا: باز کردن فایل ورودی امکان پذیر نیست." << endl;
        return 2;
    }
    
    // ۲. یافتن اطلاعات جریان (Stream Info)
    if (avformat_find_stream_info(format_context, NULL) < 0) {
        cerr << "خطا: یافتن اطلاعات جریان امکان پذیر نیست." << endl;
        avformat_close_input(&format_context);
        return 3;
    }

    // ۳. یافتن بهترین جریان ویدئویی
    video_stream_index = av_find_best_stream(format_context, AVMEDIA_TYPE_VIDEO, -1, -1, &codec, 0);
    if (video_stream_index < 0) {
        cerr << "خطا: جریان ویدئویی معتبری یافت نشد." << endl;
        avformat_close_input(&format_context);
        return 4;
    }

    // ۴. ساخت Codec Context و باز کردن Codec
    AVStream *video_stream = format_context->streams[video_stream_index];
    codec_context = avcodec_alloc_context3(codec);
    if (!codec_context) {
        cerr << "خطا: تخصیص حافظه برای Codec Context ناموفق." << endl;
        avformat_close_input(&format_context);
        return 5;
    }
    
    if (avcodec_parameters_to_context(codec_context, video_stream->codecpar) < 0) {
        cerr << "خطا: کپی کردن پارامترهای کدک ناموفق." << endl;
        avcodec_free_context(&codec_context);
        avformat_close_input(&format_context);
        return 6;
    }

    if (avcodec_open2(codec_context, codec, NULL) < 0) {
        cerr << "خطا: باز کردن Codec ناموفق." << endl;
        avcodec_free_context(&codec_context);
        avformat_close_input(&format_context);
        return 7;
    }
    
    cout << "Codec ویدئو با موفقیت باز شد: " << avcodec_get_name(codec->id) << endl;

    // ۵. حلقه‌ی خواندن (Read Loop) و دیکدینگ
    AVPacket *packet = av_packet_alloc();
    AVFrame *frame = av_frame_alloc();
    int frame_count = 0;

    cout << "شروع خواندن و دیکد کردن فریم‌ها..." << endl;
    while (av_read_frame(format_context, packet) >= 0) {
        if (packet->stream_index == video_stream_index) {
            int ret = avcodec_send_packet(codec_context, packet);
            if (ret < 0) {
                log_error("خطا در ارسال پکت به دیکودر", ret);
                continue;
            }
            
            while (ret >= 0) {
                ret = avcodec_receive_frame(codec_context, frame);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                    break;
                } else if (ret < 0) {
                    log_error("خطا در دریافت فریم دیکد شده", ret);
                    break;
                }
                
                frame_count++;
                // در اینجا باید کد رندرینگ (مثل SDL) را اضافه کنید
                // ما فعلاً فقط تعداد فریم‌ها را چاپ می‌کنیم
                if (frame_count % 100 == 0) {
                    cout << "\rتعداد فریم‌های دیکد شده: " << frame_count << flush;
                }
            }
        }
        av_packet_unref(packet);
    }
    cout << "\rتعداد کل فریم‌های دیکد شده: " << frame_count << " - پایان خواندن." << endl;

    // ۶. پاکسازی
    av_frame_free(&frame);
    av_packet_free(&packet);
    avcodec_free_context(&codec_context);
    avformat_close_input(&format_context);
    cout << "حافظه با موفقیت آزاد شد." << endl;
    
    return 0;
}
