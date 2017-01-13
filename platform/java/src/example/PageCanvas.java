package example;

import com.artifex.mupdf.fitz.*;
import com.z.OpensslSignDevice;
import com.z.PdfSignAppearance;

import javax.imageio.ImageIO;
import java.awt.*;
import java.awt.Point;
import java.awt.event.*;
import java.awt.image.*;
import java.io.File;

public class PageCanvas extends Canvas implements MouseListener, MouseMotionListener, ComponentListener
{
	static final int ViewModeNormal = 0;
	static final int ViewModeImageSign = 1;
	static final int ViewModeHandSign = 2;

	protected Page page;
	protected BufferedImage image;

	private float borderwidth;

	private float scale;
    private Rect pageRect;
	private Rect signRect;
	private Matrix pageToCanvasMtx;
	public int viewMode;

	private String imageFile;
	private java.awt.Image signImage;
	private boolean dragflag;
	private Point lastPoint;

	public static BufferedImage imageFromPixmap(Pixmap pixmap) {
		int w = pixmap.getWidth();
		int h = pixmap.getHeight();
		BufferedImage image = new BufferedImage(w, h, BufferedImage.TYPE_INT_ARGB);
		image.setRGB(0, 0, w, h, pixmap.getPixels(), 0, w);
		return image;
	}

	public boolean doSign(Document doc) {
		OpensslSignDevice device = getOpensslDevice();
		PdfSignAppearance app = getSignApp();
		boolean isok = doc.pdfAddSignature(page, device, app);
		device.destroy();
		app.destroy();
		return isok;
	}

	public void updatePageImage() {
		if(image!=null)
			image = null;
		image = imageFromPage(page, Matrix.Scale(scale));
        repaint();
	}

	public void cancelSign() {
		if(null!=signImage) signImage = null;
        viewMode = ViewModeNormal;
		dragflag = false;
	}

	private boolean innerDosign(OpensslSignDevice device, PdfSignAppearance app) {
		return false;
	}

	private OpensslSignDevice getOpensslDevice() {
		Frame owner = (Frame)getParent();
		String pfxfile = Helper.pfxSelect(owner);
		if(null!=pfxfile) {
            return Helper.createOpensslDevice(pfxfile, owner);
		}
		return null;
	}

	private PdfSignAppearance getSignApp() {
		Rect r = new Rect(signRect);
		r.transform(new Matrix(Matrix.Identity(), pageToCanvasMtx).invert());
		PdfSignAppearance appearance = new PdfSignAppearance(imageFile, r);
		return appearance;
	}

	public static BufferedImage imageFromPageWithDevice(Page page, Matrix ctm) {
		Rect bbox = page.getBounds();
		Pixmap pixmap = new Pixmap(ColorSpace.DeviceBGR, bbox);
		pixmap.clear(255);
		DrawDevice dev = new DrawDevice(pixmap);
		page.run(dev, new Matrix());
		dev.destroy();
		BufferedImage image = imageFromPixmap(pixmap);
		pixmap.destroy();
		return image;
	}

	public static BufferedImage imageFromPage(Page page, Matrix ctm) {
		Pixmap pixmap = page.toPixmap(ctm, ColorSpace.DeviceRGB, true);
		BufferedImage image = imageFromPixmap(pixmap);
		pixmap.destroy();
		return image;
	}

	public PageCanvas(Page page_) {
		page = page_;
        scale = 1.0f;
		borderwidth = 3.0f;
		viewMode = ViewModeNormal;
        imageFile = null;
        lastPoint = new Point();

		pageToCanvasMtx = new Matrix();
		// add mouse motion listener for mouse move event
		addMouseMotionListener(this);
		addMouseListener(this);
        addComponentListener(this);
	}

	public Dimension getPreferredSize() {
        if(null!=image) {
			return new Dimension(image.getWidth(), image.getHeight());
		}
		return new Dimension(0, 0);
	}

	public Dimension getMinimumSize() {
		return getPreferredSize();
	}

	public Dimension getMaximumSize() {
		return getPreferredSize();
	}

	public void paint(Graphics g) {
		Graphics2D g2 = (Graphics2D) g;
		BasicStroke stroke = new BasicStroke(borderwidth, BasicStroke.CAP_ROUND, BasicStroke.JOIN_ROUND);
		g2.setColor(new Color(0x80, 0x80, 0x80));
        g2.setStroke(stroke);

		Rectangle r = awtRectangleFromRect(pageRect);
		g2.drawImage(image, r.x, r.y, r.width, r.height, Color.white, null);

        if(signImage!=null) {
            r = awtRectangleFromRect(signRect);
			g2.setComposite(AlphaComposite.getInstance(AlphaComposite.SRC_OVER, 0.9f));
			g2.drawImage(signImage, r.x, r.y, r.width, r.height, null);

			float[] dash = {2.0f, 3.0f};
			g2.setStroke(new BasicStroke(1.0f, BasicStroke.CAP_ROUND, BasicStroke.JOIN_ROUND, 3.0f, dash, 0.0f));
			g2.setColor(Color.blue);
			g2.drawRect(r.x, r.y, r.width, r.height);
		}
	}

