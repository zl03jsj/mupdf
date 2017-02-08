package example;
/**
 * Created by zl03jsj on 2017/1/13.
 */

import com.z.OpensslSignDevice;
import javax.swing.*;
import javax.swing.filechooser.FileFilter;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.File;

class PfxCreatorDialog extends JDialog implements ActionListener{
    private String _pfxfile;
    private JPasswordField _password;
    private Button _ok;
    private Button _cancel;
    private Label _label;
    public OpensslSignDevice device;
    private int retainCount;

    public PfxCreatorDialog(Frame owner, String pfxfile) {
        super(owner, "input pfx file password...", true);
        Point location = owner.getLocationOnScreen();
        location.x += 20;
        location.y += 20;
        setLocation(location);
        setSize(300, 100);
        device = null;
        retainCount = 4;
        _pfxfile = pfxfile;

        Panel panel = new Panel(new FlowLayout(FlowLayout.LEFT, 0, 0));
        panel.add(new Label("password:"));
        _password = new JPasswordField(null, 16);
        _ok = new Button("ok");
        _cancel = new Button("canncel");
        panel.add(_password);
        add(panel, BorderLayout.NORTH);

        panel = new Panel(new FlowLayout(FlowLayout.LEFT, 0, 0));
        panel.add(_ok);
        panel.add(_cancel);
        _ok.addActionListener(this);
        _cancel.addActionListener(this);
        add(panel, BorderLayout.CENTER);

        _label = new Label("pfxfile:" + pfxfile, FlowLayout.LEFT);
        add(_label, BorderLayout.SOUTH);

        validate();
    }

    public void actionPerformed(ActionEvent e) {
        if(e.getSource()==_ok) {
            if(device!=null) device.destroy();
            String password = new String(_password.getPassword());
            device = OpensslSignDevice.fromPfxfile(_pfxfile, password);
            if(null==device) {
                retainCount--;
                _label.setText("woring password, retain " + retainCount + " times");
                if( 0==retainCount )
                    setVisible(false);
            }
            else
                setVisible(false);
        }
        else if(e.getSource()==_cancel) {
            setVisible(false);
        }
    }

    public OpensslSignDevice popup() {
        this.setVisible(true);
        return device;
    }
}

public class Helper {
    private static String[] imgfilter = {".png", ".bmp", ".gif", ".jpg"};
    private static String[] pfxfilter = {".pfx"};
    private static String[] pdffilter = {".pdf", ".xps", ".jpg", ".jpeg", ".png", ".epub", ".cbz", ".cbr"};

    private static JFileChooser createFileChooser(final String[] filter) {
        JFileChooser fileChooser = new JFileChooser();
        fileChooser.setDialogTitle("Choose a file to open");
        fileChooser.setFileFilter(new FileFilter() {
            public String getDescription() {
                String s = "Supported files(";
                for (String one : filter) {
                    s += "*" + one + ",";
                }
                s = s.replace(filter[filter.length - 1] + ",", filter[filter.length - 1] + ")");
                return s;
            }
            public boolean accept(File f) {
                if (f.isDirectory())
                    return true;
                String filename = f.getName().toLowerCase();
                for (String one:filter) {
                    if(filename.endsWith(one))
                        return true;
                }
                return false;
            }
        });
        return fileChooser;
    }

    private static String fileSelect(Frame owner, String[] filter, String defaultpath) {
        JFileChooser fileChooser = createFileChooser(filter);
        fileChooser.setCurrentDirectory(new File(defaultpath));
        while (true) {
            try {
                int result = fileChooser.showOpenDialog(owner);
                if (result == JFileChooser.APPROVE_OPTION) {
                    File selectedFile = fileChooser.getSelectedFile();
                    if (selectedFile != null)
                        return selectedFile.getPath();
                    else {
                        infoBox("Selected file not found.","Error");
                    }
                }
                else {
                    infoBox("File selection cancelled.","Error");
                    return null;
                }
            } catch (Exception e) {
                infoBox("Exception: " + e.getMessage(), "Error");
                break;
            }
        }

        return null;
    }

    public static String imageSelect(Frame owner, String defpath) {
        return fileSelect(owner, imgfilter, defpath);
    }

    public static String pfxSelect(Frame owner, String defpath) {
        return fileSelect(owner, pfxfilter, defpath);
    }

    public static String pdfSelect(Frame owner, String defpath) {
        return fileSelect(owner, pdffilter, defpath);
    }



    public static void infoBox(String infoMessage, String titleBar)
    {
        JOptionPane.showMessageDialog(null, infoMessage, "InfoBox: " + titleBar, JOptionPane.INFORMATION_MESSAGE);
    }

    public static OpensslSignDevice createOpensslDevice(String pfxfile, Frame owner) {
        PfxCreatorDialog dialog = new PfxCreatorDialog(owner, pfxfile);
        return dialog.popup();
    }
}
