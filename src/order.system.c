//
// Created by shaobin on 2026/7/13.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "message.h"


/*定义常量*/
#define MAX_FLIGHT_NO 1000//航班号最大字节符
#define MAX_CITY 20//城市名字的最大长度
#define MAX_NAME 20//名字的最大长度
#define MAX_ID 20//身份证号的最大长度
#define MAX_ORDER_ID 20//订单编号长度
#define MAX_LINE 512//读取文件时单行缓冲区大小


/*写订单的结构*/
typedef struct {
    char orderId[MAX_ORDER_ID];//订单号
    char passengerName[MAX_NAME];//乘客姓名
    char idNumber[MAX_ID];//乘客身份证号
    char flightNo[MAX_FLIGHT_NO];//绑定的航班号，关联Flight结构体
    int status;//订单状态码
} Order;


int reserve_seat(Manager *manager, const char *flight_no) {
    Flight *flight = get_flight_by_number(manager,flight_no);
    if (flight == NULL) {
        printf("错误：未找到航班 %s\n", flight_no);
        return false;
    }

    flight->remaining_seats --;
    get_current_time(flight->last_updated,sizeof(flight->last_updated));
    manager->is_modified = true;
    printf("成功订票\n");
    return true;
}

/* 退票 */
int cancel_seat(Manager *manager,const char *flight_no) {
    Flight *flight = get_flight_by_number(manager,flight_no);
    if (flight == NULL) {
        printf("错误：未找到航班 %s\n", flight_no);
        return false;
    }

    flight->remaining_seats ++;
    get_current_time(flight->last_updated,sizeof(flight->last_updated));
    manager->is_modified = true;
    printf("成功退票\n");
    return true;
}
/* ===================== 订单管理器（人员B核心模块） ===================== */
/*定义哈希表的结构*/

//订单号哈希链表节点
typedef struct OrderIdxNode {
    char key[MAX_ORDER_ID];//key:订单号
    int value;//在动态数组的下标
    struct OrderIdxNode *next;//哈希冲突时，拉链法穿起多个节点
}OrderIdxNode;

/*运用哈希表，用证件号来倒排索引*/
typedef struct IdListNode {
    int orderIndex;//订单的下标
    struct IdListNode *next;
}IdListNode;

typedef struct IdHashNode {
    char key[MAX_ID];//证件号
    IdListNode *orderList;//订单下表链表
    struct IdHashNode *next;//哈希冲突链
}IdHashNode;
/*用动态数组进行主储存*/

typedef struct {
    Order *data;//指向Order数组的指针，存完整订单信息
    int capacity;//数组总容量，最多能存多少订单
    int size;//当前已存入有效订单数量
}OrderArray;

typedef struct {
    OrderArray orders;//真实订单动态数组
    OrderIdxNode **orderHash;//订单号哈希表，运用桶数组，二级指针
    int hashSize;//两张哈希表，统一桶数组
    IdHashNode **idHash;//身份证哈希表，桶数组
}OrderManager;


/*哈希函数*/
static unsigned int hash_str(const char *str,int table_size) {
    unsigned int hash=0;
    while (*str) hash = (hash << 5) + (*str++);//等价于hash=hash*31+当前字符ASCII
    return hash%table_size;//将hash取余，对应哈希桶数组下标
}


static int add_order(OrderManager *mgr,const Order *ord) {
    if (mgr->orders.size>=mgr->orders.capacity) {//容量判断，size>=capacity扩容
        int newCapacity=mgr->orders.capacity==0?16:mgr->orders.capacity*2;//初始无容量分配16个，有容量，容量翻倍
        Order *newData=(Order*)realloc(mgr->orders.data,newCapacity*sizeof(Order));
        if (!newData) return -1;//失败返回-1
        mgr->orders.data=newData;
        mgr->orders.capacity=newCapacity;
    }
    mgr->orders.data[mgr->orders.size]=*ord;//将传入订单拷贝到数组末尾
    return mgr->orders.size++;//返回当前订单的数组下标，同时size++记录有效订单总数
}

static void insert_order_hash(OrderManager *mgr, const char *orderId, int idx) {
    unsigned int slot = hash_str(orderId, mgr->hashSize);//用订单号计算出哈希桶位置
    OrderIdxNode *node = (OrderIdxNode*)malloc(sizeof(OrderIdxNode));//分配一块链表节点内存
    strcpy(node->key, orderId);//把订单号存进节点
    node->value = idx;//节点记录对应数组下标
    node->next = mgr->orderHash[slot];//新节点指向原有链表头
    mgr->orderHash[slot] = node;//桶头更新为新节点
}

