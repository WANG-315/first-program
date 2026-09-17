#include <stdio.h>
#include<time.h>
#include<stdbool.h>
#include <stdlib.h>
#include <string.h>

#define MAX_FIGHT_NO 1000
#define HASH_NUM 2003  //素数，减少哈希冲突
#define DATA_FILE "flights.dat"

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

//哈希函数
//将任意字符串转换为一个0 到 HASH_NUM-1 之间的整数，用于在哈希表中快速定位数据
unsigned int hash_string(const char *str) {
    unsigned int hash = 0;
    while (*str) {
        hash = (hash << 5) + *str++;
    }
    return hash % HASH_NUM;
}

//初始化哈希表
void init_flight_index(Manager *manager) {
    for (int i = 0; i < HASH_NUM; i++) {
        manager->flight_index[i] = NULL;
    }
}

//添加航班号到哈希索引
void add_to_flight_index(Manager *manager, const char *flight_no, int index) {
    unsigned int hash = hash_string(flight_no);
    FlightIndexNode *node = (FlightIndexNode*)malloc(sizeof(FlightIndexNode));  //用于存储航班号和它在数组中的位置
    strcpy(node->flight_no, flight_no);
    node->array_index = index;  //储存数组索引
    //头插法
    node->next = manager->flight_index[hash];
    manager->flight_index[hash] = node;
}

//寻找航班号索引
int find_flight_index(Manager *manager,const char *flight_no) {
    unsigned int hash = hash_string(flight_no);
    FlightIndexNode *current = manager->flight_index[hash];
    while (current != NULL) {
        if (strcmp(current->flight_no, flight_no) == 0) {
            return current->array_index;
        }
        current = current->next;
    }
    return -1;
}

//删除航班号索引
void remove_from_flight_index(Manager *manager,const char *flight_no) {
    unsigned int hash = hash_string(flight_no);
    FlightIndexNode *current = manager->flight_index[hash];
    FlightIndexNode *previous = NULL;

    while (current != NULL) {
        if (strcmp(current->flight_no, flight_no) == 0) {
            if (previous == NULL) {
                //删除头节点
                manager->flight_index[hash] = current->next;
            }else {
                //删除非头节点
                previous->next = current->next;
            }
            free(current);
            return;
        }
        previous = current;
        current = current->next;
    }
}

void init_route_index(Manager *manager) {
    for (int i = 0; i < HASH_NUM; i++) {
        manager->route_index[i] = NULL;
    }
}

RouteDestNode *find_destination(RouteHashNode* route_node, const char* end) {
    RouteDestNode *dest = route_node->destinations;
    while (dest != NULL) {
        if (strcmp(dest->end, end) == 0) {
            return dest;
        }
        dest = dest->next;
    }
    return NULL;
}

void add_to_route_index(Manager *manager, const char *start,
                        const char *end, const char *flight_no) {
    unsigned int hash = hash_string(start);
    RouteHashNode *route_node = manager->route_index[hash];

    //查找或创建航线节点
    while (route_node != NULL) {
        if (strcmp(route_node->start, start) == 0) {
            break;
        }
        route_node = route_node->next;
    }

    if (route_node == NULL) {
        route_node = (RouteHashNode*)malloc(sizeof(RouteHashNode));
        strcpy(route_node->start, start);
        route_node->destinations = NULL;
        route_node->next = manager->route_index[hash];
        manager->route_index[hash] = route_node;
    }

    //查找或创建目的地节点
    RouteDestNode *dest_node = find_destination(route_node, end);
    if (dest_node == NULL) {
        dest_node = (RouteDestNode*)malloc(sizeof(RouteDestNode));
        strcpy(dest_node->end, end);
        dest_node->flights = NULL;
        dest_node->next = route_node->destinations;
        route_node->destinations = dest_node;
    }

    //添加航班到航线链表
    RouteListNode *flight_node = (RouteListNode*)malloc(sizeof(RouteListNode));
    strcpy(flight_node->flight_no, flight_no);
    flight_node->next = dest_node->flights;
    dest_node->flights = flight_node;
}

