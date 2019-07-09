#include <jni.h>
#include <string>
#include <malloc.h>
#include <android/bitmap.h>
#include <jpeglib.h>


/**
 * 对图片数据进行哈夫曼编码
 * @param data 图片数据（排除了alpha通道数据）
 * @param width 图片宽
 * @param height 图片高
 * @param quality 压缩的质量
 * @param path 压缩后图片保存路径
 */
void compress_JPEG_file(uint8_t *data, int width, int height, jint quality, const char *path) {
//   创建jpeg压缩对象
     jpeg_compress_struct jcs;
    //存储错误信息
     jpeg_error_mgr error;
     jcs.err = jpeg_std_error(&error);
    //创建压缩对象
    jpeg_create_compress(&jcs);

//  指定存储文件
    FILE *f = fopen(path,"wb");
    jpeg_stdio_dest(&jcs,f);
//  设置压缩参数
    jcs.image_width = width;
    jcs.image_height = height;
    //bgr
    jcs.input_components = 3;
    jcs.in_color_space =  JCS_RGB;
    jpeg_set_defaults(&jcs);
    //是否开启哈夫曼 1=true 0=false 关键参数
    jcs.optimize_coding = 1;
    jpeg_set_quality(&jcs, quality, 1);
//  开始压缩
    jpeg_start_compress(&jcs,1);
//  每行数据的字节数
    int row_stride = width * 3;
    JSAMPROW row[1];
    while (jcs.next_scanline < jcs.image_height) {
        //拿一行数据
        uint8_t *pixels = data + jcs.next_scanline * row_stride;
        row[0] = pixels;
        jpeg_write_scanlines(&jcs, row, 1);
    }
//  压缩完成
    jpeg_finish_compress(&jcs);
//  释放资源
    fclose(f);
    jpeg_destroy_compress(&jcs);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_wanglei_libjpeg_MainActivity_nativeLibjpegCompress(JNIEnv *env, jobject instance,
                                                            jobject bitmap, jint quality,
                                                            jstring path_) {
        const char *path = env->GetStringUTFChars(path_, 0);

        AndroidBitmapInfo info;
        //获得bitmap的信息:宽、高等
        AndroidBitmap_getInfo(env,bitmap,&info);
        uint8_t *pixels;
        //锁定bitmap获取每个像素信息，pixels会指向内存中图片数据的起始位置
        AndroidBitmap_lockPixels(env, bitmap, (void **) &pixels);

        int w = info.width;
        int h = info.height;
        int color;//存储每个像素数据，int正好是4字节
        //分配w * h * 3字节空间，后面会去掉图片的alpha通道数据，
        //原本是每个像素包含argb 4个字节数据，去掉一个alpha数据后
        //每个像素只需要占用3字节空间
        uint8_t* data = (uint8_t *) malloc(w * h * 3);
        uint8_t* temp = data;//temp记录起始地址，后续操作会移动data指针。
        uint8_t r,g,b;
        //遍历每个像素信息，取出每个像素的rgb数据存储起来
        for(int i =0;i<h;++i){
            for (int j = 0; j < w; ++j) {
                //获取每个像素信息
                color = *(int*)pixels;
                //从每个像素取出rgb数据，每个像素数据排列方式为argb
                r = (color >> 16) & 0xFF;
                g = (color >> 8) & 0xFF;
                b = color & 0xFF;
                //libjpeg中每个像素存储数据顺序为bgr顺序，而不是rgb顺序，
                //古老的库多数用bgr顺序存储
                *data = b;
                *(data+1) = g;
                *(data+2) = r;
                data += 3;
                //指针 跳过4个字节
                pixels +=4; //4-7字节
            }
        }
        //经过上面处理图像数据(除去alpha通道后数据)，都保存在temp所指向的空间了
        //继续可以利用libjpeg进行哈夫曼编码了
        compress_JPEG_file(temp,w,h,quality,path);

        AndroidBitmap_unlockPixels(env,bitmap);
        free(temp);//记得一定要释放内存
        env->ReleaseStringUTFChars(path_, path);
}