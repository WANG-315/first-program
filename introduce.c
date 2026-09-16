#include"message.h"
#include <stdlib.h>
#include<string.h>

//价格AVL树节点
typedef struct PriceAVLNode {
    double price;  //键值（价格），用于排序和查找
    char flight_no [20];
    struct PriceAVLNode *left;  //AVL树左子树
    struct PriceAVLNode *right;  //AVL树右子树
    int height;  //高度
}PriceAVLNode;

//时间AVL树节点（起飞时间）
typedef struct TimeAVLNode {
    int minutes;  //从分钟数开始，效率更快更精确
    char flight_no [20];
    struct TimeAVLNode *left;
    struct TimeAVLNode *right;
    int height;
} TimeAVLNode;

//查询条件结构体
typedef struct {
    char start[50];  //起飞地点
    char end[50];  //落地地点
    char date[20];  //具体日期，YYYY-MM-DD
    int start_minutes;  //开始时间（分钟）
    int end_minutes;  //结束时间（分钟）
    double min_price;  //最便宜价格
    double max_price;  //最昂贵价格
    int min_remaining_seats;
    bool has_started;  //是否指定了起飞城市
    bool has_ended;  //是否指定了落地城市
    bool has_date;  //是否指定了日期
    bool has_time_range;  //是否指定了时间范围
    bool has_price_range;  //是否指定了价格范围
    bool has_seats;  //是否指定了座位数
} QueryCriteria;

//推荐结果结构体
typedef struct {
    Flight *flight;
    double score;  //推荐分数（分数越低越优先）
} RecommendationResult;

//AVL树辅助函数
//获取高度
static int price_get_height(PriceAVLNode *node) {
    return node ? node->height : 0;
}

static int time_get_height(TimeAVLNode *node) {
    return node ? node->height : 0;
}

//更新高度
static void price_update_height(PriceAVLNode *node) {
    if (node) {
        int left_h = price_get_height(node->left);  //左子树高度
        int right_h = price_get_height(node->right);  //右子树高度
        node->height = (left_h > right_h ? left_h : right_h) + 1;  //取左右子树高度的最大值+1
    }
}
static void time_update_height(TimeAVLNode* node) {
    if (node) {
        int left_h = time_get_height(node->left);
        int right_h = time_get_height(node->right);
        node->height = (left_h > right_h ? left_h : right_h) + 1;
    }
}

//计算平衡因子
static int price_balance_factor(PriceAVLNode *node) {
    return node ? price_get_height(node->left) - price_get_height(node->right) : 0;
}
static int time_balance_factor(TimeAVLNode *node) {
    return node ? time_get_height(node->left) - time_get_height(node->right) : 0;
}

//AVL树旋转操作
//价格
static PriceAVLNode *price_rotate_right(PriceAVLNode *y) {
    PriceAVLNode *x = y->right;  //x是y的左子节点
    PriceAVLNode *t2 = x->right;  //t2是x的右子节点

    //右旋操作
    x->right = y;
    y->left = t2;

    price_update_height(y);  // 先更新 y 的高度（因为 y 变成了子树）
    price_update_height(x);

    return x;
}
static PriceAVLNode *price_rotate_left(PriceAVLNode *x) {
    PriceAVLNode *y = x->right;
    PriceAVLNode *t2 = y->left;

    y->left = x;
    x->right = t2;

    price_update_height(x);
    price_update_height(y);

    return y;
}

//时间
static TimeAVLNode *time_rotate_right(TimeAVLNode *y) {
    TimeAVLNode *x = y->right;  //x是y的左子节点
    TimeAVLNode *t2 = x->right;  //t2是x的右子节点

    x->right = y;
    y->left = t2;

    time_update_height(y);  // 先更新 y 的高度（因为 y 变成了子树）
    time_update_height(x);

    return x;
}
static TimeAVLNode *time_rotate_left(TimeAVLNode *x) {
    TimeAVLNode *y = x->right;
    TimeAVLNode *t2 = y->left;

    y->left = x;
    x->right = t2;

    time_update_height(x);
    time_update_height(y);

    return y;
}

