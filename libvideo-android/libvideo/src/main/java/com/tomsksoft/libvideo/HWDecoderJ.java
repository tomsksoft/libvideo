package com.tomsksoft.libvideo;

import android.view.Surface;

public class HWDecoderJ {
    private final long cppDecoderPtr;

    public HWDecoderJ(DecoderParamsJ params, Surface surface){
        cppDecoderPtr = createDecoder(params, surface);
    }
    private native long createDecoder(DecoderParamsJ params, Surface surface);

    public void decode(byte[] encoded_data) {
        decode(cppDecoderPtr, encoded_data);
    }
    private native void decode(long dec_ptr, byte[] encoded_data);

    public void setLogsActive(boolean enableLogs) { setLogsActive(cppDecoderPtr, enableLogs); }
    private native void setLogsActive(long cppDecoderPtr, boolean enableLogs);
}
