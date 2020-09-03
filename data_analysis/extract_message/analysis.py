
import codecs  # 打开ANSI格式的文档，需要codecs库
import os
# import pandas as pd

# keyword_file = 'keywords.txt'
# message_file_dir = 'message'

# 输出提示信息
print('当前所在位置：', os.getcwd())
print('输入的keyword文件每行一个关键字，空格个数、空行不影响结果，如：Last   time')
keyword_file = input('请输入keyword所在文件（相对路径）:')
print('message文件路径如：message/message1，输出到同文件目录下，格式为[文件名加后缀_result.txt]，如：message/message1_result.txt')
message_file_dir = input('请输入message所在文件夹（相对路径）:')

# 读取各个keyword
keywords = []
try:
    open(keyword_file, 'r')
except:
    print('错误的keyword路径，请重试')
    exit(-1)
with open(keyword_file, 'r') as f:
    for line in f.readlines():
        line = line.strip('\n| ').replace('"', '')
        if line == '':
            continue
        keyword = line.lower()
        keywords.append(keyword)
print('Keywords:', keywords)


def match_keyword(line, keywords):
    '''
    在行中寻找keyword，如果匹配上则返回keyword，无匹配项返回-1
    '''
    line = line.lower().replace(' ','')
    for keyword in keywords:
        if line.find(keyword.replace(' ', '')) != -1:
            return keyword
    return -1


def output_result(keyword_result, output_path):
    '''
    结果输出到文件中
    '''
    with open(output_path, 'w', encoding='utf8') as f:
        for keyword in keyword_result:
            f.write(
                '------------------------{}---------------------\n'.format(keyword))
            results = keyword_result[keyword]
            if len(results) == 0:
                f.write("Can't find matched result, keyword: {}\n".format(keyword))
            else:
                f.writelines(results)
#                 for line in keyword_result[keyword]:
#                     f.write(line)
            f.write('--------------------------------------------\n\n')

message_files = os.listdir(message_file_dir)
for message_file in message_files:
    if message_file.endswith('_result.txt') or not message_file.endswith('.txt'):
        continue
    keyword_result = {keyword: [] for keyword in keywords}
    message_file_path = os.path.join(message_file_dir, message_file)
    print('正在处理文件：',message_file_path)
    # f = codecs.open(message_file_path, 'r', encoding='ansi')
    f = open(message_file_path, 'r', encoding='ISO-8859-1')
    # f = codecs.open(message_file_path, 'r', encoding='utf-8')
    for line in f.readlines():
        matched_keyword = match_keyword(line, keywords)
        # 未匹配
        if matched_keyword == -1:
            continue
        keyword_result[matched_keyword].append(line)
    f.close()
    output_path = os.path.join(message_file_dir, message_file.strip('.txt') +'_result.txt')
    output_result(keyword_result, output_path)
    print('处理完成，输入到文件：',  output_path)