//插入函数（向AVL树中插入一个航班价格节点，返回插入后的树根节点）
//只返回指针而不是整个结构体，效率更高
PriceAVLNode *price_avl_insert(PriceAVLNode *node, double price, const char *flight_no) {
    if (!node) {
        PriceAVLNode *new_node = (PriceAVLNode *)malloc(sizeof(PriceAVLNode));/* 在堆上分配，函数返回后仍然存在，
                                                                                     直到手动调用 free() 释放 */
        new_node->price = price;
        strcpy(new_node->flight_no, flight_no);
        new_node->left = new_node->right = NULL;
        new_node->height = 1;
        return new_node;
    }

    //小于插入到左子树，大于等于插入到右子树
    if (price < node->price) {
        node->left = price_avl_insert(node->left,price, flight_no);
    }else if (price > node->price) {
        node->right = price_avl_insert(node->right,price, flight_no);
    }else {
        //价格相同，插入到右子树
        node->right = price_avl_insert(node->right,price, flight_no);
    }

    price_update_height(node);

    int balance = price_balance_factor(node);

    //LL情况
    if (balance > 1 && price < node->left->price) {
        return price_rotate_right(node);
    }
    //RR
    if (balance < -1 && price > node->right->price) {
        return price_rotate_left(node);
    }
    //LR
    if (balance > 1 && price > node->right->price) {
        node->left = price_rotate_left(node->left);
        return price_rotate_right(node);
    }
    //RL
    if (balance < -1 && price < node->left->price) {
        node->right = price_rotate_right(node->right);
        return price_rotate_left(node);
    }

    return node;
}

TimeAVLNode* time_avl_insert(TimeAVLNode *node, int minutes, const char *flight_no) {
    if (!node) {
        TimeAVLNode* new_node = (TimeAVLNode*)malloc(sizeof(TimeAVLNode));
        new_node->minutes = minutes;
        strcpy(new_node->flight_no, flight_no);
        new_node->left = new_node->right = NULL;
        new_node->height = 1;
        return new_node;
    }

    if (minutes < node->minutes) {
        node->left = time_avl_insert(node->left, minutes, flight_no);
    } else if (minutes > node->minutes) {
        node->right = time_avl_insert(node->right, minutes, flight_no);
    } else {
        node->right = time_avl_insert(node->right, minutes, flight_no);
    }

    time_update_height(node);

    int balance = time_balance_factor(node);

    // LL情况
    if (balance > 1 && minutes < node->left->minutes) {
        return time_rotate_right(node);
    }
    // RR情况
    if (balance < -1 && minutes > node->right->minutes) {
        return time_rotate_left(node);
    }
    // LR情况
    if (balance > 1 && minutes > node->left->minutes) {
        node->left = time_rotate_left(node->left);
        return time_rotate_right(node);
    }
    // RL情况
    if (balance < -1 && minutes < node->right->minutes) {
        node->right = time_rotate_right(node->right);
        return time_rotate_left(node);
    }

    return node;
}

void price_avl_range_query(PriceAVLNode *node, double min_price,
double max_price,Flight **results,int *count,Manager *manager) {
    if (!node || *count >= MAX_FIGHT_NO) return;

    //如果当前节点价格在范围内
    if (node->price >= min_price && node->price <= max_price) {
        int index = find_flight_index(manager,node->flight_no);
        if (index != -1) {
            results[*count] = manager->flights[index];
            (*count)++;
        }
    }

    // 如果当前价格大于最小值，继续遍历左子树
    if (node->price > min_price) {
        price_avl_range_query(node->left, min_price, max_price, results, count, manager);
    }

    // 如果当前价格小于最大值，继续遍历右子树
    if (node->price < max_price) {
        price_avl_range_query(node->right, min_price, max_price, results, count, manager);
    }
}

void time_avl_range_query(TimeAVLNode* node, int start_minutes, int end_minutes,
                          Flight** results, int* count, Manager* manager) {
    if (!node || *count >= MAX_FIGHT_NO) return;

    if (node->minutes >= start_minutes && node->minutes <= end_minutes) {
        int index = find_flight_index(manager, node->flight_no);
        if (index != -1) {
            results[*count] = manager->flights[index];
            (*count)++;
        }
    }

    if (node->minutes > start_minutes) {
        time_avl_range_query(node->left, start_minutes, end_minutes, results, count, manager);
    }

    if (node->minutes < end_minutes) {
        time_avl_range_query(node->right, start_minutes, end_minutes, results, count, manager);
    }
}

//递归释放AVL树
void price_avl_free(PriceAVLNode* node) {
    if (!node) return;
    price_avl_free(node->left);
    price_avl_free(node->right);
    free(node);
}

void time_avl_free(TimeAVLNode* node) {
    if (!node) return;
    time_avl_free(node->left);
    time_avl_free(node->right);
    free(node);
}

//将时间字符串转换为分钟数
int time_to_minutes_from_str(const char *time_str) {
    int hour,minute;
    sscanf(time_str,"%d:%d",&hour,&minute);
    return hour*60 + minute;
}

//提取日期部分
void date_from_datetime(const char *datetime, char *date) {
    strncpy(date,datetime,10);  //YYYY-MM-DD
    date[10] = '\0';
}

//提取时间部分
void time_from_datetime(const char *datetime, char *time) {
    strncpy(time,datetime + 11,5); //(YYYY-MM-DD-)->11  HH:MM
    time[5] = '\0';
}

