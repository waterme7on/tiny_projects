from face_register import *         # 人脸注册
from face_extractor import *        # 人脸特征提取
from face_recognizer import *       # 人脸验证

def main():
    import PySimpleGUI as sg
    # All the stuff inside your window.
    layout = [[sg.Text('    人脸识别签收系统    ')],
              [sg.Canvas(size=(400, 400))],
              [sg.Button('注册'), sg.Button('签收'), sg.Button('退出')],
              [sg.Text('')],
              [sg.Text('')]]
    # Create the Window
    window = sg.Window('Window Title', layout, default_element_size=(40, 1))
    # Event Loop to process "events" and get the "values" of the inputs
    while True:
        event, values = window.read()
        if event == sg.WIN_CLOSED or event == '退出':  # if user closes window or clicks cancel
            break
        elif event == '注册':
            Face_Register_con = Face_Register()
            Face_Register_con.run()
            extract_faces()
        elif event == '签收':
            Face_Recognizer_con = Face_Recognizer()
            Face_Recognizer_con.run()
    window.close()
    # while True:
    #     print("人脸识别系统")
    #     print("1:注册")
    #     print("2:签收")
    #     print("3:退出")
    #     k = input()
    #     if (k == "1"):
    #         Face_Register_con = Face_Register()
    #         Face_Register_con.run()
    #         extract_faces()
    #     elif (k == "2"):
    #         Face_Recognizer_con = Face_Recognizer()
    #         Face_Recognizer_con.run()
    #     else:
    #         break

if __name__ == '__main__':
    main()
