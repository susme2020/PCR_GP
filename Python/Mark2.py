import cv2
import numpy as np
from skimage.io import imread
from skimage.morphology import binary_opening, disk
import matplotlib.pyplot as plt

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


cap = cv2.VideoCapture(0)

# Check if the webcam is opened correctly
if not cap.isOpened():
    raise IOError("Cannot open webcam")

while True:
    ret, frame = cap.read()
    frame = cv2.resize(frame, None, fx=0.6, fy=0.6, interpolation=cv2.INTER_AREA)
    cv2.imshow('Input', frame)

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

cap.release()
cv2.destroyAllWindows()

