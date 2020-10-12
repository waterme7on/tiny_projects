from django.shortcuts import render, redirect, get_object_or_404
from recognizer.models import *
from django import forms
from django.http import HttpResponse
from pyzbar import pyzbar
import cv2
import os
from django.conf import settings
from django import forms
from django.http import JsonResponse

class InformationForm(forms.Form):
    product = forms.CharField(max_length=100)
    location = forms.CharField(max_length=100)
    price = forms.CharField(max_length=100)
    notes = forms.CharField(max_length=100)
    date = forms.DateField()
    barcode = forms.FileField()

def scan(img):
    img = cv2.imread(img)
    barcodes = pyzbar.decode(img)
    result = ''
    for barcode in barcodes:
        barcodeData = barcode.data.decode("utf-8")
        # print(barcodeData)
        result = barcodeData
    return result


# Create your views here.
def test(request):
    # return render(request, )
    return HttpResponse("Test")




def index(request):
    if request.method == 'POST':
        barcode = request.FILES.get('barcode')

        accessory_dir = settings.MEDIA_URL
        if not os.path.isdir(accessory_dir):
            os.mkdir(accessory_dir)
        barcode_file = "%s/tmp.png" % (accessory_dir)
        recv_size = 0
        with open(barcode_file, 'wb') as new_file:
            for chunk in barcode.chunks():
                new_file.write(chunk)
        barcode = (scan(barcode_file))
        os.remove(barcode_file)
        product = (request.POST.get('product'))
        location = (request.POST.get('location'))
        price = (request.POST.get('price'))
        notes = (request.POST.get('notes'))
        date = (request.POST.get('date'))
        msg='成功！'
        try:
            Barcode(product=product, location=location, price=price, note=notes, date=date, barcode=barcode).save()
        except:
            msg='失败！'
        print(msg)
        # response = {'msg': 'ok'}
        return HttpResponse(msg)
    else:
        return render(request, 'index.html')
