t_move = 5              # 单位移动时间
t_fetch = 15            # 取、放时间
length = 30             # 默认槽长度
position1 = -1           # 1号机机库位置
position2 = length + 2  # 2号机机库位置
POS1 = [position1,]             # 存储1号机每次的最终位置
POS2 = [position2,]            # 存储2号机每次的最终位置
time = 0                # 计时
TIME = [0,]             # 每次操作花费时间


# 测试数据
AGV = [2, 1, 2, 1, 1, 1, 2, 1, 2, 2, 1, 2, 1]
Sta = [1, 2, 4, 7, 9, 10, 11, 14, 16, 17, 18, 19, 20]
Tar = [20, 3, 11, 8, 7, 9, 12, 7, 5, 8, 1, 12, 13]


class agv():
    def __init__(self, pos, tasks_list, c):
        super().__init__()
        self.pos = pos  # 当前位置
        self.tasks_list = tasks_list  # 任务列表
        # 例如：[(2, 3), (7, 8), (9, 7), (10, 9), (14, 7), (18, 1), (20, 13)] 每个元组为（start，target）
        self.curTask = None  # 当前执行任务
        self.status = 0  # 0 等待命令 1 取货 2 运送
        self.to = 0 # 前往的地方
        self.direction = 0 # 方向
        self.processTime = 0 # 撞击时间
        self.c = c # 仓库位置

    def selectTask(self):
        '''
        选择新任务执行，贪心策略
        '''
        # 无任务了，返回仓库
        if len(self.tasks_list) == 0:
            self.curTask = None
            self.status = 0
            self.to = self.c
            self.updateDirection()
            return
        # 有任务发，分配任务
        minTaskIdx = 0
        minTaskDist = 100000
        # 选择执行时间最短的任务
        for idx, task in enumerate(self.tasks_list):
            if abs(task[1] - task[0]) + abs(self.pos - task[0]) <= minTaskDist:
                minTaskDist = abs(task[1] - task[0]) + abs(self.pos - task[0])
                minTaskIdx = idx

        self.curTask = self.tasks_list[minTaskIdx]
        self.tasks_list.pop(minTaskIdx)
        self.status = 1
        self.to = self.curTask[0]
        self.updateDirection()

    def updateDirection(self):
        '''
        更新方向
        '''
        if self.to - self.pos > 0:
            self.direction = 1
        elif self.to - self.pos < 0:
            self.direction = -1
        else:
            self.direction = 0
    def updateStatus(self):
        '''
        更新状态
        '''
        global t_fetch, t_move
        # 到达指定地点
        # 可能为装货卸货地点或者机库
        if self.to == self.pos:
            # 卸货或者装货
            if self.status == 1 or self.status == 2:
                # 到达装货卸货地点
                if self.processTime == 0:
                    self.processTime = t_fetch - t_move
                    self.direction = 0
                else:
                    self.processTime -= t_move
                    # 装货卸货完成
                    if self.processTime == 0:
                        # 装货完成则卸货
                        if self.status == 1:
                            self.to = self.curTask[1]
                            self.status = 2
                        # 卸货完成则等待命令
                        else:
                            self.status = 0
                            self.curTask = None
                            self.to = self.c
            # 到达机库
            else:
                pass
        # 否则运动即可
        else:
            # 有任务执行的状态
            self.pos += self.direction


def update(agv1, agv2, update_sec=t_move):
    '''
    更新agv状态，每次更新时间间隔为update_sec = t_move
    '''
    # 更新时间
    global time
    time += update_sec

    # 1
    # 如果等待命令且还有任务，则先分配任务
    if agv1.status == 0 and len(agv1.tasks_list):
        agv1.selectTask()
    if agv2.status == 0 and len(agv2.tasks_list):
        agv2.selectTask()

    # 2
    # 分配任务后开始更新两个agv状态
    # 相隔为1的情况
    avg_distance = abs(agv1.pos - agv2.pos)
    agv1.updateDirection()
    agv2.updateDirection()
    # 相隔较近的情况
    if avg_distance <= 3:
        # 相向而行的情况，需要暂时改变方向
        if agv1.direction != agv2.direction:
            # agv2装货卸货
            if agv2.status == 0 or agv2.pos == agv2.to:
                if abs((agv1.direction + agv1.pos) - agv2.pos) < 2:
                    agv1.direction = agv2.direction
            # agv1装货卸货
            elif agv1.status == 0 or agv1.pos == agv1.to:
                if abs((agv2.direction + agv2.pos) - agv1.pos) < 2:
                    agv2.direction = agv1.direction
            # 没有agv在装货卸货
            else:
                if agv1.to == agv1.c and agv1.to == agv1.pos:
                    pass
                else:
                    agv1.direction = agv2.direction
        # 同向而行可以直接走
        else:
            pass
    print('-------------------------------------------------------------')
    print('time: {}'.format(time))
    print('before')
    print('agv1 pos:{} status: {} to:{} task:{} process_time:{} direction:{}'.format(agv1.pos, agv1.status, agv1.to, agv1.curTask, agv1.processTime, agv1.direction))
    print('agv2 pos:{} status: {} to:{} task:{} process_time:{} direction:{}'.format(agv2.pos, agv2.status, agv2.to, agv2.curTask, agv2.processTime, agv2.direction))
    # 相隔大于2的情况，直接执行
    agv1.updateStatus()
    agv2.updateStatus()
    POS1.append(agv1.pos)
    POS2.append(agv2.pos)
    print('after')
    print('agv1 pos:{} status: {} to:{} task:{} process_time:{} direction:{}'.format(agv1.pos, agv1.status, agv1.to, agv1.curTask, agv1.processTime, agv1.direction))
    print('agv2 pos:{} status: {} to:{} task:{} process_time:{} direction:{}'.format(agv2.pos, agv2.status, agv2.to, agv2.curTask, agv2.processTime, agv2.direction))
    print('res_tasks')
    print('agv1 task lists:', agv1.tasks_list)
    print('agv2 task lists:', agv2.tasks_list)
    if (agv1.c < agv2.c) != (agv1.pos  < agv2.pos) or abs(agv1.pos - agv2.pos) < 2:
        print('error')




tasks = {1:[], 2:[]}
for i in range(len(AGV)):
    tasks[AGV[i]].append((Sta[i], Tar[i]))

print(tasks)
agv1 = agv(position1, tasks[1], position1)
agv2 = agv(position2, tasks[2], position2)

while len(agv1.tasks_list) or len(agv2.tasks_list) or agv1.curTask or agv2.curTask:
    update(agv1, agv2)

while agv1.pos != agv1.c or agv2.pos != agv2.c:
    update(agv1, agv2)

print('AGV1位置:', POS1)
print('AGV2位置:', POS2)
print('总时间:', time)
