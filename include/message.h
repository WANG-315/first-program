//
// Created by wzh on 2026/7/10.
//

#ifndef SYSTEM1_MESSAGE_H
#define SYSTEM1_MESSAGE_H

#include <stdio.h>
#include<time.h>
#include<stdbool.h>

#define MAX_FIGHT_NO 1000
#define HASH_NUM 2003  //素数，减少哈希冲突

//航班数据结构
typedef struct {
    char flight_no[20];
    char start[50];
    char end[50];
    char start_time[50];
    char end_time[50];
    double price;
    double discount;
    int total_seats;
    int remaining_seats;
    char last_updated[30];
}Flight;

//航班号索引节点（哈希表）
typedef struct FightIndexNode{
    char flight_no[MAX_FIGHT_NO];
    int array_index;  //在主储存数组中的索引
    struct FightIndexNode *next;  //指向下一个航班号
}FlightIndexNode;

//航线索引节点（嵌套哈希表）
typedef struct RouteListNode{
    char flight_no[MAX_FIGHT_NO];
    struct RouteListNode *next;  //指向下一个航班
}RouteListNode;

typedef struct RouteHashNode{
    char start[50];
    struct RouteDestNode *destinations;  //指向目的地的航班链表
    struct RouteHashNode *next;  //解决哈希冲突
}RouteHashNode;

typedef struct RouteDestNode {
    char end[50];
    RouteListNode* flights;  //该航线的航班链表
    struct RouteDestNode* next;  //指向下一个目的地
} RouteDestNode;

typedef struct {
    Flight *flights[MAX_FIGHT_NO];
    int flights_count;

    //航班号索引
    FlightIndexNode *flight_index[HASH_NUM];

    //航线索引
    RouteHashNode *route_index[HASH_NUM];

    bool is_modified;  //航线信息是否被修改
}Manager;

/**
 * 哈希函数
 * @param str 要计算哈希值的字符串
 * @return 哈希值（无符号整数）
 */
unsigned int hash_string(const char *str);  //哈希函数
/**
 * 初始化航班号索引
 * @param manager 航班管理器指针
 */
void init_flight_index(Manager *manager);  //航班号索引
/**
 * 向航班号索引中添加航班
 * @param manager 航班管理器指针
 * @param flight_no 航班号
 * @param index 在主存储数组中的索引
 */
void add_to_flight_index(Manager *manager, const char *flight_no, int index);  //添加航班
/**
 * 通过航班号在索引中查找航班
 * @param manager 航班管理器指针
 * @param flight_no 航班号
 * @return 在主存储数组中的索引，未找到返回 -1
 */
int find_flight_index(Manager *manager,const char *flight_no);  //查找航班
/**
 * 从航班号索引中删除航班
 * @param manager 航班管理器指针
 * @param flight_no 要删除的航班号
 */
void remove_from_flight_index(Manager *manager,const char *flight_no);  //删除航班
/**
 * 初始化航线索引
 * @param manager 航班管理器指针
 */
void init_route_index(Manager *manager);  //航线索引
/**
* 在航线节点中查找目的地
* @param route_node 航线节点指针
* @param end 目的地城市
* @return 找到的目的地节点指针，未找到返回 NULL
*/
RouteDestNode *find_destination(RouteHashNode* route_node, const char* end);
/**
 * 向航线索引中添加航班
 * @param manager 航班管理器指针
 * @param start 出发城市
 * @param end 到达城市
 * @param flight_no 航班号
 */
void add_to_route_index(Manager *manager, const char *start,
                        const char *end, const char *flight_no);
/**
 * 从航线索引中删除航班
 * @param manager 航班管理器指针
 * @param start 出发城市
 * @param end 到达城市
 * @param flight_no 要删除的航班号
 */
void remove_from_route_index(Manager *manager, const char *start,
                             const char *end, const char *flight_no);
/**
 * 创建航班管理器实例
 * @return 初始化好的航班管理器指针
 */
Manager *create_flight_manager();
/**
 * 释放单个航班内存
 * @param flight 要释放的航班指针
 */
void free_flight(Flight *flight);
/**
 * 释放航班管理器所有内存
 * @param manager 要释放的航班管理器指针
 */
void free_flight_manager(Manager *manager);
/**
 * 获取当前系统时间字符串
 * @param buffer 输出缓冲区
 * @param size 缓冲区大小
 */
void get_current_time(char *buffer, size_t size);
/**
 * 创建航班对象
 * @param flight_no 航班号
 * @param start 出发城市
 * @param end 到达城市
 * @param start_time 起飞时间
 * @param end_time 到达时间
 * @param price 基础价格
 * @param discount 折扣率
 * @param total_seats 总座位数
 * @return 创建的航班对象指针
 */
Flight* create_flight(const char *flight_no, const char *start,
                      const char *end, const char *start_time,
                      const char *end_time, double price,
                      double discount, int total_seats);
/**
 * 计算航班当前实际价格（考虑折扣）
 * @param flight 航班指针
 * @return 实际价格
 */
double get_current_price(Flight *flight);
/**
 * 添加航班到管理系统
 * @param manager 航班管理器指针
 * @param flight 要添加的航班指针
 * @return 成功返回 true，失败返回 false
 */
bool add_flight(Manager *manager, Flight *flight);
/**
 * 通过航班号获取航班
 * @param manager 航班管理器指针
 * @param flight_no 航班号
 * @return 航班指针，未找到返回 NULL
 */
Flight* get_flight_by_number(Manager *manager, const char *flight_no);
/**
 * 按航线（出发地-目的地）获取航班列表
 * @param manager 航班管理器指针
 * @param start 出发城市
 * @param end 到达城市
 * @param result 输出参数，存放结果的航班指针数组
 * @param count 输出参数，结果数量
 */
void get_flights_by_route(Manager *manager, const char *start,
                          const char *end, Flight **result, int *count);
/**
 * 更新航班信息
 * @param manager 航班管理器指针
 * @param flight_no 要更新的航班号
 * @param ... 可变参数，要更新的字段和值
 * @return 成功返回 true，失败返回 false
 */
bool update_flight(Manager *manager,const char *flight_no,...);
/**
 * 删除航班
 * @param manager 航班管理器指针
 * @param flight_no 要删除的航班号
 * @return 成功返回 true，失败返回 false
 */
bool delete_flight(Manager *manager, const char *flight_no);
/**
 * 预订座位
 * @param manager 航班管理器指针
 * @param flight_no 航班号
 * @param quantity 预订数量
 * @return 成功返回 true，失败返回 false
 */
bool book_seat(Manager *manager, const char *flight_no, int quantity);
/**
 * 保存数据到文件（持久化）
 * @param manager 航班管理器指针
 */
void save_to_file(Manager *manager);  //数据持久化
/**
 * 从文件加载数据
 * @param manager 航班管理器指针
 */
void load_from_file(Manager *manager);
/**
 * 打印航班信息
 * @param flight 航班指针
 */
void print_flight(Flight *flight);
/**
 * 打印所有航班信息
 * @param manager 航班管理器指针
 */
void print_all_flights(Manager *manager);
/**
 * 打印航班统计信息
 * @param manager 航班管理器指针
 */
void print_flight_stats(Manager *manager);

#endif //SYSTEM1_MESSAGE_H