//遍历所有航班，将价格和起飞时间分别插入到AVL树中，构建两个索引
void build_avl_indexes(Manager *manager, PriceAVLNode **price_root, TimeAVLNode **time_root) {
    *price_root = NULL;
    *time_root = NULL;

    //  双指针，分为调用时要传入地址，以及函数内部初始化

    for (int i=0; i < manager->flights_count; i++) {
        Flight *flight = manager->flights[i];
        if (!flight) continue;

        //插入价格索引
        double price = get_current_price(flight);
        *price_root = price_avl_insert(*price_root,price,flight->flight_no);  //价格为键，航班号为值

        //插入时间索引
        char time_str[10];
        time_from_datetime(flight->start_time,time_str);
        int minutes = time_to_minutes_from_str(time_str);
        *time_root = time_avl_insert(*time_root,minutes,flight->flight_no);
    }
}

int complex_query(Manager *manager, QueryCriteria* qc,
                  Flight **results, int max_results) {
    int count = 0;

    // 优先使用航线索引（如果有航线条件）
    if (qc->has_started && qc->has_ended) {
        Flight* route_results[100];
        int route_count = 0;
        get_flights_by_route(manager, qc->start,
                            qc->end, route_results, &route_count);

        // 在航线结果上进行二次过滤
        for (int i = 0; i < route_count && count < max_results; i++) {
            Flight* flight = route_results[i];
            bool match = true;

            // 检查日期
            if (qc->has_date) {
                char date[20];
                date_from_datetime(flight->start_time, date);
                if (strcmp(date, qc->date) != 0) {
                    match = false;
                }
            }

            // 检查时间范围
            if (match && qc->has_time_range) {
                char time_str[10];
                time_from_datetime(flight->start_time, time_str);
                int minutes = time_to_minutes_from_str(time_str);
                if (minutes < qc->start_minutes ||
                    minutes > qc->end_minutes) {
                    match = false;
                }
            }

            // 检查价格范围
            if (match && qc->has_price_range) {
                double price = get_current_price(flight);
                if (price > qc->max_price) {
                    match = false;
                }
            }

            // 检查剩余座位
            if (match && qc->has_seats) {
                if (flight->remaining_seats < qc->min_remaining_seats) {
                    match = false;
                }
            }

            if (match) {
                results[count++] = flight;
            }
        }
    } else {
        // 没有航线条件，遍历所有航班
        for (int i = 0; i < manager->flights_count && count < max_results; i++) {
            Flight* flight = manager->flights[i];
            if (!flight) continue;

            bool match = true;

            if (qc->has_started &&
                strcmp(flight->start, qc->start) != 0) {
                match = false;
            }

            if (match && qc->has_ended &&
                strcmp(flight->end, qc->end) != 0) {
                match = false;
            }

            if (match && qc->has_date) {
                char date[20];
                date_from_datetime(flight->start_time, date);
                if (strcmp(date, qc->date) != 0) {
                    match = false;
                }
            }

            if (match && qc->has_time_range) {
                char time_str[10];
                time_from_datetime(flight->start_time, time_str);
                int minutes = time_to_minutes_from_str(time_str);
                if (minutes < qc->start_minutes ||
                    minutes > qc->end_minutes) {
                    match = false;
                }
            }

            if (match && qc->has_price_range) {
                double price = get_current_price(flight);
                if (price < qc->min_price || price > qc->max_price) {
                    match = false;
                }
            }

            if (match && qc->has_seats) {
                if (flight->remaining_seats < qc->min_remaining_seats) {
                    match = false;
                }
            }

            if (match) {
                results[count++] = flight;
            }
        }
    }

    return count;
}

//价格范围查询
int price_range_query(PriceAVLNode *root, double min_price, double max_price,
    Manager *manager,Flight **results, int max_results) {
    int count = 0;
    price_avl_range_query(root, min_price, max_price, results, &count, manager);
    return count;
}

//时间范围查询
int time_range_query(TimeAVLNode *root, int start_minutes, int end_minutes,
    Manager *manager,Flight **results, int max_results) {
    int count = 0;
    time_avl_range_query(root, start_minutes, end_minutes, results, &count, manager);
    return count;
}

void sort_flights_by_price(Flight **flight, int count) {
    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j< count - i - 1; j++) {
            if (get_current_price(flight[j]) < get_current_price(flight[j + 1])) {
                Flight *tmp = flight[j];
                flight[j] = flight[j + 1];
                flight[j + 1] = tmp;
            }
        }
    }
}
void sort_flights_by_time(Flight **flight,int count) {
    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j< count - i - 1; j++) {
            char time1[10], time2[10];
            time_from_datetime(flight[j]->start_time, time1);
            time_from_datetime(flight[j + 1]->start_time, time2);
            int t1 = time_to_minutes_from_str(time1);
            int t2 = time_to_minutes_from_str(time2);
            if (t1 > t2) {
                Flight *tmp = flight[j];
                flight[j] = flight[j + 1];
                flight[j + 1] = tmp;
            }
        }
    }
}