static int find_order_by_id(OrderManager *mgr,const char *orderId) {
    unsigned int slot=hash_str(orderId,mgr->hashSize);//计算该订单号的哈希桶
    OrderIdxNode *p=mgr->orderHash[slot];//指针p指向桶链表第一个节点
    while (p) {
        if (strcmp(orderId,p->key)==0) return p->value;//单号匹配，直接返回订单下标
        p=p->next;//不匹配，一到下一个节点
    }
    return -1;
}
/*删除订单号索引*/
static void insert_id_index(OrderManager *mgr,const char *idNumber,int idx) {
    unsigned int slot=hash_str(idNumber,mgr->hashSize);//
    IdHashNode *p=mgr->idHash[slot];
    while (p) {
        if (strcmp(p->key,idNumber)==0) {
            IdListNode *node=(IdListNode*)malloc(sizeof(IdListNode));
            node->orderIndex= idx;
            node->next=p->orderList;
            p->orderList=node;
            return;
        }
        p=p->next;
    }
    //未找到证件号，创建新节点
    IdHashNode *newNode = (IdHashNode*)malloc(sizeof(IdHashNode));
    strcpy(newNode->key,idNumber);
    newNode->orderList=NULL;
    newNode->next=mgr->idHash[slot];
    mgr->idHash[slot]=newNode;
    //插入订单
    IdListNode *node=(IdListNode*)malloc(sizeof(IdListNode));
    node->orderIndex=idx;
    node->next=newNode->orderList;
    newNode->orderList=node;
}
/*订单文件持久化*/
static int load_orders_from_file(OrderManager *mgr) {
    int fd=open("orders.dat",O_RDONLY);
    if (fd<0) {
        //文件不存在，初始化空数据
        return 0;
    }
    char buf[1024];
    char line[MAX_LINE];
    ssize_t n;
    int lineLen=0;
    while ((n=read(fd,buf,sizeof(buf)-1))>0) {
        buf[n]='\0';
        for (int i=0;i<n;i++) {
            if (buf[i]=='\n') {
                line[lineLen]='\0';
                //解析这一行
                Order ord;
                char *token=strtok(line,"|");
                if (!token) { lineLen=0;
                continue;
            }
            strcpy(ord.orderId,token);
            token=strtok(NULL,"|");
            if (!token) continue;
            strcpy(ord.passengerName,token);
            token=strtok(NULL,"|");
            if (!token) continue;
            strcpy(ord.idNumber,token);
            token=strtok(NULL,"|");
            if (!token) continue;
            strcpy(ord.flightNo,token);
            token=strtok(NULL,"|");
            if (!token) continue;
            ord.status = atoi(token);
            int idx=add_order(mgr,&ord);
            if (idx>=0) {
                insert_order_hash(mgr,ord.orderId,idx);
                insert_id_index(mgr,ord.idNumber,idx);
            }
            lineLen=0;
        }else {
            line[lineLen++]=buf[i];
            if (lineLen >= MAX_LINE) lineLen = 0;
        }
    }
}
close(fd);
return 0;
   }

static int save_orders_to_file(OrderManager *mgr) {
    int fd = open("orders.dat", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) return -1;
    for (int i = 0; i < mgr->orders.size; i++) {
        Order *o = &mgr->orders.data[i];
        char line[MAX_LINE];
        sprintf(line, "%s|%s|%s|%s|%d\n",
                o->orderId, o->passengerName, o->idNumber, o->flightNo, o->status);
        write(fd, line, strlen(line));
    }
    fsync(fd);
    close(fd);
    return 0;
}

/*生成唯一订单号*/
static void generate_order_id(char *orderId) {
    time_t t=time(NULL);
    struct tm *tm=localtime(&t);
    int rand_part=rand()%1000;
    sprintf(orderId,"%04d%02d%02d%02d%02d%02d%03d",
        tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday,
        tm->tm_hour, tm->tm_min, tm->tm_sec,rand_part);
}

