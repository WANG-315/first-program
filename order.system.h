//
// Created by wzh on 2026/7/15.
//

#ifndef SYSTEM1_ORDER_SYSTEM_H
#define SYSTEM1_ORDER_SYSTEM_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>


/*定义常量*/
#define MAX_FLIGHT_NO 10
#define MAX_CITY 20
#define MAX_NAME 20
#define MAX_ID 20
#define MAX_ORDER_ID 20
#define MAX_LINE 512

/*写航班的结构*/
typedef struct {
    char flightNo[MAX_FLIGHT_NO];
    char srcCity[MAX_CITY];
    char dstCity[MAX_CITY];
    char depTime[6];
    char arrTime[6];
    int basePrice;
    double discount;
    int totalSeats;
    int remainSeats;
}Flights;

/*写订单的结构*/
typedef struct {
    char orderId[MAX_ORDER_ID];
    char passengerName[MAX_NAME];
    char idNumber[MAX_ID];
    char flightNo[MAX_FLIGHT_NO];
    int status;
} Order;

typedef struct OrderIdxNode {
    char key[MAX_ORDER_ID];
    int value;//在数组的下标
    struct OrderIdxNode *next;
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
    Order *data;//指向Order数组的指针
    int capacity;
    int size;
}OrderArray;

typedef struct {
    OrderArray orders;
    OrderIdxNode **orderHash;
    int hashSize;
    IdHashNode **idHash;
}OrderManager;

/**
 * @brief 预定航班座位
 * @param manager 预定航班座位
 * @param flight_no 目标航班号
 * @return 成功返回1，失败返回错误码
 */
int reserve_seat(Manager *manager, const char *flight_no) ;//预定指定航班座位，扣减余票
/**
 * @brief 退票，归还航班空余座位
 * @param manager 航班管理器指针
 * @param flightNo 目标航班号
 * @return 成功返回1，失败返回错误码
 */
int cancel_seat(Manager *manager, const char *flightNo) ;//退票，恢复航班剩余座位
/**
 * @brief 私有哈希函数，计算字符串哈希值
 * @param str 待哈希字符串
 * @param table_size 哈希表长度
 * @return 哈希下标
 */
static unsigned int hash_str(const char *str,int table_size);//私有哈希函数，计算字符串哈希值
/**
 * @brief 将新订单存入订单管理器
 * @param mgr 订单管理器指针
 * @param ord 待存入的订单结构体指针
 * @return 成功返回1，失败返回0
 */
static int add_order(OrderManager *mgr,const Order *ord);//新增订单存入管理系统
/**
 * @brief 将订单ID存入哈希索引，用于快速查询订单
 * @param mgr 订单管理器指针
 * @param orderId 订单编号
 * @param idx 订单在订单数组中的下标
 */
static void insert_order_hash(OrderManager *mgr, const char *orderId, int idx);//将订单ID存入哈希索引方便快速查找
/**
 * @brief 根据订单号查找订单下标
 * @param mgr 订单管理器指针
 * @param orderId 待查询订单号
 * @return 找到返回下标，未找到返回-1
 */
static int find_order_by_id(OrderManager *mgr,const char *orderId);//通过订单ID检索订单下标
/**
 * @brief 身份证索引插入，绑定身份证与订单下标
 * @param mgr 订单管理器指针
 * @param idNumber 乘客身份证号
 * @param idx 订单数组下标
 */
static void insert_id_index(OrderManager *mgr,const char *idNumber,int idx);//私有，将身份证号存入订单索引
/**
 * @brief 从文件读取全部订单数据加载到内存
 * @param mgr 订单管理器指针
 * @return 加载成功返回1，失败返回0
 */
static int load_orders_from_file(OrderManager *mgr);//从文件索引加载全部订单数据
/**
 * @brief 将内存中所有订单写入文件持久化保存
 * @param mgr 订单管理器指针
 * @return 保存成功返回1，失败返回0
 */
static int save_orders_to_file(OrderManager *mgr);//将所有订单写入文件持久保存
/**
 * @brief 生成全局唯一订单编号
 * @param orderId 输出字符数组，存放生成的订单号
 */
static void generate_order_id(char *orderId);//生成唯一订单编号
/**
 * @brief 创建并初始化订单管理器，初始化哈希、索引、存储数组
 * @return 初始化完成的OrderManager指针，失败返回NULL
 */
OrderManager* create_order_manager();//创建订单管理对象，初始化索引与存储
/**
 * @brief 释放订单管理器全部堆内存资源
 * @param mgr 待销毁的订单管理器指针
 */
void destroy_order_manager(OrderManager *mgr);//释放订单管理器所有内存资源
/**
 * @brief 创建新订单，绑定乘客信息与航班
 * @param mgr 订单管理器指针
 * @param id 乘客身份证号
 * @param flightNo 预订航班号
 * @param name 乘客姓名
 * @return 创建成功返回1，失败返回对应错误码
 */
int create_order(OrderManager *mgr,const char *id,const char *flightNo, const char *name) ;//创建新订单，绑定乘客，航班信息
/**
 * @brief 根据订单号撤销/退票订单
 * @param mgr 订单管理器指针
 * @param orderId 待取消订单号
 * @param manager 航班管理器，用于释放座位
 * @return 成功返回1，失败返回错误码
 */
int cancel_order(OrderManager *mgr,const char *orderId, Manager *manager);//根据订单号撤销订单
/**
 * @brief 根据身份证号查询该用户全部订单
 * @param mgr 订单管理器指针
 * @param idNumber 乘客身份证号
 * @param count 输出参数，返回匹配到的订单总数
 * @return 动态分配的订单数组首地址，无订单返回NULL
 */
Order* query_orders_by_id(OrderManager *mgr, const char *idNumber, int *count) ;//凭身份证号查询该用户全部订单
/**
 * @brief 校验订单号是否真实存在
 * @param mgr 订单管理器指针
 * @param orderId 待校验订单号
 * @return 存在返回1，不存在返回0
 */
int is_valid_order(OrderManager *mgr, const char *orderId);//比较订单号是否存在
/**
 * @brief 分页打印航班列表
 * @param flights 航班指针数组
 * @param count 有效航班数量
 */
void display_flights_page_by_page(Flight **flights, int count);//分页打印航班列表
/**
 * @brief 将航班数组按起飞时间升序排序
 * @param flights 航班指针数组
 * @param count 航班总数
 */
void sort_flights_by_departure(Flight **flights, int count);//按起飞时间给航班排序
/**
 * @brief 智能推荐同航线备选航班，排序后分页展示
 * @param manager 航班管理器指针
 * @param src 出发城市
 * @param dst 抵达城市
 */
void recommend_flights(Manager *manager, const char *src, const char *dst);//按起止城市，推荐同航线备选航班
/**
 * @brief 凭身份证查询用户全部订单
 * @param mgr 订单管理器指针
 * @param idNumber 乘客身份证号
 * @param count 输出参数，匹配订单数量
 * @return 订单数组指针，无订单返回NULL
 */
Order* query_all_orders_by_id(OrderManager *mgr, const char *idNumber, int *count);//凭证件号查用户全部订单
/**
 * @brief 订单改签，更换绑定航班
 * @param mgr 订单管理器指针
 * @param orderId 需要改签的订单号
 * @param newFlightNo 新目标航班号
 * @param manager 航班管理器，校验航班、增减余座
 * @return 0成功，负数为各类错误码
 */
int change_flight(OrderManager *mgr, const char *orderId, const char *newFlightNo, Manager *manager);//改签

#endif //SYSTEM1_ORDER_SYSTEM_H
