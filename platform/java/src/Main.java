import com.artifex.mupdf.fitz.Document;
import com.artifex.mupdf.fitz.Page;
import com.artifex.mupdf.fitz.Rect;
import com.z.OpensslSignDevice;
import com.z.PdfSignAppearance;

public class Main {
    public static void main(String[] args) {
        final String rootPath = "/Users/zl03jsj/Documents/pdftest/";
        final String pdffile  = "test.pdf";
        final String savefile = "jni_saved.pdf";

        String pfxfile = rootPath + "user/zl.pfx";
        String pfxpassword = "111111";
        String imagefile = rootPath + "monkeysmile.JPG";

        Document doc = new Document(rootPath + "pdffile/" + pdffile);
        int pagecount = doc.countPages();
        Page page = doc.loadPage(0);

        Rect rect = page.getBounds();
//        System.out.println("page rect" + rect.toString());

        rect = new Rect(0, 0, 100, 100);

        OpensslSignDevice device = OpensslSignDevice.fromPfxfile(pfxfile, pfxpassword);
        PdfSignAppearance appearance = new PdfSignAppearance(imagefile, rect);

        if( device!=null && appearance!=null ){
            doc.pdfAddSignature(page, device, appearance);
            doc.save(rootPath + savefile);
            System.out.println("save file:" + rootPath+savefile);
            System.out.println("done!");
        }
        else {
            System.out.println("sign device or appearance is null.");
        }
    }
}
