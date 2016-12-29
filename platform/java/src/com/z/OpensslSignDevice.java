package com.z;

/**
 * Created by zl03jsj on 2016/12/27.
 */
public class OpensslSignDevice {
    private long pointer;

    public OpensslSignDevice(String pfxfile, String password) {
        pointer = newNativeWithPfxFile(pfxfile, password);
    }

    protected native void finalize();

    private static native long newNativeWithPfxFile(String pfxfile, String password);
}

