package com.artifex.mupdf.fitz;

import java.io.FileOutputStream;
import java.io.InputStream;

// This class handles the loading of the MuPDF shared library, together
// with the ThreadLocal magic to get the required context.
//
// The only publicly accessible method here is Context.setStoreSize, which
// sets the store size to use. This must be called before any other MuPDF
// function.
public class Context
{
	private static boolean inited = false;
	private static native int initNative();

	public static void init() {
		if (!inited) {
			try {
				System.loadLibrary(getLibraryName());
				inited = true;
			}
			catch (java.lang.UnsatisfiedLinkError e) {
				e.printStackTrace();
			}
			if( !inited ) {
				String absolutepath = unpackeNativeMupdflib();
				System.load(absolutepath);
                inited = true;
			}
			if (initNative() < 0)
				throw new RuntimeException("cannot initialize mupdf library");
		}
	}

	public static native int gprfSupportedNative();

	private static String getLibraryName() {
		/* Mac OS always uses 64bit DLLs for any JDK 1.7 or above */
		if (System.getProperty("os.name").toLowerCase().contains("mac os")) {
			return "mupdf_java64";
		}
		/* Sun and OpenJDK JVMs support this way of finding bittedness */
		String val = System.getProperty("sun.arch.data.model");
		/* Android does NOT support this, and returns NULL */
		if (val != null && val.equals("64")) {
			return "mupdf_java64";
		}

		/* We might be running Android here. We could find out by
		 * doing the following test:
		 *  val = System.getProperty("java.vm.name");
		 *  if (val != null && val.toLowerCase().contains("dalvik")) {
		 *    ...Do something Androidy to test for 32/64 here...
		 *  }
		 * (Currently, both Dalvik and ART return 'Dalvik').
		 * We know of no portable way to detect 32 or 64bittedness
		 * on android though, so for now will assume 32.
		 */
		return "mupdf_java32";
	}

	static { init(); }

    private static boolean pathinit = false;
    private static String libname;
	private static String libsuffix;

	private static void initPaths() {
        if(pathinit)
        	return;
        String osName = System.getProperty("os.name").toLowerCase();
        if( osName.contains("mac os") ) {
			libname = "mupdf_java64";
            libsuffix = ".jnilib";
		}
		else {
			String val = System.getProperty("sun.arch.data.model");
			/* Android does NOT support this, and returns NULL */
			if (val != null && val.equals("64"))
				libname = "mupdf_java64";

			libname = "mupdf_java64";
			libsuffix = ".so";
		}
		pathinit = true;
	}

	private static String unpackeNativeMupdflib() {
		initPaths();
		String absolutPath = System.getProperty("user.dir") + "/";
		String libRealName = libname + libsuffix;
		String libFullpath = absolutPath + libRealName;

		try {
			InputStream in = Context.class.getResourceAsStream("/lib/" + libRealName);
            if(null==in) {
				// linux system's libaray name begin with "lib"
				in = Context.class.getResourceAsStream("/lib/" + "lib" + libRealName);
			}
            java.io.File outFile = new java.io.File(libFullpath);
			FileOutputStream out = new FileOutputStream(outFile, false);

			byte[] buf = new byte[1024];
			int len = 0;

			while(-1!=(len=in.read(buf)))
				out.write(buf, 0, len);

			out.close();
			in.close();
		}
		catch (Exception e) {
			e.printStackTrace();
            libFullpath = null;
		}
		return libFullpath;
	}

	// FIXME: We should support the store size being changed dynamically.
	// This requires changes within the MuPDF core.
	//public native static void setStoreSize(long newSize);
}
