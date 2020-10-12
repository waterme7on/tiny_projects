from pyzbar import pyzbar
import cv2

def scan(img):
    img = cv2.imread(img)
    barcodes = pyzbar.decode(img)
    result = ''
    for barcode in barcodes:
        barcodeData = barcode.data.decode("utf-8")
        # print(barcodeData)
        result = barcodeData
    return result
