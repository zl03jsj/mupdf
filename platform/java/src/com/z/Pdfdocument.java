package com.z;

import com.artifex.mupdf.fitz.Document;
import java.lang.reflect.Field;

/**
 * Created by zl03jsj on 2016/12/27.
 */
public class Pdfdocument {
    private long pointer;

    public static Pdfdocument fromDocument(Document document) {
        Pdfdocument doc = null;
//        long pointer_ = document.pdfDocument();
//        doc = new Pdfdocument(pointer_);
        return doc;
        /*
        Field fpointer = null;
        try {
            fpointer = document.getClass().getField("pointer");
            fpointer.setAccessible(true);
            doc = new Pdfdocument((long)fpointer.get(document));
            fpointer.set(document, 0);
            fpointer.setAccessible(false);
        }
        catch(java.lang.Exception e){
            if(fpointer!=null) fpointer.setAccessible(false);
            e.printStackTrace();
        }
        */
    }


    private Pdfdocument(long pointer_) {
        pointer = pointer_;
//        keep();
    }

    // TODO:implement native function
    // private native void keep();
}
