package example;

import java.awt.*;
import java.awt.image.*;

/**
 * Created by zl03jsj on 2017/1/11.
 */
public class Transparency {
    public static Image makeColorTransparent(java.awt.Image image, final Color color)
    {
        ImageFilter filter = new RGBImageFilter() {
            // the color we are looking for... Alpha bits are set to opaque
            private int r = getR(color.getRGB());
            private int g = getG(color.getRGB());
            private int b = getB(color.getRGB());

            private int getR(int rgb){return (rgb & 0x00FF0000)>>16;}
            private int getG(int rgb){return (rgb & 0x0000FF00)>>8;}
            private int getB(int rgb){return (rgb & 0x000000FF);}

            private boolean isTransparent(int rgb) {
                int dr = Math.abs(r-getR(rgb));
                int dg = Math.abs(g-getR(rgb));
                int db = Math.abs(b-getR(rgb));
                return (dr+dg+db) < 9;
            }

            public final int filterRGB(int x, int y, int rgb) {
                if ( isTransparent(rgb) ) {
                    // Mark the alpha bits as zero - transparent
                    return 0x00FFFFFF & rgb;
                } else {
                    // nothing to do
                    return rgb;
                }
            }
        };

        ImageProducer ip = new FilteredImageSource(image.getSource(), filter);
        return Toolkit.getDefaultToolkit().createImage(ip);
    }

  public static BufferedImage makeImageTranslucent(BufferedImage source,
                                                   double alpha) {
      BufferedImage target = new BufferedImage(source.getWidth(),
              source.getHeight(), java.awt.Transparency.TRANSLUCENT);
      // Get the images graphics
      Graphics2D g = target.createGraphics();
      // Set the Graphics composite to Alpha
      g.setComposite(AlphaComposite.getInstance(AlphaComposite.SRC_OVER,
              (float) alpha));
      // Draw the image into the prepared reciver image
      g.drawImage(source, null, 0, 0);
      // let go of all system resources in this Graphics
      g.dispose();
      // Return the image
      return target;
  }
}
