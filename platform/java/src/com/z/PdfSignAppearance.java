package com.z;

import com.artifex.mupdf.fitz.Rect;

/**
 * Created by zl03jsj on 2016/12/27.
 */
public class PdfSignAppearance {
    private long pointer;

    public PdfSignAppearance(String imgfile, Rect rect) {
        pointer = newNativeWithImageFile(imgfile, rect);
    }

    public PdfSignAppearance(byte[] imgdata, Rect rect) {
        pointer = newNativeWithImageData(imgdata, rect);
    }

    protected native void finalize();
    private static native long newNativeWithImageFile(String imgfile, Rect rect);
    private static native long newNativeWithImageData(byte[] imgdata, Rect rect);
}
