package sectionViewer;

import java.awt.*;
import javax.swing.*;

/**
 *   Associates a SectionViewer with JInternalFrame.
 */
public class SVFrameInt extends JInternalFrame {

   private final boolean _debug = false;

   /**   the SectionViewer that is associated with this SVFrameInt */
   protected SectionViewer _SV = null;
   JPanel _contentPane;

   /**   the initial height of the window. */
   int totalH = 500;
   /**   the initial width of the window. */
   int totalW = 400;
   int pad = 1;

   //=========================================================
   // constructor
   //=========================================================
   /**
    *   Constructs an SVFrameInt with the given title
    *   and resizable.
    *   @param viewstr
    */
   public SVFrameInt(String viewstr) {
      this(viewstr,true,false,false,false);
   }

   //---------------------------------------------
   /**
    *   Constructs an SVFrameInt with the given title
    *   and window controls.
    *   @param viewstr
    *   @param resizeable
    *   @param closeable
    *   @param maximizable
    *   @param iconifiable
    */
   public SVFrameInt(String viewstr,
                     boolean resizeable,
		     boolean closeable,
		     boolean maximizeable,
		     boolean iconifiable) {

      super(viewstr,
            resizeable,
	    closeable,
	    maximizeable,
	    iconifiable);

      if (_debug) System.out.println("enter SVFrameInt");

      try {
	 guiInit();
      }
      catch(Exception e) {
	 System.out.println("SVFrameInt");
	 e.printStackTrace();
      }

      if (_debug)
	 System.out.println("exit SVFrameInt");
   } // constructor

   //---------------------------------------------
   /**
    *   Builds the SVFrameInt.
    */
   private void guiInit() throws Exception {

      _contentPane = (JPanel)this.getContentPane();

      _contentPane.setLayout(new BorderLayout());
      _contentPane.setPreferredSize(new Dimension(totalW+pad, totalH+pad));
      _contentPane.setMinimumSize(new Dimension(totalW+pad, totalH+pad));
   }

   //---------------------------------------------
   /**
    *   Returns the SectionViewer for this SVFrameInt.
    */
   public SectionViewer getSectionViewer() {
      return _SV;
   }

   //---------------------------------------------
   /**
    *   Sets the SectionViewer for this SVFrameInt.
    *   @param sv the SectionViewer to be associated with this SVFrame.
    */
   public void setSectionViewer(SectionViewer sv) {
      _SV = sv;
      _contentPane.add(_SV, BorderLayout.CENTER);
   }

} // class SVExt
