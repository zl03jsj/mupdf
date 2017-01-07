package com.z;

/**
 * Created by zl03jsj on 2016/12/27.
 */
public class OpensslSignDevice {
    private long pointer;

//    public OpensslSignDevice(String pfxfile, String password) {
//        pointer = newNativeWithPfxFile(pfxfile, password);
//    }

    private OpensslSignDevice(long devPointer) {
        pointer = devPointer;
    }

    protected native void finalize();

    private static native long newNativeWithPfxFile(String pfxfile, String password);

    public static OpensslSignDevice fromPfxfile(String pfxfile, String password)
    {
        long devPointer = newNativeWithPfxFile(pfxfile, password);
        if(devPointer!=0) {
            return new OpensslSignDevice(devPointer);
        }
        return null;
    }
}

