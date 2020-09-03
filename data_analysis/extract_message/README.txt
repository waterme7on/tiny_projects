格式要求：
1. message文件：要求message为".txt"格式文件，UTF-8编码（编码方式可以通过记事本打开，另存为的时候选择UTF-8编码），参考：https://jingyan.baidu.com/article/b0b63dbfe9aae74a483070d8.html
2. keyword：无具体要求

执行
1. python 运行analysis.py，python
2. 根据提示输入keyword文件和message文件夹即可

示例：
    当前所在位置： C:\Users\t-jintli\Desktop\python_projects\data_analysis\outpu
    输入的keyword文件每行一个关键字，空格个数、空行不影响结果，如：Last   time
    请输入keyword所在文件（相对路径）:keywords.txt
    message文件路径如：message/message1，输出到同文件目录下，格式为[文件名加后缀_result.txt]，如：message/message1_result.txt
    请输入message所在文件夹（相对路径）:message
    Keywords: ['last   time', 'linux version', 'call trace', 'unknown product type or slotid', 'machine check', 'peer power on', 'peer power status']
    正在处理文件： message\messages1.txt
    处理完成，输入到文件： message\messages1_result.txt
    正在处理文件： message\messages2.txt
    处理完成，输入到文件： message\messages2_result.txt
    正在处理文件： message\messages3.txt
    处理完成，输入到文件： message\messages3_result.txt
    正在处理文件： message\messages4.txt
    处理完成，输入到文件： message\messages4_result.txt
    正在处理文件： message\messages重点.txt
    处理完成，输入到文件： message\messages重点_result.txt