void remove_from_route_index(Manager *manager, const char *start,
                             const char *end, const char *flight_no) {
    unsigned int hash = hash_string(start);
    RouteHashNode *route_node = manager->route_index[hash];

    while (route_node != NULL) {
        if (strcmp(route_node->start, start) == 0) {
            RouteDestNode *dest_node = route_node->destinations;
            RouteDestNode *dest_previous = NULL;

            while (dest_node != NULL) {
                if (strcmp(dest_node->end, end) == 0) {
                    RouteListNode *flight_node = dest_node->flights;
                    RouteListNode *flight_previous = NULL;

                    while (flight_node != NULL) {
                        if (strcmp(flight_node->flight_no, flight_no) == 0) {
                            if (flight_previous != NULL) {
                                flight_previous->next = flight_node->next;
                            }else {
                                dest_node->flights = flight_node->next;
                            }
                            free(flight_node);
                            break;
                        }
                        flight_previous = flight_node;
                        flight_node = flight_node->next;
                    }

                    //如果该目的地没有航班了，删除目的地节点
                    if (!dest_node->flights) {
                        if (dest_previous != NULL) {
                            dest_previous->next = dest_node->next;
                        }else {
                            route_node->destinations = dest_node->next;
                        }
                        free(dest_node);
                    }
                    break;
                }
                dest_previous = dest_node;
                dest_node = dest_node->next;
            }

            //如果该航线没有目的地了，删除航线节点
            if (!route_node->destinations) {
                //需要从链表中删除route_node
                RouteHashNode *previous = NULL;
                RouteHashNode *current = manager->route_index[hash];
                while (current != NULL) {
                    if (current == route_node) {
                        if (previous != NULL) {
                            previous->next = current->next;
                        }else {
                            manager->route_index[hash] = current->next;
                        }
                        free(current);
                        break;
                    }
                    previous = current;
                    current = current->next;
                }
            }
            break;
        }
        route_node = route_node->next;
    }
}

Manager *create_flight_manager() {
    Manager *manager = (Manager*)malloc(sizeof(Manager));
    manager->flights_count = 0;
    manager->is_modified = false;  //初始化

    for (int i=0;i < HASH_NUM;i++) {
        manager->route_index[i] = NULL;
    }

    init_flight_index(manager);  //初始化航班号索引
    init_route_index(manager);  //初始化航线索引

    return manager;
}

void free_flight(Flight *flight) {
    if (flight != NULL) {
        free(flight);
    }
}

void free_flight_manager(Manager *manager) {
    if (manager == NULL) {
        return;
    }

    //释放所有航班
    for (int i= 0; i< manager->flights_count; i++) {
        free(manager->flights[i]);
    }

    //释放航班号索引
    for (int i=0;i < HASH_NUM;i++) {
        FlightIndexNode *current = manager->flight_index[i];
        while (current != NULL) {
            FlightIndexNode *tmp = current;
            current = current->next;
            free(tmp);
        }
    }

    //释放航线索引
    for (int i=0;i< HASH_NUM;i++) {
        RouteHashNode *route = manager->route_index[i];
        while (route != NULL) {
            RouteHashNode *tmp_route = route;
            RouteDestNode *dest = route->destinations;
            while (dest != NULL) {
                RouteDestNode *tmp_dest = dest;
                RouteListNode *flight_node = dest->flights;
                while (flight_node != NULL) {
                    RouteListNode *tmp_flight = flight_node;
                    flight_node = flight_node->next;
                    free(tmp_flight);
                }
                dest = dest->next;
                free(tmp_dest);
            }
            route = route->next;
            free(tmp_route);
        }
    }
    free(manager);
}

