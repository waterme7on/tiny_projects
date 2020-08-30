import PySimpleGUI as sg
import os.path
import PIL.Image
from PIL import ImageDraw, ImageFont
import io
import base64
import time
import os
from ocr import *

def convert_to_bytes(file_or_bytes, resize=None):
    '''
    Will convert into bytes and optionally resize an image that is a file or a base64 bytes object.
    Turns into  PNG format in the process so that can be displayed by tkinter
    :param file_or_bytes: either a string filename or a bytes base64 image object
    :type file_or_bytes:  (Union[str, bytes])
    :param resize:  optional new size
    :type resize: (Tuple[int, int] or None)
    :return: (bytes) a byte-string object
    :rtype: (bytes)
    '''
    if isinstance(file_or_bytes, str):
        img = PIL.Image.open(file_or_bytes)
    else:
        try:
            img = PIL.Image.open(io.BytesIO(base64.b64decode(file_or_bytes)))
        except Exception as e:
            dataBytesIO = io.BytesIO(file_or_bytes)
            img = PIL.Image.open(dataBytesIO)

    cur_width, cur_height = img.size
    if resize:
        new_width, new_height = resize
        scale = min(new_height/cur_height, new_width/cur_width)
        img = img.resize((int(cur_width*scale), int(cur_height*scale)), PIL.Image.ANTIALIAS)
    bio = io.BytesIO()
    img.save(bio, format="PNG")
    del img
    return bio.getvalue()



menu_def = [['File', ['重启', '退出']]]

left_col = [[sg.Text('Folder'), sg.In(size=(25,1), enable_events=True ,key='-FOLDER-'), sg.FolderBrowse()],
            [sg.Listbox(values=[], enable_events=True, size=(40,20),key='-FILE LIST-')]]
# For now will only show the name of the file that was chosen
images_col = [[sg.Text('待识别文件')],
              [sg.Button('识别', key='-BRECOG-')],
              [sg.Text(size=(40,1), key='-TOUT-')],
              [sg.Image(key='-IMAGE-')]]

template_col = [[sg.Text('模板文件')],
                [sg.FileBrowse('上传', key='-TUPLOAD-', enable_events=True)],
                [sg.Text(size=(40, 1), key='-TEMPLATE-')],
                [sg.Image(key='-TIMAGE-')]]

# ----- Full layout -----
layout = [[sg.Menu(menu_def, key='-MENU-')],
          [sg.Column(left_col, element_justification='c'), sg.VSeperator(),sg.Column(images_col, element_justification='c'), sg.VSeparator(), sg.Column(template_col, element_justification='c')]]

# --------------------------------- Create Window ---------------------------------
window = sg.Window('OCR', layout,resizable=True)


rec_areas = []
tmp_template_file = 'tmp_tem.jpg'
tmp_template_file_show = 'tmp_tem_show.jpg'
# ----- Run the Event Loop -----
# --------------------------------- Event Loop ---------------------------------
while True:
    event, values = window.read()
    if event in (sg.WIN_CLOSED, '退出'):
        break

    if event in ('重启'):
        window.hide()
        time.sleep(0.5)
        window.un_hide()

    if event == '-FOLDER-':                         # Folder name was filled in, make a list of files in the folder
        folder = values['-FOLDER-']
        try:
            file_list = os.listdir(folder)         # get list of files in folder
        except:
            file_list = []
        fnames = [f for f in file_list if os.path.isfile(
            os.path.join(folder, f)) and f.lower().endswith((".png", ".jpg", "jpeg", ".tiff", ".bmp"))]
        window['-FILE LIST-'].update(fnames)
    elif event == '-FILE LIST-':    # A file was chosen from the listbox
        try:
            filename = os.path.join(values['-FOLDER-'], values['-FILE LIST-'][0])
            window['-TOUT-'].update(filename)
            window['-IMAGE-'].update(data=convert_to_bytes(filename,resize=(800, 800)))
        except Exception as E:
            print(f'** Error {E} **')
    elif event == '-TUPLOAD-':
        filename = values['-TUPLOAD-']
        filedir = os.path.dirname(filename)
        tmp_template_file = os.path.join(filedir, tmp_template_file)
        try:
            rec_areas = get_recog_areas(os.path.join(filedir, filename))
            draw_rec_areas(filename, rec_areas, tmp_template_file)
            window['-TIMAGE-'].update(data=convert_to_bytes(tmp_template_file, resize=(800, 800)))
        except Exception as E:
            print(f'** Error {E} **')
    elif event=='-BRECOG-':
        # 识别模板中标识的各个段
        try:
            print("Start to recognize")
            tem_image_path = values['-TUPLOAD-']
            rec_image_path = os.path.join(values['-FOLDER-'], values['-FILE LIST-'][0])

            tmp_tem_image = PIL.Image.open(tmp_template_file)
            draw_text = ImageDraw.Draw(tmp_tem_image)
            fnt = ImageFont.truetype('msyh.ttf', size=10)
            for rec_area in rec_areas:
                try:
                    result = get_ocr(tem_image_path, rec_image_path, rec_area)
                    rec_text = ''.join(result[0])
                    print(rec_text)
                    draw_text.text((rec_area[0], rec_area[1]), rec_text,fill='#000000', font=fnt)
                except Exception as e:
                    print(e)
            tmp_tem_image.save(tmp_template_file_show)
            window['-TIMAGE-'].update(data=convert_to_bytes(
                tmp_template_file_show, resize=(800, 800)))

        except Exception as e:
            print(e)

try:
    os.remove(tmp_template_file)
    os.remove(tmp_template_file_show)
except:
    pass
window.close()
