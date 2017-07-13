# -------- SpiNNaker-related parameters --------

# NOTE: This must compatible with SpinJPEG.h

#-- port and cmd_rc
SDP_RECV_JPG_DATA_PORT = 1
SDP_RECV_JPG_INFO_PORT = 2
SDP_RECV_RAW_DATA_PORT = 3
SDP_RECV_RAW_INFO_PORT = 4
SDP_CMD_INIT_SIZE = 1
SDP_CMD_CLOSE_IMAGE = 2 #only for video processing, not image

# if aplx detect SDP_DATA_TYPE_IMAGE, it will close sdramImgBuf right after decoding it
# but if SDP_DATA_TYPE_VIDEO is received, it will not close sdramImgBuf until it
# receives SDP_CMD_CLOSE_IMAGE
SDP_DATA_TYPE_IMAGE = 1
SDP_DATA_TYPE_VIDEO = 2

SDP_SEND_RESULT_PORT = 30000
SDP_RECV_CORE_ID = 2
DEC_MASTER_CORE = SDP_RECV_CORE_ID
ENC_MASTER_CORE = 10
SPINN_HOST = '192.168.240.253'
SPINN_PORT = 17893
DEBUG_MODE = 1
DELAY_IS_ON = True
DELAY_SEC = 0.1  # useful for debugging
#DELAY_SEC = 0.0001  # for release mode