//获取当前时间字符串
void get_current_time(char *buffer, size_t size) {
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    strftime(buffer, size, "%Y-%m-%d-%H:%M:%S", tm_info);
}

//创建航班
Flight* create_flight(const char *flight_no, const char *start,
                      const char *end, const char *start_time,
                      const char *end_time, double price,
                      double discount, int total_seats) {
    Flight *flight = (Flight*)malloc(sizeof(Flight));
    strcpy(flight->flight_no, flight_no);
    strcpy(flight->start,start);
    strcpy(flight->end,end);
    strcpy(flight->start_time,start_time);
    strcpy(flight->end_time,end_time);
    flight->price = price;
    flight->discount = discount;
    flight->total_seats = total_seats;
    flight->remaining_seats = total_seats;
    get_current_time(flight->last_updated,sizeof(flight->last_updated));
    return flight;
}

//获取当前票价
double get_current_price(Flight *flight) {
    return flight->price * flight->discount;
}

//添加航班
bool add_flight(Manager *manager, Flight *flight) {
    if (manager->flights_count >= MAX_FIGHT_NO) {
        printf("错误：航班数量已达上限 %d\n", MAX_FIGHT_NO);
        return false;
    }

    //检查航班号是否存在
    if (find_flight_index(manager,flight->flight_no) != -1) {
        printf("错误：航班号 %s 已存在\n",flight->flight_no);
        return false;
    }

    //添加到主存储
    int index = manager->flights_count;
    manager->flights[index] = flight;
    manager->flights_count++;

    //添加到索引
    add_to_flight_index(manager,flight->flight_no,index);
    add_to_route_index(manager,flight->start,flight->end,flight->flight_no);

    manager->is_modified = true;
    printf("成功添加航班： %s\n",flight->flight_no);
    return true;
}

//通过航班号查询
Flight* get_flight_by_number(Manager *manager, const char *flight_no) {
    int index = find_flight_index(manager,flight_no);
    if (index == -1) {
        return NULL;
    }
    return manager->flights[index];
}

//通过航线查询
void get_flights_by_route(Manager *manager, const char *start,
                          const char *end, Flight **result, int *count) {
    *count = 0;
    unsigned int hash = hash_string(start);
    RouteHashNode *route_node = manager->route_index[hash];

    while (route_node != NULL) {
        if (strcmp(route_node->start,start) == 0) {
            RouteDestNode *dest_node = route_node->destinations;
            while (dest_node != NULL) {
                if (strcmp(dest_node->end,end) == 0) {
                    RouteListNode *flight_node = dest_node->flights;
                    while (flight_node != NULL) {
                        Flight *flight = get_flight_by_number(manager,flight_node->flight_no);
                        if (flight != NULL) {
                            result[*count] = flight;
                            (*count)++;
                        }
                        flight_node = flight_node->next;
                    }
                    return;
                }
                dest_node = dest_node->next;
            }
            break;
        }
        route_node = route_node->next;
    }
}

bool update_flight(Manager *manager,const char *flight_no,double price, double discount) {
    if (manager == NULL || flight_no == NULL) {
        printf("错误：无效的参数\n");
        return false;
    }

    if (price < 0) {
        printf("错误：价格不能为负\n");
        return false;
    }

    if (discount < 0 || discount > 1.0) {
        printf("错误：折扣只能在0和1之间\n");
        return false;
    }

    int index = find_flight_index(manager,flight_no);
    if (index == -1) {
        printf("错误：未找到航班 %s\n", flight_no);
        return false;
    }

    Flight *flight = manager->flights[index];
    if (flight == NULL) {
        printf("错误：数据异常\n");
        return false;
    }

    double old_price = flight->price;
    double old_discount = flight->discount;
    double old_current_price = get_current_price(flight);

    flight->price = price;
    flight->discount = discount;
    get_current_time(flight->last_updated,sizeof(flight->last_updated));

    manager->is_modified = true;

    printf("✓ 成功更新航班 %s\n", flight_no);
    printf("  票价: %.2f → %.2f\n", old_price, price);
    printf("  折扣: %.2f → %.2f\n", old_discount, discount);
    printf("  当前票价: %.2f → %.2f\n", old_current_price, get_current_price(flight));
    printf("  更新时间: %s\n", flight->last_updated);

    return true;
}

