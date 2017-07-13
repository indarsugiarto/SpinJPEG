#!/usr/bin/python
from PyQt4 import Qt, QtCore, QtGui
import sys
import argparse


class cViewerDlg(QtGui.QWidget):
    def __init__(self, ras_file, w, h, parent=None):
        #super(cViewerDlg, self).__init__(parent)
        QtGui.QWidget.__init__(self, parent)
        self.viewPort = QtGui.QGraphicsView(self)
        self.scene = QtGui.QGraphicsScene(self)
        self.viewPort.setScene(self.scene)

        """
        Catatan: untuk gambar yang ukurannya tidak genap, misal shuttle.ras (dari shuttle.jpg)
                 berukuran 669 x 1004, maka file ras yang dihasilkan bukan 669*1004*3, tetapi
                 670*1004*3
                 Nah, kalo ukurannya 479 x 639, maka hasilnya adalah 480*639*3
        """
        self.img = QtGui.QImage(w,h,QtGui.QImage.Format_RGB32)
        #load image
        x=0; y=0; # pixel position
        c=0
        sz=w*h*3
        with open(ras_file, "rb") as f:
            d = f.read()
        while c<sz:
            """
            Ini menarik, karena formatnya bukan RGB tapi BGR -> big endian!!!
            """
            b = ord(d[c]); g = ord(d[c+1]); r = ord(d[c+2]); c += 3;
            pxVal = QtGui.qRgb(r,g,b)
            #print "pxVal = {}".format(pxVal, '#04x')
            self.img.setPixel(x,y,pxVal)
            x += 1
            if x == w:
                y += 1
                x = 0
        self.px = QtGui.QPixmap.fromImage(self.img)
        self.scene.addPixmap(self.px)
        self.viewPort.update()
        self.show()
        self.setGeometry(self.x(),self.y(),w,h)

#---------------------------- end of class cViewerDlg -----------------------------------


if __name__=="__main__":
    parser = argparse.ArgumentParser(description='Show RAS-without-header colorful image')
    parser.add_argument("ras_file", type=str, help="Input file (.ras)")
    parser.add_argument("--Width","-W", type=int, help="Image width")
    parser.add_argument("--Height","-H", type=int, help="Image height")
    args = parser.parse_args()

    if args.ras_file is None:
        print "Give the .ras file as input!"
        sys.exit(1)
    if args.Width is None or args.Height is None:
        print "Specify image size!"
        sys.exit(1)
    w = int(args.Width)
    h = int(args.Height)
    # the following behaves similar to ceil_div() in the aplx c-version
    if w % 2 !=0:
        w += 1

    app = Qt.QApplication(sys.argv)
    gui = cViewerDlg(args.ras_file, w, h)
    gui.show()
    sys.exit(app.exec_())