int smart_recommend(Manager *manager, const char *start,const char *end,
    const char *preferred_time_str, double max_price, Flight **recommendations, int max_count) {
    Flight *candidates[100];
    int candidate_count = 0;

    QueryCriteria qc = {0};
    qc.has_started = true;
    strcpy(qc.start, start);
    qc.has_ended = true;
    strcpy(qc.end, end);
    qc.has_price_range = true;
    qc.min_price = 0;
    qc.max_price = max_price;
    qc.has_seats = true;
    qc.min_remaining_seats = 1;

    candidate_count = complex_query(manager,&qc,candidates,100);

    if (candidate_count == 0) {
        printf("没有找到符合条件的航班\n");
        return 0;
    }

    //计算推荐分数
    RecommendationResult recs[100];
    int preferred_time = time_to_minutes_from_str(preferred_time_str);

    for (int i = 0; i < candidate_count; i++) {
        recs[i].flight = candidates[i];

        // 计算时间偏差分数（越接近越好）
        char time_str[10];
        time_from_datetime(candidates[i]->start, time_str);
        int flight_minutes = time_to_minutes_from_str(time_str);
        int time_diff = abs(flight_minutes - preferred_time);
        double time_score = time_diff / 60.0;  // 转换为小时偏差

        // 计算价格分数（价格越低越好）
        double price = get_current_price(candidates[i]);
        double price_score = price / 100.0;  // 标准化

        // 计算座位分数（座位越多越好）
        double seat_score = 1.0 / (candidates[i]->remaining_seats + 1);

        // 综合评分（权重可调整）
        recs[i].score = time_score * 0.5 + price_score * 0.3 + seat_score * 0.2;
    }

    // 3. 按分数排序（冒泡排序）
    for (int i = 0; i < candidate_count - 1; i++) {
        for (int j = 0; j < candidate_count - i - 1; j++) {
            if (recs[j].score > recs[j + 1].score) {
                RecommendationResult tmp = recs[j];
                recs[j] = recs[j + 1];
                recs[j + 1] = tmp;
            }
        }
    }

    // 4. 返回推荐结果
    int result_count = candidate_count < max_count ? candidate_count : max_count;
    for (int i = 0; i < result_count; i++) {
        recommendations[i] = recs[i].flight;
    }

    return result_count;
}

int full_flight_recommend(Manager *manager, const char *start, const char *end,
    const char *preferred_time_str, double max_price, Flight **recommendations,int max_count) {
    // 获取所有符合条件的航班（包括满仓的）
    Flight* all_flights[100];
    int all_count = 0;

    QueryCriteria qc = {0};
    qc.has_started = true;
    strcpy(qc.start, start);
    qc.has_ended = true;
    strcpy(qc.end, end);
    qc.has_price_range = true;
    qc.min_price = 0;
    qc.max_price = max_price;
    // 不限制座位数，包括满仓的

    all_count = complex_query(manager, &qc, all_flights, 100);

    if (all_count == 0) {
        printf("没有找到任何航班\n");
        return 0;
    }

    // 排除满仓的航班（可用座位为0），并计算推荐分数
    RecommendationResult recs[100];
    int rec_count = 0;
    int preferred_minutes = time_to_minutes_from_str(preferred_time_str);

    for (int i = 0; i < all_count; i++) {
        // 跳过满仓航班（可用座位为0）
        if (all_flights[i]->remaining_seats == 0) {
            continue;
        }

        recs[rec_count].flight = all_flights[i];

        char time_str[10];
        time_from_datetime(all_flights[i]->start_time, time_str);
        int flight_minutes = time_to_minutes_from_str(time_str);
        int time_diff = abs(flight_minutes - preferred_minutes);
        double time_score = time_diff / 60.0;  //转换为小时偏差

        double price = get_current_price(all_flights[i]);
        double price_score = price / 100.0;

        double seat_score = 1.0 / (all_flights[i]->remaining_seats + 1);

        // 综合评分
        recs[rec_count].score = time_score * 0.5 + price_score * 0.3 + seat_score * 0.2;
        rec_count++;
    }

    if (rec_count == 0) {
        printf("所有航班都已满仓\n");
        return 0;
    }

    // 按分数排序
    for (int i = 0; i < rec_count - 1; i++) {
        for (int j = 0; j < rec_count - i - 1; j++) {
            if (recs[j].score > recs[j + 1].score) {
                RecommendationResult tmp = recs[j];
                recs[j] = recs[j + 1];
                recs[j + 1] = tmp;
            }
        }
    }

    // 返回推荐结果
    int result_count = rec_count < max_count ? rec_count : max_count;
    for (int i = 0; i < result_count; i++) {
        recommendations[i] = recs[i].flight;
    }

    return result_count;
}
