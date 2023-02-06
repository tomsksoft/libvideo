/**
 * Parametres for creating EncoderJ.
 */

package com.tomsksoft.libvideo;

public class EncoderParamsJ {
    public int width = 0;
    public int height = 0;
    public long bit_rate = 0;
    public int framerate = 30;
    public LibvideoH264Preset preset = LibvideoH264Preset.FASTER;
}