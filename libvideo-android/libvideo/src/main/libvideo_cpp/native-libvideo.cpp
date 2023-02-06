extern "C" {
#include <libavcodec/jni.h>
}
#include <jni.h>
#include <mutex>

#include "libvideo.h"

static std::mutex mtx;

static std::vector<std::vector<uint8_t>> encoder_outputs;
static std::vector<std::vector<uint8_t>> decoder_outputs;

static std::vector<uint8_t> sw_initial_data_cpp;

static std::vector<uint8_t> sw_encoded_data_cpp;
static std::vector<uint8_t> hw_encoded_data_cpp;

using namespace tomsksoft::libvideo;

enum LibvideoH264Preset libvideo264preset_from_presetCode(jint presetCode) {
    switch (presetCode) {
        case 0:
            return LibvideoH264Preset::Ultrafast;

        case 1:
            return LibvideoH264Preset::Superfast;

        case 2:
            return LibvideoH264Preset::Veryfast;

        case 3:
            return LibvideoH264Preset::Faster;

        case 4:
            return LibvideoH264Preset::Fast;

        case 5:
            return LibvideoH264Preset::Medium;

        case 6:
            return LibvideoH264Preset::Slow;

        case 7:
            return LibvideoH264Preset::Slower;

        case 8:
            return LibvideoH264Preset::Veryslow;

        default:
            return LibvideoH264Preset::Faster;
    }
};

EncoderParams prepare_enc_params(JNIEnv *env, jobject params, jint presetCode) {
    jclass cls = env->GetObjectClass(params);

    jfieldID fidWidth = env->GetFieldID(cls, "width", "I");
    jint width = env->GetIntField(params, fidWidth);

    jfieldID fidHeight = env->GetFieldID(cls, "height", "I");
    jint height = env->GetIntField(params, fidHeight);

    jfieldID fidBitrate = env->GetFieldID(cls, "bit_rate", "J");
    jlong bit_rate = env->GetLongField(params, fidBitrate);

    jfieldID fidFramerate = env->GetFieldID(cls, "framerate", "I");
    jint framerate = env->GetIntField(params, fidFramerate);

    EncoderParams params_cpp;
    params_cpp.width = width;
    params_cpp.height = height;
    params_cpp.bit_rate = bit_rate;
    params_cpp.framerate = framerate;
    params_cpp.preset = libvideo264preset_from_presetCode(presetCode);

    return params_cpp;
}

DecoderParams prepare_dec_params(JNIEnv *env, jobject params) {
    jclass cls = env->GetObjectClass(params);

    jfieldID fidWidth = env->GetFieldID(cls, "width", "I");
    jint width = env->GetIntField(params, fidWidth);

    jfieldID fidHeight = env->GetFieldID(cls, "height", "I");
    jint height = env->GetIntField(params, fidHeight);

    jfieldID fidExtradata = env->GetFieldID(cls, "extradata", "[B");
    auto extradata = (jbyteArray)env->GetObjectField( params, fidExtradata);
    std::vector<uint8_t> extradata_cpp;

    if (extradata)
    {
        jboolean cp = false;
        auto extradata_ptr = (uint8_t*)(env->GetByteArrayElements(extradata, &cp));
        extradata_cpp.insert(extradata_cpp.begin(),extradata_ptr, extradata_ptr + env->GetArrayLength(extradata));
    }

    DecoderParams params_cpp;
    params_cpp.width = width;
    params_cpp.height = height;
    params_cpp.extradata = extradata_cpp;

    return params_cpp;
}

