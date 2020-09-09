'''
AGV 串行作业 
'''
t_move = 5              # 单位移动时间
t_fetch = 15            # 取、放时间
length = 30             # 默认槽长度
position1 = 0           # 1号机机库位置
position2 = length + 1  # 2号机机库位置
POS1 = [0,]             # 存储1号机每次的最终位置
POS2 = [31,]            # 存储2号机每次的最终位置
time = 0                # 计时
TIME = [0,]             # 每次操作花费时间

# 示意图：

        #+++#                       #+++#                                    
        # 1 #                       # 2 # 
        #+++#                       #+++#
###################################################

# 轨道为1维，所以“1号机永远只能在2号机左侧”，若有碰撞情况，另一台机器必须退让
# 每一次运行，所花费的总时间为： （当前机器位置-物品位置） + （物品位置-目标位置） + （取、放时间）

def calculate(agv, start, target):
    global t_move, t_fetch, length, position1, position2, time
    # agv = 1, 则1号机运行
    # 不发生位置交叉的情况
    if agv == 1 and position2 - target >= 1:
        time = abs(start - position1)*t_move + abs(target - start)*t_move + 2*t_fetch
        position1 = target
    # 发生位置交叉情况
    elif agv == 1 and position2 - target < 1:
        time = abs(start - position1)*t_move + abs(target - start)*t_move + 2*t_fetch
        position1 = target
        # 另外一台机器（2号）退位到1号机器最终位置的“右侧”，保持1个槽的安全距离
        position2 = target + 1
    # agv = 2, 则2号机运行

    # 不发生位置交叉的情况
    elif agv == 2 and target - position1 >= 1:
        time = abs(start - position2)*t_move + abs(target - start)*t_move + 2*t_fetch
        position2 = target
    # 发生位置交叉的情况
    elif agv == 2 and target- position1 < 1:
        time = abs(start - position2)*t_move + abs(target - start)*t_move + 2*t_fetch
        position2 = target
        # 另外一台机器（1号）退位到2号机器最终位置的“左侧”，保持1个槽的安全距离
        position1 = target - 1
    POS1.append(position1)
    POS2.append(position2)
    TIME.append(time)
    return 

# 测试
AGV = [2,1,2,1,1,1,2,1,2,2,1,2,1]
Sta = [1,2,4,7,9,10,11,14,16,17,18,19,20]
Tar = [20,3,11,8,7,9,12,7,5,8,1,12,13]

for i in range(len(AGV)):
    agv = AGV[i]
    start = Sta[i]
    target = Tar[i]
    calculate(agv, start, target)

print('AGV1位置:', POS1, '\n')
print('AGV2位置:', POS2, '\n')
print('时间/次:', TIME, '\n')
print('总时间:', sum(TIME),'\n')

    
