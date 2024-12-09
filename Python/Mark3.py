import cv2
import numpy as np
from skimage.io import imread
from skimage.morphology import binary_opening, disk
import matplotlib.pyplot as plt

# DepthAI
# first, import all necessary modules
from pathlib import Path
import blobconverter
#import cv2
import depthai
#import numpy as np

def binary2png(binary):
    png = []
    x = [10000, -1]
    y = [10000, -1]
    for i in range(0, len(binary)):
        nueva_fila = []
        for j in range(0, len(binary[0])):
            if binary[i][j]:
                nueva_fila.append([255, 255, 255])
                if i < y[0]:
                    y[0] = i
                if i > y[1]:
                    y[1] = i
                if j < x[0]:
                    x[0] = j
                if j > x[1]:
                    x[1] = j
            else:
                nueva_fila.append([0, 0, 0])
        png.append(nueva_fila)
    centro_x = x[0] + int((x[1]-x[0])/2)
    centro_y = y[0] + int((y[1]-y[0])/2)
    ancho = 5
    largo = ancho
    y_inicial = max(0, centro_y-largo)
    y_final = min(len(png)-1, centro_y+largo)
    x_inicial = max(0, centro_x-ancho)
    x_final = min(len(png)-1, centro_x+ancho)
    for k in range(y_inicial, y_final):
        for l in range(x_inicial, x_final):
            png[k][l] = [0, 0, 255]

    return png

pipeline = depthai.Pipeline()

# First, we want the Color camera as the output
cam_rgb = pipeline.createColorCamera()
cam_rgb.setPreviewSize(600, 600)  # 300x300 will be the preview frame size, available as 'preview' output of the node
cam_rgb.setInterleaved(False)

detection_nn = pipeline.createMobileNetDetectionNetwork()
# Blob is the Neural Network file, compiled for MyriadX. It contains both the definition and weights of the model
# We're using a blobconverter tool to retreive the MobileNetSSD blob automatically from OpenVINO Model Zoo
detection_nn.setBlobPath(blobconverter.from_zoo(name='mobilenet-ssd', shaves=6))
# Next, we filter out the detections that are below a confidence threshold. Confidence can be anywhere between <0..1>
detection_nn.setConfidenceThreshold(0.5)

# XLinkOut is a "way out" from the device. Any data you want to transfer to host need to be send via XLink
xout_rgb = pipeline.createXLinkOut()
xout_rgb.setStreamName("rgb")

xout_nn = pipeline.createXLinkOut()
xout_nn.setStreamName("nn")

cam_rgb.preview.link(xout_rgb.input)
cam_rgb.preview.link(detection_nn.input)
detection_nn.out.link(xout_nn.input)

# Pipeline is now finished, and we need to find an available device to run our pipeline
# we are using context manager here that will dispose the device after we stop using it
with depthai.Device(pipeline) as device:
    # From this point, the Device will be in "running" mode and will start sending data via XLink

    # To consume the device results, we get two output queues from the device, with stream names we assigned earlier
    q_rgb = device.getOutputQueue("rgb")
    q_nn = device.getOutputQueue("nn")
    # Here, some of the default values are defined. Frame will be an image from "rgb" stream, detections will contain nn results
    frame = None
    detections = []

    # Since the detections returned by nn have values from <0..1> range, they need to be multiplied by frame width/height to
    # receive the actual position of the bounding box on the image
    """
    def frameNorm(frame, bbox):
        normVals = np.full(len(bbox), frame.shape[0])
        normVals[::2] = frame.shape[1]
        return (np.clip(np.array(bbox), 0, 1) * normVals).astype(int)

        cap = cv2.VideoCapture(0)
        # Check if the webcam is opened correctly
        if not cap.isOpened():
            raise IOError("Cannot open webcam")
    """

    while True:

        # we try to fetch the data from nn/rgb queues. tryGet will return either the data packet or None if there isn't any
        in_rgb = q_rgb.tryGet()
        in_nn = q_nn.tryGet()

        if in_rgb is not None:
            # If the packet from RGB camera is present, we're retrieving the frame in OpenCV format using getCvFrame
            frame = in_rgb.getCvFrame()

        if in_nn is not None:
            # when data from nn is received, we take the detections array that contains mobilenet-ssd results
            detections = in_nn.detections


        if frame is not None:
            """
            for detection in detections:
                # for each bounding box, we first normalize it to match the frame size
                bbox = frameNorm(frame, (detection.xmin, detection.ymin, detection.xmax, detection.ymax))
                # and then draw a rectangle on the frame to show the actual result
                cv2.rectangle(frame, (bbox[0], bbox[1]), (bbox[2], bbox[3]), (255, 0, 0), 2)
            # After all the drawing is finished, we show the frame on the screen
            #cv2.imshow("preview", frame)
            """

            # at any time, you can press "q" and exit the main loop, therefore exiting the program itself
            if cv2.waitKey(1) == ord('q'):
                break
            #ret, frame = cap.read()
            frame = cv2.resize(frame, None, fx=0.6, fy=0.6, interpolation=cv2.INTER_AREA)
            #cv2.imshow('Input', frame)

            c = cv2.waitKey(1)
            #print(c)

            # Filtrado de camara
            #result = frame.copy()
            result = frame
            image = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)
            
            # lower boundary RED color range values; Hue (0 - 10)
            lower1 = np.array([0, 100, 20])
            upper1 = np.array([10, 255, 255])
            
            # upper boundary RED color range values; Hue (160 - 180)
            lower2 = np.array([160,100,20])
            upper2 = np.array([179,255,255])
            
            lower_mask = cv2.inRange(image, lower1, upper1)
            upper_mask = cv2.inRange(image, lower2, upper2)
            
            full_mask = lower_mask + upper_mask # El lower_mask es casi lo de adentro mientras que el upper_mask son los bordes.
            
            result = cv2.bitwise_and(result, result, mask=full_mask)

            #final_result = full_mask.copy()
            final_result = full_mask
            binary_filter = np.array(binary2png(binary_opening(final_result, disk(10))), dtype='uint8')
                
            #cv2.imshow('mask', full_mask)
            #cv2.imshow('result', result)
            cv2.imshow('final_result.png', binary_filter)

            if c == 27: #Tecla ESC
                break

# cap.release()
cv2.destroyAllWindows()