extern "C"
{
    JNIEXPORT jint
    JNI_OnLoad(JavaVM *vm, void *reserved)
    {
        av_jni_set_java_vm(vm, nullptr);
        return JNI_VERSION_1_6;
    }

    JNIEXPORT jlong JNICALL
    Java_com_tomsksoft_libvideo_SWEncoderJ_createEncoder(JNIEnv *env, jobject thiz, jobject params, jint presetCode) {
        return reinterpret_cast<jlong>(CreateEncoder(prepare_enc_params(env, params, presetCode), EncoderTypes::Software));
    }

    JNIEXPORT jint JNICALL
    Java_com_tomsksoft_libvideo_SWEncoderJ_addOutput(JNIEnv *env, jobject thiz, jlong cpp_encoder_ptr) {
        std::unique_lock<std::mutex>lck (mtx);
        std::vector<uint8_t> out_vec;
        encoder_outputs.emplace_back(out_vec);
        jint index = static_cast<jint>(encoder_outputs.size() - 1);
        auto output_callback = [index](std::vector<uint8_t> &encoded_data){
            encoder_outputs[index] = std::move(encoded_data);
        };
        reinterpret_cast<IEncoder *>(cpp_encoder_ptr)->add_output(output_callback);
        return index;
    }

    JNIEXPORT jbyteArray JNICALL
    Java_com_tomsksoft_libvideo_SWEncoderJ_encodeFrame(JNIEnv *env, jobject thiz, jlong cpp_encoder_ptr,
                                                     jint out_storage_index, jbyteArray rgba_data) {
        std::unique_lock<std::mutex>lck (mtx);
        jboolean cp = false;
        auto initial_bytes_cpp_ptr = (uint8_t*)(env->GetByteArrayElements(rgba_data, &cp));

        sw_initial_data_cpp.clear();
        sw_initial_data_cpp.insert(sw_initial_data_cpp.begin(), initial_bytes_cpp_ptr,
                                   initial_bytes_cpp_ptr + env->GetArrayLength(rgba_data));
        reinterpret_cast<IEncoder *>(cpp_encoder_ptr)->encode_frame(sw_initial_data_cpp);

        jclass cls = env->GetObjectClass(thiz);

        jfieldID fidEncodedData = env->GetFieldID(cls, "encodedData", "[B");
        auto encoded_data = (jbyteArray)(env->GetObjectField(thiz,fidEncodedData));
        env->SetByteArrayRegion(encoded_data, 0, (jsize)encoder_outputs[out_storage_index].size(),
                                (jbyte*)encoder_outputs[out_storage_index].data());

        return encoded_data;
    }

    JNIEXPORT void JNICALL
    Java_com_tomsksoft_libvideo_SWEncoderJ_setLogsActive(JNIEnv *env, jobject thiz, jlong cpp_encoder_ptr,
                                                   jboolean enable_logs) {
        reinterpret_cast<IEncoder *>(cpp_encoder_ptr)->set_logs_active(enable_logs);
    }

    JNIEXPORT jlong JNICALL
    Java_com_tomsksoft_libvideo_SWDecoderJ_createDecoder(JNIEnv *env, jobject thiz, jobject params) {
        return reinterpret_cast<jlong>(CreateDecoder(prepare_dec_params(env, params), DecoderTypes::Software));
    }

    JNIEXPORT jint JNICALL
    Java_com_tomsksoft_libvideo_SWDecoderJ_addOutput(JNIEnv *env, jobject thiz, jlong cpp_decoder_ptr) {
        std::unique_lock<std::mutex>lck (mtx);
        std::vector<uint8_t> out_vec;
        decoder_outputs.emplace_back(out_vec);
        jint index = static_cast<jint>(decoder_outputs.size() - 1);
        auto output_callback = [index](std::vector<uint8_t> &decoded_data){
            decoder_outputs[index] = std::move(decoded_data);
        };
        reinterpret_cast<IDecoder *>(cpp_decoder_ptr)->add_output(output_callback);
        return index;
    }
    JNIEXPORT jbyteArray JNICALL
    Java_com_tomsksoft_libvideo_SWDecoderJ_decode(JNIEnv *env, jobject thiz, jlong cpp_decoder_ptr,
                                                jint out_storage_index, jbyteArray encoded_data) {
        std::unique_lock<std::mutex>lck (mtx);
        jboolean cp = false;
        auto initial_bytes_cpp_ptr = (uint8_t*)(env->GetByteArrayElements(encoded_data, &cp));

        sw_encoded_data_cpp.clear();
        sw_encoded_data_cpp.insert(sw_encoded_data_cpp.begin(), initial_bytes_cpp_ptr,
                                   initial_bytes_cpp_ptr + env->GetArrayLength(encoded_data));
        reinterpret_cast<IDecoder *>(cpp_decoder_ptr)->decode(sw_encoded_data_cpp);

        jclass cls = env->GetObjectClass(thiz);

        jfieldID fidDecodedData = env->GetFieldID(cls, "decodedData", "[B");
        auto decoded_data = (jbyteArray)(env->GetObjectField(thiz,fidDecodedData));

        env->SetByteArrayRegion(decoded_data, 0, (jsize)decoder_outputs[out_storage_index].size(),
                                (jbyte*)decoder_outputs[out_storage_index].data());
        return decoded_data;
    }

    JNIEXPORT void JNICALL
    Java_com_tomsksoft_libvideo_SWDecoderJ_setLogsActive(JNIEnv *env, jobject thiz, jlong cpp_decoder_ptr,
                                                       jboolean enable_logs) {
        reinterpret_cast<IDecoder *>(cpp_decoder_ptr)->set_logs_active(enable_logs);
    }

    JNIEXPORT jlong JNICALL
    Java_com_tomsksoft_libvideo_HWDecoderJ_createDecoder(JNIEnv *env, jobject thiz, jobject params,
                                                       jobject surface) {
        return reinterpret_cast<jlong>(CreateDecoder(prepare_dec_params(env, params),
                                                     DecoderTypes::MEDIACODEC, (void*)surface));
    }

    JNIEXPORT void JNICALL
    Java_com_tomsksoft_libvideo_HWDecoderJ_decode(JNIEnv *env, jobject thiz, jlong cpp_decoder_ptr,
                                                jbyteArray encoded_data) {
        jboolean cp = false;
        auto encoded_bytes_cpp_ptr = (uint8_t*)(env->GetByteArrayElements(encoded_data, &cp));

        hw_encoded_data_cpp.clear();
        hw_encoded_data_cpp.insert(hw_encoded_data_cpp.begin(), encoded_bytes_cpp_ptr,
                                   encoded_bytes_cpp_ptr + env->GetArrayLength(encoded_data));
        reinterpret_cast<IDecoder *>(cpp_decoder_ptr)->decode(hw_encoded_data_cpp);
    }

    JNIEXPORT void JNICALL
    Java_com_tomsksoft_libvideo_HWDecoderJ_setLogsActive(JNIEnv *env, jobject thiz, jlong cpp_decoder_ptr,
                                                       jboolean enable_logs) {
        reinterpret_cast<IDecoder *>(cpp_decoder_ptr)->set_logs_active(enable_logs);
    }

    JNIEXPORT jbyteArray JNICALL
    Java_com_tomsksoft_libvideo_Utilities_makeRawSMPTEImageCPP(JNIEnv *env, jclass clazz, jint width,
                                                          jint height) {
        std::vector<uint8_t> raw_data_cpp;
        Utilities::fill_by_raw_SMPTE_rgba_data(raw_data_cpp, width, height);

        jbyteArray raw_data = env->NewByteArray((jsize)raw_data_cpp.size());
        env->SetByteArrayRegion(raw_data, 0, (jsize)raw_data_cpp.size(), (jbyte*)raw_data_cpp.data());

        return raw_data;
    }

    JNIEXPORT jbyteArray JNICALL
    Java_com_tomsksoft_libvideo_Utilities_getExtradataByEncoderParamsCPP(JNIEnv *env, jclass clazz,
                                                                       jobject params, jint presetCode) {
        EncoderParams enc_params = prepare_enc_params(env, params, presetCode);
        std::vector<uint8_t> extradata_cpp = Utilities::get_extradata_by_enc_params(enc_params);

        jbyteArray extradata_j = env->NewByteArray((jsize)extradata_cpp.size());
        env->SetByteArrayRegion(extradata_j, 0, (jsize)extradata_cpp.size(), (jbyte*)extradata_cpp.data());

        return extradata_j;
    }
}