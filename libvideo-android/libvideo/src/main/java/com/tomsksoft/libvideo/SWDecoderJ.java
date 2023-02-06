/**
 * JNI wrapper for Decoder
 *
 * Example of usage Decoder: AndroidSDK/cameratest2/src/main/java/com/example/cameratest2/MainActivity.java.
 */

package com.tomsksoft.libvideo;

public class SWDecoderJ {
    private final long cppDecoderPtr;
    private final int outStorageIndex;
    private final byte[] decodedData;

    public SWDecoderJ(DecoderParamsJ params){
        cppDecoderPtr = createDecoder(params);
        outStorageIndex = addOutput(cppDecoderPtr);
        decodedData = new byte[params.width * params.height * 4];
    }
    private native long createDecoder(DecoderParamsJ params);
    private native int addOutput(long cppDecoderPtr);

    public byte[] decode(byte[] encoded_data) {
        return decode(cppDecoderPtr, outStorageIndex, encoded_data); }
    private native byte[] decode(long cppDecoderPtr, int outStorageIndex, byte[] encoded_data);

    public void setLogsActive(boolean enableLogs) { setLogsActive(cppDecoderPtr, enableLogs); }
    private native void setLogsActive(long cppDecoderPtr, boolean enableLogs);
}