/*初始化订单与销毁订单*/
/*初始化订单管理器*/
OrderManager* create_order_manager() {
    OrderManager *mgr=(OrderManager*)malloc(sizeof(OrderManager));
    mgr->orders.data=NULL;
    mgr->orders.capacity=0;
    mgr->orders.size=0;
    mgr->hashSize=101;//素数减少冲突
    mgr->orderHash=(OrderIdxNode**)calloc(mgr->hashSize,sizeof(OrderIdxNode*));
    mgr->idHash = (IdHashNode**)calloc(mgr->hashSize, sizeof(IdHashNode*));
    //加载文件
    load_orders_from_file(mgr);
    return mgr;
}

void destroy_order_manager(OrderManager *mgr) {
    save_orders_to_file(mgr);
    /* 释放订单号哈希表 */
    for (int i = 0; i < mgr->hashSize; i++) {
        OrderIdxNode *p = mgr->orderHash[i];
        while (p) {
            OrderIdxNode *tmp = p;
            p = p->next;
            free(tmp);
        }
    }
    /* 释放证件号哈希表 */
    for (int i = 0; i < mgr->hashSize; i++) {
        IdHashNode *p = mgr->idHash[i];
        while (p) {
            IdHashNode *tmp = p;
            p = p->next;
            /* 释放链表 */
            IdListNode *lst = tmp->orderList;
            while (lst) {
                IdListNode *ltmp = lst;
                lst = lst->next;
                free(ltmp);
            }
            free(tmp);
        }
    }
    free(mgr->orderHash);
    free(mgr->idHash);
    free(mgr->orders.data);
    free(mgr);
}

/*创建订单*/
int create_order(OrderManager *mgr,const char *id,const char *flightNo, const char *name) {
    Order ord;
    generate_order_id(ord.orderId);
    strcpy(ord.passengerName,name);
    strcpy(ord.idNumber,id);
    strcpy(ord.flightNo,flightNo);
    ord.status=0;
    int idx=add_order(mgr,&ord);
    if (idx<0) {return -1;}
    insert_order_hash(mgr,ord.orderId,idx);
    insert_id_index(mgr,id,idx);
    save_orders_to_file(mgr);
    return 0;
}

/*退票*/
int cancel_order(OrderManager *mgr,const char *orderId,Manager *manager) {
    int idx=find_order_by_id(mgr,orderId);
    if (idx<0)return -1;//订单不存在
    Order *ord=&mgr->orders.data[idx];
    if (ord->status!=0) {return -2;}//已退票或不存在
    if (!cancel_seat(manager,ord->flightNo)) {
        return -3;//航班座位恢复失败
    }
    ord->status=1;//标记已退票
    save_orders_to_file(mgr);
    return 0;
}
/* 根据证件号查询所有有效订单（返回动态数组，调用者负责 free） */
Order* query_orders_by_id(OrderManager *mgr, const char *idNumber, int *count) {
    unsigned int slot = hash_str(idNumber, mgr->hashSize);
    IdHashNode *p = mgr->idHash[slot];
    while (p) {
        if (strcmp(p->key, idNumber) == 0) {
            Order *result = NULL;
            int cnt = 0;
            IdListNode *cur = p->orderList;
            while (cur) {
                Order *ord = &mgr->orders.data[cur->orderIndex];
                if (ord->status == 0) {
                    result = (Order*)realloc(result, (cnt+1) * sizeof(Order));
                    result[cnt] = *ord;
                    cnt++;
                }
                cur = cur->next;
            }
            *count = cnt;
            return result;
        }
        p = p->next;
    }
    *count = 0;
    return NULL;
}

/* 检查订单是否有效 */
int is_valid_order(OrderManager *mgr, const char *orderId) {
    int idx = find_order_by_id(mgr, orderId);
    if (idx < 0) return 0;
    return (mgr->orders.data[idx].status == 0);
}

