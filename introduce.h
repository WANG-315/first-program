//
// Created by wzh on 2026/7/10.
//

#ifndef SYSTEM1_INTRODUCE_H
#define SYSTEM1_INTRODUCE_H

#include"message.h"

//价格AVL树节点
typedef struct PriceAVLNode {
    double price;
    char flight_no [20];
    struct PriceAVLNode *left;  //AVL树左旋
    struct PriceAVLNode *right;  //AVL树右旋
    int height;  //高度
}PriceAVLNode;

//时间AVL树节点（起飞时间）
typedef struct TimeAVLNode {
    int minutes;  //从分钟数开始，效率更快更精确
    char flight_no [20];
    struct TimeAVLNode *left;
    struct TimeAVLNode *right;
    int height;
}TimeAVLNode;

//查询条件结构体
typedef struct {
    char start[50];
    char end[50];
    char date[20];  //具体日期，YYYY-MM-DD
    int start_minutes;  //开始时间（分钟）
    int end_minutes;  //结束时间（分钟）
    double min_price;  //最便宜价格
    double max_price;  //最昂贵价格
    int min_remaining_seats;
    bool has_started;
    bool has_ended;
    bool has_date;
    bool has_time_range;
    bool has_price_range;
    bool has_seats;
} QueryCriteria;

//推荐结果结构体
typedef struct {
    Flight *flight;
    double score;  //推荐分数（分数越低越优先）
} RecommendationResult;

/**
 * 向价格AVL树中插入航班
 * @param node AVL树根节点指针
 * @param price 航班价格
 * @param flight_no 航班号
 * @return 插入后的新树根节点
 */
PriceAVLNode *price_avl_insert(PriceAVLNode *node, double price, const char *flight_no);
/**
 * 价格范围查询
 * @param node AVL树根节点
 * @param min_price 最低价格
 * @param max_price 最高价格
 * @param results 输出参数，存放查询结果的航班指针数组
 * @param count 输出参数，查询结果数量
 * @param manager 航班管理器指针
 */
void price_avl_range_query(PriceAVLNode *node, double min_price,double max_price,Flight **results,int *count,Manager *manager);
/**
 * 释放价格AVL树内存
 * @param node 要释放的树根节点
 */
void price_avl_free(PriceAVLNode *node);

/**
 * 向时间AVL树中插入航班
 * @param node AVL树根节点指针
 * @param minutes 起飞时间（分钟数）
 * @param flight_no 航班号
 * @return 插入后的新树根节点
 */
TimeAVLNode *time_avl_insert(TimeAVLNode *node, int minutes, const char *flight_no);
/**
 * 时间范围查询
 * @param node AVL树根节点
 * @param start_minutes 开始时间（分钟数）
 * @param end_minutes 结束时间（分钟数）
 * @param results 输出参数，存放查询结果的航班指针数组
 * @param count 输出参数，查询结果数量
 * @param manager 航班管理器指针
 */
void time_avl_range_query(TimeAVLNode *node, int start_minutes, int end_minutes,Flight **results,int *count,Manager *manager);
/**
 * 释放时间AVL树内存
 * @param node 要释放的树根节点
 */
void time_avl_free(TimeAVLNode *node);

/**
 * 构建价格和时间AVL索引
 * @param manager 航班管理器指针
 * @param price_root 输出参数，价格AVL树根节点
 * @param time_root 输出参数，时间AVL树根节点
 */
void build_avl_indexes(Manager *manager, PriceAVLNode **price_root, TimeAVLNode **time_root);

/**
 * 多条件组合查询
 * @details 支持按出发地、目的地、日期、时间范围、价格范围、座位数等条件组合查询
 * @param manager 航班管理器指针
 * @param qc 查询条件结构体指针
 * @param results 输出参数，存放查询结果的航班指针数组
 * @param max_results 最大返回结果数
 * @return 实际查询到的结果数量
 */
int complex_query(Manager *manager, QueryCriteria *qc, Flight **results,int max_results);
/**
 * 价格范围查询
 * @param root 价格AVL树根节点
 * @param min_price 最低价格
 * @param max_price 最高价格
 * @param manager 航班管理器指针
 * @param results 输出参数，存放查询结果的航班指针数组
 * @param max_results 最大返回结果数
 * @return 实际查询到的结果数量
 */
int price_range_query(PriceAVLNode *root, double min_price, double max_price, Manager *manager,Flight **results, int max_results);
/**
 * 时间范围查询
 * @param root 时间AVL树根节点
 * @param start_minutes 开始时间（分钟数）
 * @param end_minutes 结束时间（分钟数）
 * @param manager 航班管理器指针
 * @param results 输出参数，存放查询结果的航班指针数组
 * @param max_results 最大返回结果数
 * @return 实际查询到的结果数量
 */
int time_range_query(TimeAVLNode *root, int start_minutes, int end_minutes, Manager *manager,Flight **results, int max_results);
/**
 * 智能推荐算法
 * @details 根据出发地、目的地、偏好时间和价格上限推荐最优航班
 * @param manager 航班管理器指针
 * @param start 出发城市
 * @param end 到达城市
 * @param preferred_time_str 偏好时间字符串
 * @param max_price 最高可接受价格
 * @param recommendations 输出参数，存放推荐结果的航班指针数组
 * @param max_count 最大推荐数量
 * @return 实际推荐结果数量
 */
int smart_recommend(Manager *manager, const char *start,const char *end, const char *preferred_time_str, double max_price, Flight **recommendations, int max_count);
/**
 * 满仓推荐算法
 * @details 在价格和时间约束下推荐尽可能满仓的航班
 * @param manager 航班管理器指针
 * @param start 出发城市
 * @param end 到达城市
 * @param preferred_time_str 偏好时间字符串
 * @param max_price 最高可接受价格
 * @param recommendations 输出参数，存放推荐结果的航班指针数组
 * @param max_count 最大推荐数量
 * @return 实际推荐结果数量
 */
int full_flight_recommend(Manager *manager, const char *start, const char *end, const char *preferred_time_str, double max_price, Flight **recommendations,int max_count);
/**
 * 将时间字符串转换为分钟数
 * @param time_str 时间字符串，格式：HH:MM
 * @return 从00:00开始的分钟数
 */
int time_to_minutes_from_str(const char *time_str);
/**
 * 从日期时间字符串中提取日期
 * @param datetime 日期时间字符串，格式：YYYY-MM-DD HH:MM:SS
 * @param date 输出参数，提取的日期字符串，格式：YYYY-MM-DD
 */
void date_from_datetime(const char *datetime, char *date);
/**
 * 从日期时间字符串中提取时间
 * @param datetime 日期时间字符串，格式：YYYY-MM-DD HH:MM:SS
 * @param time 输出参数，提取的时间字符串，格式：HH:MM:SS
 */
void time_from_datetime(const char *datetime, char *time);

/**
 * 按价格对航班数组进行排序（升序）
 * @param flight 航班指针数组
 * @param count 数组长度
 */
void sort_flights_by_price(Flight **flight, int count);
/**
 * 按起飞时间对航班数组进行排序（升序）
 * @param flight 航班指针数组
 * @param count 数组长度
 */
void sort_flights_by_time(Flight **flight,int count);


#endif //SYSTEM1_INTRODUCE_H
