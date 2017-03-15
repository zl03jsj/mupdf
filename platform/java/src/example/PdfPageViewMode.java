package example;

/**
 * Created by zl03jsj on 2017/1/12.
 */
public class PdfPageViewMode
{
    static final int mode_normal = 0;
    static final int mode_image_sign = 1;
    static final int mode_hand_sign = 2;

    private int mode;
    private PdfPageViewMode(int _mode) {
        mode = _mode;
    }

    static {
        ViewModeNormal = new PdfPageViewMode(mode_normal);
        ViewModeImageSign = new PdfPageViewMode(mode_image_sign);
        ViewModeHandSign = new PdfPageViewMode(mode_hand_sign);
    }

    public static PdfPageViewMode ViewModeNormal;
    public static PdfPageViewMode ViewModeImageSign;
    public static PdfPageViewMode ViewModeHandSign;

}