/*分页显示航班列表（直接传入航班指针数组）*/
void display_flights_page_by_page(Flight **flights, int count) {
    if (count == 0) {
        printf("没有符合条件的航班。\n");
        return;
    }

    int page_size = 5;
    int total_pages = (count + page_size - 1) / page_size;
    int current_page = 0;
    char cmd;

    while (current_page < total_pages) {
        printf("\033[2J\033[H");  // 清屏
        printf("====== 航班列表（第 %d/%d 页） ======\n", current_page + 1, total_pages);

        int start = current_page * page_size;
        int end = (start + page_size < count) ? start + page_size : count;

        for (int i = start; i < end; i++) {
            Flight *f = flights[i];
            double price = f->price * f->discount;
            printf("%d. %s  %s→%s  起飞:%s  到达:%s  票价:%.2f  余票:%d/%d\n",
                   i + 1,
                   f->flight_no,          // 航班号字段
                  f->start,              // 起飞城市
                   f->end,                // 抵达城市
                   f->start_time,         // 起飞时间
                   f->end_time,           // 到达时间
                   price,
                   f->remaining_seats,    // 剩余座位
                   f->total_seats);       // 总座位
        }
        printf("--------------------------------------------\n");
        if (current_page < total_pages - 1) {
            printf("输入 'n' 下一页, 'q' 退出: ");
            scanf(" %c", &cmd);
            if (cmd == 'n' || cmd == 'N')
                current_page++;
            else
                break;
        } else {
            printf("已是最后一页，按回车键继续...");
            getchar();
            getchar();
            break;
        }
    }
}

/*按起飞时间对航班数组排序*/
void sort_flights_by_departure(Flight **flights, int count) {
    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - i - 1; j++){
            if (strcmp(flights[j]->start_time, flights[j+1]->start_time) > 0) {
                Flight *tmp = flights[j];
                flights[j] = flights[j+1];
                flights[j+1] = tmp;
            }
        }
    }
}

/*智能推荐同航线备选航班（按起飞时间排序，分页显示）*/
void recommend_flights(Manager *manager, const char *src, const char *dst) {
    Flight *results[100];
    int count = 0;
    get_flights_by_route(manager, src, dst, results, &count);

    if (count == 0) {
        printf("该航线没有其他航班。\n");
        return;
    }
    // 按起飞时间排序
    sort_flights_by_departure(results, count);
    printf("该航班已满仓，为您推荐以下备选航班（按起飞时间排序）：\n");
    display_flights_page_by_page(results, count);
}

Order* query_all_orders_by_id(OrderManager *mgr, const char *idNumber, int *count) {
    unsigned int slot = hash_str(idNumber, mgr->hashSize);
    IdHashNode *p = mgr->idHash[slot];
    while (p) {
        if (strcmp(p->key, idNumber) == 0) {
            Order *result = NULL;
            int cnt = 0;
            IdListNode *cur = p->orderList;
            while (cur) {
                Order *ord = &mgr->orders.data[cur->orderIndex];
                // 不过滤 status，全部返回
                result = (Order*)realloc(result, (cnt+1) * sizeof(Order));
                result[cnt] = *ord;
                cnt++;
                cur = cur->next;
            }
            *count = cnt;
            return result;
        }
        p = p->next;
    }
    *count = 0;
    return NULL;
}

/*改票*/
int change_flight(OrderManager *order_mgr, const char *orderId,
                  const char *newFlightNo, Manager *manager) {
    // 1. 参数有效性检查
    if (order_mgr == NULL || orderId == NULL || newFlightNo == NULL || manager == NULL) {
        return -5; // 参数错误
    }

    // 2. 查找原订单
    int idx = find_order_by_id(order_mgr, orderId);
    if (idx < 0) return -1; // 订单不存在
    Order *ord = &order_mgr->orders.data[idx];
    if (ord->status != 0) return -2; // 已退票或无效

    // 3. 获取原航班和新航班
    Flight *oldFlight = get_flight_by_number(manager, ord->flightNo);
    Flight *newFlight = get_flight_by_number(manager, newFlightNo);
    if (!oldFlight || !newFlight) {
        return -3; // 新航班不存在
    }

    // 4. 检查航线是否一致
    if (strcmp(oldFlight->start, newFlight->start) != 0 ||
        strcmp(oldFlight->end, newFlight->end) != 0) {
        return -4; // 航线不一致
        }

    // 5. 检查新航班余票
    if (newFlight->remaining_seats <= 0) {
        return -3; // 余票不足
    }

    // 6. 执行改签：先加旧座位，再减新座位（保证原子性，若失败则回滚）
    if (!cancel_seat(manager, ord->flightNo)) {
        return -3; // 恢复旧座位失败（理论不会）
    }
    if (!reserve_seat(manager, newFlightNo)) {
        // 回滚：把刚才加的旧座位减回去
        reserve_seat(manager, ord->flightNo); // 注意：这里可能因为并发有问题，但单机测试无妨
        return -3;
    }

    // 7. 更新订单的航班号
    strcpy(ord->flightNo, newFlightNo);
    save_orders_to_file(order_mgr); // 持久化
    return 0;
}
