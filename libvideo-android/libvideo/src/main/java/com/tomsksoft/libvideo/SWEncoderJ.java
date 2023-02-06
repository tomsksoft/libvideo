 /**
 * JNI wrapper for Encoder
 *
 * Example of usage Encoder: AndroidSDK/cameratest2/src/main/java/com/example/cameratest2/MainActivity.java.
 */

package com.tomsksoft.libvideo;

public class SWEncoderJ {

    private final long cppEncoderPtr;
    private final int outStorageIndex;
    private final byte[] encodedData;

    public SWEncoderJ(EncoderParamsJ params) {
        cppEncoderPtr = createEncoder(params, Utilities.getPresetCodeByEnum(params.preset));
        outStorageIndex = addOutput(cppEncoderPtr);
        encodedData = new byte[params.width * params.height];
    }
    private native long createEncoder(EncoderParamsJ params, int presetCode);
    private native int addOutput(long cppEncoderPtr);

    public byte[] encodeFrame(byte[] rgba_data) {
        return encodeFrame(cppEncoderPtr, outStorageIndex, rgba_data);
    }

    private native byte[] encodeFrame(long cppEncoderPtr, int outStorageIndex, byte[] rgba_data);

    public void setLogsActive(boolean enableLogs) { setLogsActive(cppEncoderPtr, enableLogs); }
    private native void setLogsActive(long cppEncoderPtr, boolean enableLogs);

}