bool delete_flight(Manager *manager, const char *flight_no) {
    int index = find_flight_index(manager,flight_no);
    if (index == -1) {
        printf("错误：未找到航班 %s\n", flight_no);
        return false;
    }

    Flight *flight = manager->flights[index];

    //从索引中移除
    remove_from_flight_index(manager,flight_no);
    remove_from_route_index(manager,flight->start,flight->end,flight_no);

    //从主存储中删除
    free(flight);
    manager->flights[index] = NULL;

    //压缩数组
    for (int i=0;i < manager->flights_count - 1;i++) {
        manager->flights[i] = manager->flights[i+1];
    }
    manager->flights_count--;

    //重建航班号索引
    init_flight_index(manager);
    for (int i=0;i<manager->flights_count;i++) {
        if (manager->flights[i] != NULL) {
            add_to_flight_index(manager,manager->flights[i]->flight_no,i);
        }
    }

    manager->is_modified = true;
    printf("成功删除航班：%s\n",flight_no);
    return true;
}

//预定座位
bool book_seat(Manager *manager, const char *flight_no, int quantity) {
    Flight *flight = get_flight_by_number(manager,flight_no);
    if (flight == NULL) {
        printf("错误：未找到航班 %s\n", flight_no);
        return false;
    }

    if (flight->remaining_seats < quantity) {
        printf("错误：剩余座位不足（需要 %d，剩余 %d）\n",quantity,flight->remaining_seats);
        return false;
    }

    flight->remaining_seats -= quantity;
    get_current_time(flight->last_updated,sizeof(flight->last_updated));
    manager->is_modified = true;
    printf("成功预定 %d 个座位\n",quantity);
    return true;
}

//数据持久化
void save_to_file(Manager *manager) {
    FILE *file = fopen(DATA_FILE,"w");
    if (file == NULL) {
        printf("错误：无法打开文件 %s 进行写入\n",DATA_FILE);
        return;
    }

    //写入航班数量
    fprintf(file,"%d\n",manager->flights_count);

    //写入每个航班信息
    for (int i=0;i<manager->flights_count;i++) {
        Flight *flight = manager->flights[i];
        if (flight == NULL) continue;

        fprintf(file,"%s\n",flight->flight_no);
        fprintf(file,"%s\n",flight->start);
        fprintf(file,"%s\n",flight->end);
        fprintf(file,"%s\n",flight->start_time);
        fprintf(file,"%s\n",flight->end_time);
        fprintf(file,"%lf\n",flight->price);
        fprintf(file,"%lf\n",flight->discount);
        fprintf(file,"%d\n",flight->total_seats);
        fprintf(file,"%d\n",flight->remaining_seats);
        fprintf(file,"%s\n",flight->last_updated);
        }
    fclose(file);
    manager->is_modified = false;
    printf("数据已保存到 %s （%d 条记录）\n",DATA_FILE,manager->flights_count);
}

