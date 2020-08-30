from cnocr import CnOcr
from PIL import Image, ImageEnhance, ImageDraw, ImageFilter, ImageOps
import cv2
import numpy as np
# import aircv
import os
# import easyocr


# # import matplotlib.pyplot as plt
# # from matplotlib.gridspec import GridSpec

# # from skimage import data, transform, exposure
# # from skimage.util import compare_images

# need to run only once to load model into memory
# reader = easyocr.Reader(['ch_sim', 'en'])



# def matchImg(imgsrc, imgobj, confidence=0.25):
#     """
#         图片对比识别imgobj在imgsrc上的相对位置（批量识别统一图片中需要的部分）
#     :param imgsrc: 原始图片路径(str)
#     :param imgobj: 待查找图片路径（模板）(str)
#     :param confidence: 识别度（0<confidence<1.0）
#     :return: None or dict({'confidence': 相似度(float), 'rectangle': 原始图片上的矩形坐标(tuple), 'result': 中心坐标(tuple)})
#     """
#     imsrc = aircv.imread(imgsrc)
#     imobj = aircv.imread(imgobj)
#     match_result = aircv.find_template(imsrc, imobj, confidence)  # {'confidence': 0.5435812473297119, 'rectangle': ((394, 384), (394, 416), (450, 384), (450, 416)), 'result': (422.0, 400.0)}
#     if match_result is not None:
#         match_result['shape'] = (imsrc.shape[1], imsrc.shape[0])  # 0为高，1为宽

#     return match_result

# def cutImg(imgsrc, out_img_name, coordinate):
#     """
#     根据坐标位置剪切图片
#     :param imgsrc: 原始图片路径(str)
#     :param out_img_name: 剪切输出图片路径(str)
#     :param coordinate: 原始图片上的坐标(tuple) egg:(x, y, w, h) ---> x,y为矩形左上角坐标, w,h为右下角坐标
#     :return:
#     """
#     image = Image.open(imgsrc)
#     region = image.crop(coordinate)
#     region = ImageEnhance.Contrast(region).enhance(1.5)
#     region.save(out_img_name)

# def get_rec_areas(tem_dir, tem_image_path):
#     '''
#     获取各个需要识别的区域
#     '''
#     recognization_areas = []
#     tem_files = os.listdir(tem_dir)
#     for file in tem_files:
#         # png后缀为识别区域的截图
#         if os.path.splitext(file)[-1] == '.png':
#             match_result = matchImg(tem_image_path, os.path.join(tem_dir, file))
#             try:
#                 egg = (match_result['rectangle'][0][0], match_result['rectangle'][0][1], match_result['rectangle'][3][0], match_result['rectangle'][3][1])
#                 recognization_areas.append(egg)
#             except:
#                 pass
#     return recognization_areas


# 用白色块画出要识别的区域
def draw_rec_areas(tem_image_path, rec_areas, out_file_path):
    tem_image = Image.open(tem_image_path)
    draw_rec_ares = ImageDraw.Draw(tem_image)
    for rec_area in rec_areas:
        draw_rec_ares.rectangle(rec_area, fill='#ffffff')
    tem_image.save(out_file_path)


# 通过ocr库获取识别的结果
def get_ocr(template_image_path, recog_image_path, coordinate):
    tem_image = Image.open(template_image_path)
    rec_image = Image.open(recog_image_path)
    if tem_image.size != rec_image.size:
        rec_image = rec_image.resize(tem_image.size)
    rec_area = rec_image.crop(coordinate)
    # rec_area = rec_area.convert("L")
    # rec_area = rec_area.filter(ImageFilter.FIND_EDGES)
    # rec_area.save('tmp_rec_area_snip.jpg')
    # rec_area = Image.open('tmp_rec_area_snip.jpg')

    tmp_file_name = 'tmp_rec_area_snip.png'
    rec_area = rec_area.convert("L")
    # rec_area = rec_area.filter(ImageFilter.BLUR)
    rec_area = ImageOps.invert(rec_area)
    # rec_area.show()
    rec_area.save(tmp_file_name)

    # def get_table(threshold=115):
    #     table = []
    #     for i in range(256):
    #         if i < threshold:
    #             table.append(0)
    #         else:
    #             table.append(1)
    #     return table

    # # 打开灰度化图片并进行二值处理
    # binary_im = Image.open(tmp_file_name).point(get_table(140), "1")
    # # 保存二值化图片
    # binary_im.save(tmp_file_name)
    # binary_im.show()
    ocr = CnOcr(model_name='conv-lite-fc')
    res = ocr.ocr('tmp_rec_area_snip.png')
    try:
        os.remove(tmp_file_name)
    except:
        pass
    return res

    # result = reader.readtext(tmp_file_name, detail=0)
    # return result


# 获取识别的区域：识别模板中的红色色块
def get_recog_areas(tem_image_path):
    img = cv2.imread(tem_image_path)
    gs_frame = cv2.GaussianBlur(img, (5, 5), 0)                   # 高斯模糊
    hsv = cv2.cvtColor(gs_frame, cv2.COLOR_BGR2HSV)                 # 转化成HSV图像
    erode_hsv = cv2.erode(hsv, None, iterations=2)                  # 腐蚀 粗的变细


    lower_red = np.array([170,50,50])
    upper_red = np.array([180,255,255])
    inRange_hsv = cv2.inRange(erode_hsv, lower_red, upper_red)
    contours = cv2.findContours(inRange_hsv.copy(), cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)[-2]

    # c = max(cnts, key=cv2.contourArea)
    # rect = cv2.minAreaRect(c)
    # box = cv2.boxPoints(rect)
    # print('box:', box)
    recs_areas = []
    for countour in contours:
        x1,y1,x2,y2=10000, 100000, -1, -1
        for point in countour:
            point = point[0]
            x1 = min(x1, point[0])
            y1 = min(y1, point[1])
            x2 = max(x2, point[0])
            y2 = max(y2, point[1])
        recs_areas.append((x1,y1,x2,y2))
    return recs_areas


# tem_image_path = "Snipaste_2020-08-20_10-35-40.png"
# get_recog_areas(tem_image_path)

# res = reader.readtext('template/test.jpg', detail=0)
# print(res)

# filename = 'template_test.png'
# filedir = 'template'
# testfile='test_image.png'
# try:
#     rec_areas = get_recog_areas(os.path.join(filedir, filename))
#     print(rec_areas)
#     for rec_area in rec_areas:
#         print(get_ocr(os.path.join(filedir, filename),
#                       os.path.join(filedir, testfile), rec_area))
# except Exception as E:
#     print(f'** Error {E} **')
#     pass


