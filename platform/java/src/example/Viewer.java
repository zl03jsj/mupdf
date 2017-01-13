package example;

import com.artifex.mupdf.fitz.*;

import java.awt.*;
import java.awt.event.*;

public class Viewer extends Frame implements WindowListener,ActionListener
{
	protected Document doc;
	protected Panel toolbar;
	protected PageCanvas pageCanvas;
	protected Label pageLabel;
	protected Button firstButton, prevButton, nextButton, lastButton, saveButton, handsignbutton, signbutton, signnextbutton, signcancelbutton;
	protected int pageCount;
	protected int pageNumber;
	protected Canvas imageCanvas;

	public Viewer(Document doc_) {
		super("MuPDF");

		this.doc = doc_;

		pageCount = doc.countPages();
		pageNumber = 0;

		setSize(600, 800);
		setTitle("MuPDF: " + doc.getMetaData(Document.META_INFO_TITLE));

        Panel upPanel = new Panel(new BorderLayout(0, 0));
		toolbar = new Panel();
		toolbar.setLayout(new FlowLayout(FlowLayout.LEFT));
		firstButton = new Button("|<");
		firstButton.addActionListener(this);
		prevButton = new Button("<");
		prevButton.addActionListener(this);
		nextButton = new Button(">");
		nextButton.addActionListener(this);
		lastButton = new Button(">|");
		lastButton.addActionListener(this);
		saveButton = new Button(">|");
		saveButton.addActionListener(this);

		pageLabel = new Label();

		toolbar.add(firstButton);
		toolbar.add(prevButton);
		toolbar.add(nextButton);
		toolbar.add(lastButton);
		toolbar.add(saveButton);
		toolbar.add(pageLabel);
		upPanel.add(toolbar, BorderLayout.NORTH);

        Panel signbar = new Panel(new FlowLayout(FlowLayout.LEFT));
		handsignbutton = new Button("hand sign");
		handsignbutton.addActionListener(this);
		signbutton = new Button("sign");
		signbutton.addActionListener(this);
		signnextbutton = new Button("next");
		signnextbutton.addActionListener(this);
		signcancelbutton = new Button("canncel");
		signcancelbutton.addActionListener(this);
		signbar.add(handsignbutton);
		signbar.add(signbutton);
        signbar.add(signnextbutton);
		signbar.add(signcancelbutton);
		upPanel.add(signbar, BorderLayout.CENTER);

		add(upPanel, BorderLayout.NORTH);

		initSignCammondStatus();
		addWindowListener(this);
		stuff();
	}

	public void stuff() {
		pageLabel.setText("Page " + (pageNumber + 1) + " / " + pageCount);
		if (pageCanvas != null)
			remove(pageCanvas);
		pageCanvas = new PageCanvas(doc.loadPage(pageNumber));

		add(pageCanvas, BorderLayout.CENTER);
		validate();
	}

	public void onHandsign() {
		handsignbutton.setLabel("cancel");
	}

	public void onSign() {
        boolean isok = pageCanvas.beginSign();
        signbutton.setEnabled(!isok);
		signnextbutton.setEnabled(isok);
        signcancelbutton.setEnabled(isok);
		handsignbutton.setEnabled(!isok);
	}

	public void signNext() {
		boolean isok = pageCanvas.doSign(doc);
		signnextbutton.setEnabled(!isok);
		signcancelbutton.setEnabled(true);
		if( isok ){
			cancelSign(isok);
		}
	}

	public void pageCanvaseResize(Rectangle bound) {
		if(imageCanvas==null)
			return;

		imageCanvas.setBounds(bound);
	}

	public void actionPerformed(ActionEvent event) {
		Object source = event.getSource();
		int oldPageNumber = pageNumber;

		if (source == firstButton)
			pageNumber = 0;
		if (source == lastButton)
			pageNumber = pageCount - 1;
		if (source == prevButton) {
			pageNumber = pageNumber - 1;
			if (pageNumber < 0)
				pageNumber = 0;
		}

		if (source == nextButton) {
			pageNumber = pageNumber + 1;
			if (pageNumber >= pageCount)
				pageNumber = pageCount - 1;
		}

		if(source == saveButton) {
			// TODO: select save file path
			// doc.save();
		}

		if(source == handsignbutton)
			onHandsign();

		if(source == signbutton)
			onSign();

        if(source == signnextbutton)
        	signNext();

		if(source == signnextbutton)
			cancelSign(false);

		if (pageNumber != oldPageNumber)
			stuff();

	}

	private void cancelSign(boolean update) {
		pageCanvas.cancelSign();
		initSignCammondStatus();
		if(update) {
            pageCanvas.updatePageImage();
//            remove(pageCanvas);
//			pageCanvas = null;
//			stuff();
		}
	}

	private void initSignCammondStatus() {
		signbutton.setEnabled(true);
		handsignbutton.setEnabled(true);
		signnextbutton.setEnabled(false);
		signcancelbutton.setEnabled(false);
	}

	public void windowClosing(WindowEvent event) {
		System.exit(0);
	}

	public void windowActivated(WindowEvent event) { }
	public void windowDeactivated(WindowEvent event) { }
	public void windowIconified(WindowEvent event) { }
	public void windowDeiconified(WindowEvent event) { }
	public void windowOpened(WindowEvent event) { }
	public void windowClosed(WindowEvent event) { }

	public static void main(String[] args) {
		while (true) {
			String filepath = "/Users/zl03jsj/Documents/pdftest/pdffile/PDF32000_2008.PDF";// Helper.fileSelect(imgfilter);
            if(filepath==null) continue;
			try {
				Document doc = new Document(filepath);
				Viewer app = new Viewer(doc);
				app.setVisible(true);
                return;
			}
			catch (Exception e)
			{
				Helper.infoBox("Exception: "+e.getMessage(),"Error");
			}
		}
	}
}