void load_from_file(Manager *manager) {
    FILE *file = fopen(DATA_FILE,"r");
    if (file == NULL) {
        printf("数据文件不存在，初始化控系统\n");
        return;
    }

    //清空现有数据
    for (int i=0;i<manager->flights_count;i++) {
        if (manager->flights[i] != NULL) {
            free(manager->flights[i]);
            manager->flights[i] = NULL;
        }
    }

    manager->flights_count = 0;
    init_flight_index(manager);
    init_route_index(manager);

    // 清理索引（这里简化处理，实际需要完整清理）
    for (int i = 0; i < HASH_NUM; i++) {
        // 清理航班号索引
        FlightIndexNode* node = manager->flight_index[i];
        while (node) {
            FlightIndexNode* temp = node;
            node = node->next;
            free(temp);
        }
        manager->flight_index[i] = NULL;

        // 清理航线索引（简化）
        RouteHashNode* route = manager->route_index[i];
        while (route) {
            RouteHashNode* temp_route = route;
            RouteDestNode* dest = route->destinations;
            while (dest) {
                RouteDestNode* temp_dest = dest;
                RouteListNode* flight_node = dest->flights;
                while (flight_node) {
                    RouteListNode* temp_flight = flight_node;
                    flight_node = flight_node->next;
                    free(temp_flight);
                }
                dest = dest->next;
                free(temp_dest);
            }
            route = route->next;
            free(temp_route);
        }
        manager->route_index[i] = NULL;
    }

    int count;
    if (fscanf(file, "%d\n", &count) != 1) {
        printf("错误: 文件格式无效\n");
        fclose(file);
        return;
    }

    //读取每个航班
    for (int i=0;i<count;i++) {
        Flight *flight = (Flight *) malloc(sizeof(Flight));

        fscanf(file, "%s\n", flight->flight_no);
        fscanf(file, "%s\n", flight->start);
        fscanf(file, "%s\n", flight->end);
        fscanf(file, "%s\n", flight->start_time);
        fscanf(file, "%s\n", flight->end_time);
        fscanf(file, "%lf\n", &flight->price);
        fscanf(file, "%lf\n", &flight->discount);
        fscanf(file, "%d\n", &flight->total_seats);
        fscanf(file, "%d\n", &flight->remaining_seats);
        fscanf(file, "%s\n", flight->last_updated);

        //添加到系统
        manager->flights[manager->flights_count] = flight;
        add_to_flight_index(manager,flight->flight_no,manager->flights_count);
        add_to_route_index(manager,flight->start,flight->end,flight->flight_no);
        manager->flights_count++;
    }

    fclose(file);
    manager->is_modified = true;
    printf("成功加载 %d 条航班信息\n",manager->flights_count);
}

//显示函数
void print_flight(Flight *flight) {
    if (flight == NULL) return;
    printf("航班号: %s\n", flight->flight_no);
    printf("  起飞城市: %s\n", flight->start);
    printf("  抵达城市: %s\n", flight->end);
    printf("  计划起飞: %s\n", flight->start_time);
    printf("  计划抵达: %s\n", flight->end_time);
    printf("  基础票价: %.2f\n", flight->price);
    printf("  折扣: %.2f\n", flight->discount);
    printf("  当前票价: %.2f\n", get_current_price(flight));
    printf("  总座位数: %d\n", flight->total_seats);
    printf("  剩余座位: %d\n", flight->remaining_seats);
    printf("  更新时间: %s\n", flight->last_updated);
}

void print_all_flights(Manager *manager) {
    printf("\n========== 所有航班信息 ==========\n");
    printf("总计: %d 个航班\n\n", manager->flights_count);
    for (int i = 0; i < manager->flights_count; i++) {
        print_flight(manager->flights[i]);
        printf("------------------------\n");
    }
}

void print_flight_stats(Manager *manager) {
    printf("\n========== 系统统计 ==========\n");
    printf("总航班数: %d\n", manager->flights_count);

    int total_seats = 0;
    int remaining_seats = 0;
    for (int i = 0; i < manager->flights_count; i++) {
        if (manager->flights[i]) {
            total_seats += manager->flights[i]->total_seats;
            remaining_seats += manager->flights[i]->remaining_seats;
        }
    }
    printf("总座位数: %d\n", total_seats);
    printf("剩余座位: %d\n", remaining_seats);
    printf("上座率: %.1f%%\n",
           total_seats > 0 ? (float)(total_seats - remaining_seats) / total_seats * 100 : 0);
}
