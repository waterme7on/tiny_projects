from django.db import models

# Create your models here.

class Barcode(models.Model):
    product = models.CharField(max_length=100, verbose_name='商品')
    barcode = models.CharField(max_length=100, verbose_name='条形码')
    price = models.CharField(max_length=100, verbose_name='价格')
    date = models.DateField(verbose_name='时间')
    location = models.CharField(max_length=100, verbose_name='地点')
    note = models.CharField(max_length=100, verbose_name='备注')


    def __str__(self):
        return product