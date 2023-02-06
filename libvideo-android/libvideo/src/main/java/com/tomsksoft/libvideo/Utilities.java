/*
 * JNI wrapper for GeneralUtilities
 */

package com.tomsksoft.libvideo;

public class Utilities {
    public Utilities() {}

    public static byte[] makeRawSMPTEImage(int width, int height){
        return makeRawSMPTEImageCPP(width, height);
    }
    private static native byte[] makeRawSMPTEImageCPP(int width, int height);

    public static byte[] getExtradataByEncoderParams(EncoderParamsJ params) {
        return getExtradataByEncoderParamsCPP(params, Utilities.getPresetCodeByEnum(params.preset));
    }

    private static native byte[] getExtradataByEncoderParamsCPP(EncoderParamsJ params, int presetCode);

    public static int getPresetCodeByEnum(LibvideoH264Preset preset) {
        switch (preset) {
            case ULTRAFAST:
                return 0;
            case SUPERFAST:
                return 1;
            case VERYFAST:
                return 2;
            case FASTER:
                return 3;
            case FAST:
                return 4;
            case MEDIUM:
                return 5;
            case SLOW:
                return 6;
            case SLOWER:
                return 7;
            case VERYSLOW:
                return 8;
            default:
                return 3;
        }
    };
}
