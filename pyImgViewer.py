#!/usr/bin/python
from PyQt4 import Qt, QtCore, QtGui, QtNetwork
import sys
import struct
import argparse
import time
import os
from spinConfig import *


class imgWidget(QtGui.QWidget):
    def __init__(self, Title, parent=None):
        QtGui.QWidget.__init__(self, parent)
        self.setWindowTitle(Title)
        self.viewPort = QtGui.QGraphicsView(self)
        self.scene = QtGui.QGraphicsScene(self)
        self.viewPort.setScene(self.scene)

    def setPixmap(self, img):
        self.scene.addPixmap(img)
        self.viewPort.update()
        self.setGeometry(QtCore.QRect(self.x(),self.y(),img.width(),img.height()))
        self.show()
#---------------------------- end of class imgWidget -----------------------------------

class cViewerDlg(QtGui.QWidget):
    def __init__(self, SpiNN, parent=None):
        #super(cViewerDlg, self).__init__(parent)
        QtGui.QWidget.__init__(self, parent)
        self.setupGui()
        self.connect(self.pbLoadSend, QtCore.SIGNAL("clicked()"), QtCore.SLOT("pbLoadSendClicked()"))
        if SpiNN is None:
            print "[INFO] Will use default machine at", SPINN_HOST
            self.spinn = SPINN_HOST
        else:
            print "[INFO] Will use machine at", SpiNN
            self.spinn = SpiNN

        # prepare the sdp receiver
        self.setupUDP()

    def setupGui(self):
        self.setWindowTitle("Tester")
        #add a button
        self.pbLoadSend = QtGui.QPushButton(self)
        self.pbLoadSend.setGeometry(QtCore.QRect(0,0,92,29))
        self.pbLoadSend.setObjectName("pbLoadSend")
        self.pbLoadSend.setText("Load-n-Send")

    def closeEvent(self, e):
        if self.orgImgLoaded is not None:
            self.orgImgViewer.close()
        e.accept()

    def setupUDP(self):
        self.sdpRecv = QtNetwork.QUdpSocket(self)
        print "[INFO] Trying to open UDP port-{}...".format(SDP_SEND_RESULT_PORT),
        ok = self.sdpRecv.bind(SDP_SEND_RESULT_PORT)
        if ok:
            print "done!"
        else:
            print "fail!"
        self.sdpRecv.readyRead.connect(self.readSDP)

    @QtCore.pyqtSlot()
    def readSDP(self):
        """
        Think:
        1. how the image is stored in spinnaker memory? --> lihat efek big-endian di viewRAS_no_header.py
        2. how to send it?
           the aplx will send "empty" SDP as an EOF
        3. how to display it? see: viewRAS_no_header.py
        """
        #while self.sdpRecv.hasPendingDatagrams():
        ba = bytearray()
        sz = self.sdpRecv.pendingDatagramSize()
        # NOTE: readDatagram() will produce str, not bytearray
        datagram, host, port = self.sdpRecv.readDatagram(sz)
        ba += bytearray(datagram)
        if DEBUG_MODE > 0:
            print "Got sdp length {}-bytes".format(sz)
        # remove the first 10 bytes of SDP header
        print "len(ba) =",len(ba)
        del ba[0:10]
        print "after del, len(ba) =",len(ba)
        if len(ba) > 0:
            if DEBUG_MODE > 0:
                print "Append datagram length {}-bytes".format(len(ba))
            self.resultImg += ba
        else:
            if DEBUG_MODE > 0:
                print "Got EOF. Save to file!"
            # self.saveResult()
            self.resultName = self.fName.replace(".jpg",".ras")

            print "Resulting length of image data:", len(self.resultImg)

            # will save ras without header
            with open(self.resultName, "wb") as f:
                f.write(self.resultImg)
            #then tell aplx to close the buffer
            print "Sending close command to APLX..."
            #self.sendImgInfo(SDP_CMD_CLOSE_IMAGE, 0)


    @QtCore.pyqtSlot()
    def pbLoadSendClicked(self):
        if self.loadImg() is True:
            self.sendImg()

    def loadImg(self):
        self.fName = QtGui.QFileDialog.getOpenFileName(caption="Load image file", filter="JPEG Images (*.jpg *.JPG *.jpeg)")
        #self.fName = QtGui.QFileDialog.getOpenFileName(caption="Load image file", filter="*.*") 
        if self.fName=="" or self.fName is None:
            print "[INFO] Ups... cancelled!"
            self.fName = None
            return False
        else:
            print "[INFO] Loading image: ", self.fName
            self.orgImg = QtGui.QImage()
            self.orgImgLoaded = self.orgImg.load(self.fName)
            if self.orgImgLoaded:
                w = self.orgImg.width()
                h = self.orgImg.height()
                d = self.orgImg.depth()
                c = self.orgImg.colorCount()
                print "[INFO] Image info: w={}, h={}, d={}, c={}".format(w,h,d,c)
                pixmap = QtGui.QPixmap()
                pixmap.convertFromImage(self.orgImg)
                self.orgImgViewer = imgWidget("Original Image")
                self.orgImgViewer.setPixmap(pixmap)
                self.szImgFile = os.path.getsize(self.fName)
                return True
            else:
                print "[FAIL] Cannot load image file!"
                self.orgImgLoaded = None
                return False
        
    def sendImg(self):
        """
        Note: udp packet is composed of these segments:
              mac_hdr(14) + ip_hdr(20) + udp_hdr(8) + sdp + fcs(4)
              hence, udp_payload = 14+20+8+4 = 46 byte
        """
        print "[INFO] Sending image info to SpiNNaker...",
        self.sendImgInfo(SDP_CMD_INIT_SIZE, SDP_DATA_TYPE_IMAGE)
        print "done!"

        """
        ######################### IMPORTANT !!! ############################
        TODO: if spinjpeg needs to re-create sdram buffer (because the new image
              is bigger), then we need to introduce another delay here
        """

        print "[INFO] Sending image data to SpiNNaker...",
        cntr = 0
        szData = 0
        tic_is_set = False
        with open(self.fName,"rb") as f:
            chunk = f.read(272) #272 = 256(data) + 16(SCP)
            while chunk != "":
                ba = bytearray(chunk)
                lba = len(ba) #should be SCP+data only, which is 272 max
                if lba > 0:
                    if tic_is_set is False:
                        tic = time.time()
                        tic_is_set = True
                    self.sendChunk(ba)
                    cntr += 1
                    szData += (lba + 46 + 8) #consider udp_payload and sdp_hdr
                chunk = f.read(272)
            self.sendChunk(None) # end of image transmission
            toc = time.time()
            szData += (10+46) # sdp_hdr + udp_payload
        print "done in {} chunks!".format(cntr)
        #calculate bandwidth usage
        elapse_sec = toc - tic
        bw_KB = (szData/1024) / elapse_sec
        print "[INFO] Image file size =", self.szImgFile, "Byte = ", self.szImgFile/1024, "KB"
        print "[INFO] Total size of UDP packets =", szData/1024, "KB"
        print "[INFO] Elapsed time in sec = ", elapse_sec
        print "[INFO] Total (max) bandwidth usage = {} KBps".format(bw_KB)

        #then prepare the container for the result:
        self.resultImg = bytearray()

    def sendImgInfo(self, cmd, data_type):
        dpc = (SDP_RECV_JPG_INFO_PORT << 5) + SDP_RECV_CORE_ID #destination port and core
        pad = struct.pack('2B',0,0)
        hdr = struct.pack('4B2H',7,0,dpc,255,0,0)
        scp = struct.pack('2H3I',cmd,data_type,self.szImgFile,0,0)
        sdp = pad + hdr + scp
        CmdSock = QtNetwork.QUdpSocket()
        CmdSock.writeDatagram(sdp, QtNetwork.QHostAddress(self.spinn), SPINN_PORT)
        CmdSock.flush()
        # then give a break, otherwise spinnaker will collapse
        self.delay()

    def sendChunk(self, chunk):
        # based on sendSDP()
        # will be sent to chip <0,0>, no SCP
        dpc = (SDP_RECV_JPG_DATA_PORT << 5) + SDP_RECV_CORE_ID #destination port and core
        pad = struct.pack('2B',0,0)
        hdr = struct.pack('4B2H',7,0,dpc,255,0,0)
        if chunk is not None:
            sdp = pad + hdr + chunk
        else:
            #empty sdp means end of image transmission
            sdp = pad + hdr
        CmdSock = QtNetwork.QUdpSocket()
        CmdSock.writeDatagram(sdp, QtNetwork.QHostAddress(self.spinn), SPINN_PORT)
        CmdSock.flush()
        # then give a break, otherwise spinnaker will collapse
        self.delay()

    def delay(self):
        if DELAY_IS_ON:
            time.sleep(DELAY_SEC)
#---------------------------- end of class cViewerDlg -----------------------------------


if __name__=="__main__":
    parser = argparse.ArgumentParser(description='Encoding/decoding JPG in SpiNNaker')
    parser.add_argument("-ip", type=str, help="ip address of SpiNNaker")
    args = parser.parse_args()

    app = Qt.QApplication(sys.argv)
    gui = cViewerDlg(args.ip)
    gui.show()
    sys.exit(app.exec_())