	// add by zl03jsj@163.com
	// for hand drawing, location image, for add pdf signature.
	// mouse events
	public void mouseMoved(MouseEvent e) {}
	public void mouseClicked (MouseEvent e){}

	public void mousePressed (MouseEvent e)
	{
        if( viewMode == ViewModeImageSign ) {
			java.awt.Point point = new java.awt.Point(e.getX(), e.getY());
            if(!signRect.contains((float)point.x, (float)point.y))
            	return;

            lastPoint.x = e.getX();
			lastPoint.y = e.getY();
			dragflag = true;
        }
        else if( viewMode == ViewModeHandSign ) {
			// TODO:do something for handsign....
		}
	}

	public void mouseReleased(MouseEvent e){
		if(viewMode==ViewModeImageSign && dragflag) {
			dragflag = false;
        }

	}

	public void mouseEntered (MouseEvent e){}
	public void mouseExited  (MouseEvent e){}

    private int getRealX(int x) {
        if( x<pageRect.x0 )
            x = Math.round(pageRect.x0);
        else if( x>pageRect.x1 )
            x = Math.round(pageRect.x1);

        return x;
    }

    private int getRealY(int y) {
        if( y<pageRect.y0 )
            y = Math.round(pageRect.y0);
        else if( y>pageRect.y1 )
            y = Math.round(pageRect.y1);

        return y;
    }

	public void mouseDragged(MouseEvent e)
	{
		if(e.getButton()!=MouseEvent.BUTTON1)
			return;

		if(viewMode==ViewModeImageSign && dragflag) {
            int x = getRealX(e.getX());
            int y = getRealY(e.getY());
			int dx = x - lastPoint.x;
			int dy = y - lastPoint.y;

            if( dx!=0 || dy!=0 ){
				lastPoint.setLocation(x, y);
                signRect.transform(Matrix.Translate(dx, dy));
                repaint();
            }
		}
		else if(viewMode==ViewModeHandSign) {
			// TODO:do something for handsign....
		}

	}
	// component events
	public void componentMoved(ComponentEvent e) { }
	public void componentShown(ComponentEvent e) { }
	public void componentHidden(ComponentEvent e) { }

	public void componentResized(ComponentEvent e) {
        Dimension size = new Dimension(getSize());
        if(pageRect==null)
            pageRect = page.getBounds();
        else
            pageRect.transform(pageToCanvasMtx.invert());

		scale = Math.min((float)size.width / pageRect.width(), (float)size.height / pageRect.height());

		size.width -= 2* borderwidth;
		size.height-= 2* borderwidth;

        float w = pageRect.width() *scale;
		float h = pageRect.height()*scale;
		float x = borderwidth+(size.width -w)/2;
        float y = borderwidth+(size.height-h)/2;

        if(signRect!=null)
            signRect.transform(pageToCanvasMtx);

        pageToCanvasMtx = Matrix.Scale(scale).concat(Matrix.Translate(x, y));


        if(signRect!=null)
            signRect.transform(pageToCanvasMtx);

        pageRect.transform(pageToCanvasMtx);
		image = imageFromPage(page, Matrix.Scale(scale));
	}

	public static Rectangle awtRectangleFromRect(Rect r){
		RectI ri = r.round();
		return new Rectangle(ri.x0, ri.y0, ri.x1-ri.x0, ri.y1-ri.y0);
	}

	public boolean beginSign() {
		imageFile = "/Users/zl03jsj/Documents/pdftest/monkeysmile.JPG"; // Viewer.fileSelect(Viewer.imgfilter);
		BufferedImage image = null;
		try {
			image = ImageIO.read(new File(imageFile));
		} catch (java.io.IOException e) {
			imageFile = null;
			e.printStackTrace();
		}

        if(image==null)
        	return false;

        if(signImage!=null) signImage = null;

        signImage = Transparency.makeColorTransparent(image, Color.white);
        if(signRect==null) signRect = new Rect();

		signRect.set(0, 0, 100, 100);
        signRect.transform(pageToCanvasMtx);
        viewMode = ViewModeImageSign;
        repaint();
		return true;
	}

//	public static void setImageBorder(BufferedImage image) {
//		Graphics2D g = image.createGraphics();
//		float[] dash = {2.0f, 2.0f};
//		g.setStroke(new BasicStroke(2.0f, BasicStroke.CAP_ROUND, BasicStroke.JOIN_ROUND, 3.0f, dash, 0.0f));
//		g.setColor(Color.blue);
//		g.drawRect(1, 1, image.getWidth()-2, image.getHeight()-2);
//		g.dispose();
//	}
}
