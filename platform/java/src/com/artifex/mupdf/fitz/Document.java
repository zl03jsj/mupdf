package com.artifex.mupdf.fitz;

import com.z.*;

public class Document
{
	static {
		Context.init();
	}

	public static final String META_FORMAT = "format";
	public static final String META_ENCRYPTION = "encryption";
	public static final String META_INFO_AUTHOR = "info:Author";
	public static final String META_INFO_TITLE = "info:Title";

	protected long pointer;

	protected native void finalize();

	public void destroy() {
		finalize();
		pointer = 0;
	}

	private native long newNativeWithPath(String filename);
	private native long newNativeWithBuffer(byte buffer[], String magic);
	// private native long newNativeWithRandomAccessFile(RandomAccessFile file, String magic);

	public Document(String filename) {
		pointer = newNativeWithPath(filename);
	}

	public Document(byte buffer[], String magic) {
		pointer = newNativeWithBuffer(buffer, magic);
	}

	public native boolean needsPassword();
	public native boolean authenticatePassword(String password);

	public native int countPages();
	public native Page loadPage(int number);
	public native Outline loadOutline();
	public native String getMetaData(String key);

	public native boolean isUnencryptedPDF();

	// TODO: implement follow native functions
	// public native long pdfDocument();
	public native boolean pdfAddSignature(Page page, OpensslSignDevice dev, PdfSignAppearance app);
	public native boolean save(String path);

//	public native boolean pdfSignWithImage(Page page, Rect rect, byte[] imagedata, String pfxfile, String password);
//	public native void pdfSignKeywordWithImage(Page page, String word, byte[] imgdata);
//	public native void pdfSignEverypageWithImage(int from, int to, Rect r, byte[] imgdata);
//	public native void pdfSignRightsideCrosspageWithImage(int from, int to, byte[] imagedata, int y);
//	public native void pdfSignCrossdoublepageWithImage(int from, int to, byte[] imagedata, int y);

}
