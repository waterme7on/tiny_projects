
#3、初始化自定义背景图片
image = Image.open(r'D:\Python\test\wordcloud\data\backgroud\image1.png')
graph = np.array(image)

#4、产生词云图
#有自定义背景图：生成词云图由自定义背景图像素大小决定
wc = WordCloud(font_path=r"D:\Python\test\wordcloud\data\ttf\yahei.ttf",
               background_color='white', max_font_size=50, mask=graph)
wc.generate(result)

#5、绘制文字的颜色以背景图颜色为参考
image_color = ImageColorGenerator(graph)  # 从背景图片生成颜色值
wc.recolor(color_func=image_color)
# 按照背景图大小保存绘制好的词云图，比下面程序显示更清晰
wc.to_file(r"D:\Python\test\wordcloud\output\wordcloud.png")

# 6、显示图片
plt.figure("词云图")  # 指定所绘图名称
plt.imshow(wc)         # 以图片的形式显示词云
plt.axis("off")       # 关闭图像坐标系
plt.show()
