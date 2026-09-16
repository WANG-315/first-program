#include "message.h"
#include "introduce.h"
#include <string.h>
#include "order.system.h"


// ============ 显示推荐结果 ============
void display_recommendations(Flight** recommendations, int count, const char* title) {
    printf("\n========== %s ==========\n", title);
    if (count == 0) {
        printf("没有找到推荐的航班\n");
        return;
    }

    printf("找到 %d 个推荐航班:\n\n", count);
    for (int i = 0; i < count; i++) {
        Flight* f = recommendations[i];
        printf("%d. 航班号: %s\n", i + 1, f->flight_no);
        printf("   起飞时间: %s\n", f->start_time);
        printf("   当前票价: %.2f\n", get_current_price(f));
        printf("   剩余座位: %d\n", f->remaining_seats);
        printf("   --------------------\n");
    }
}

// ============ 演示多条件查询 ============
void demo_complex_query(Manager* manager) {
    printf("\n========== 多条件组合查询演示 ==========\n");

    // 获取用户输入
    char start[50] = {0};
    char end[50] = {0};
    char date[20] = {0};
    int start_hour = 0, start_minute = 0;
    int end_hour = 0, end_minute = 0;
    double min_price = 0, max_price = 0;
    int has_condition = 0;

    QueryCriteria qc = {0};

    printf("请输入查询条件（直接回车跳过）:\n");

    printf("起飞城市: ");
    scanf("%s", start);
    if (strlen(start) > 0) {
        qc.has_started = true;
        strcpy(qc.start, start);
        has_condition = 1;
    }

    printf("抵达城市: ");
    scanf("%s", end);
    if (strlen(end) > 0) {
        qc.has_ended = true;
        strcpy(qc.end, end);
        has_condition = 1;
    }

    printf("日期 (YYYY-MM-DD): ");
    scanf("%s", date);
    if (strlen(date) > 0) {
        qc.has_date = true;
        strcpy(qc.date, date);
        has_condition = 1;
    }

    printf("时间范围 (开始小时 结束小时, 如: 8 12): ");
    scanf("%d %d", &start_hour, &end_hour);
    if (start_hour > 0 || end_hour > 0) {
        qc.has_time_range = true;
        qc.start_minutes = start_hour * 60 + start_minute;
        qc.end_minutes = end_hour * 60 + end_minute;
        has_condition = 1;
    }

    printf("价格范围 (最低 最高, 如: 800 1200): ");
    scanf("%lf %lf", &min_price, &max_price);
    if (min_price > 0 || max_price > 0) {
        qc.has_price_range = true;
        qc.min_price = min_price;
        qc.max_price = max_price;
        has_condition = 1;
    }

    printf("最少剩余座位: ");
    scanf("%d", &qc.min_remaining_seats);
    if (qc.min_remaining_seats > 0) {
        qc.has_seats = true;
        has_condition = 1;
    }

    if (!has_condition) {
        printf("没有输入任何查询条件，显示所有航班\n");
        print_all_flights(manager);
        return;
    }

    Flight* results[100];
    int count = complex_query(manager, &qc, results, 100);

    if (count > 0) {
        printf("\n找到 %d 个航班:\n", count);
        printf("========================================\n");
        for (int i = 0; i < count; i++) {
            printf("%d. %s | %s → %s | %s | 票价: %.2f | 剩余: %d\n",
                   i + 1,
                   results[i]->flight_no,
                   results[i]->start,
                   results[i]->end,
                   results[i]->start_time,
                   get_current_price(results[i]),
                   results[i]->remaining_seats);
        }
        printf("========================================\n");
    } else {
        printf("没有找到符合条件的航班\n");
    }
}

// ============ 演示智能推荐 ============
void demo_smart_recommend(Manager* manager) {
    printf("\n========== 智能推荐演示 ==========\n");

    char start[50], end[50], preferred_time[10];
    double max_price;

    printf("请输入起飞城市: ");
    scanf("%s", start);
    printf("请输入抵达城市: ");
    scanf("%s", end);
    printf("请输入偏好的起飞时间 (HH:MM, 如 09:00): ");
    scanf("%s", preferred_time);
    printf("请输入最高可接受价格: ");
    scanf("%lf", &max_price);

    Flight* recommendations[10];
    int count = smart_recommend(manager, start, end, preferred_time,
                                max_price, recommendations, 10);

    display_recommendations(recommendations, count, "智能推荐结果");

    if (count > 0) {
        printf("\n推荐说明:\n");
        printf("  按综合评分排序（时间匹配度50%% + 价格40%% + 座位充裕度10%%）\n");
        printf("  最推荐的航班综合考虑了起飞时间、价格和剩余座位\n");
    }
}

// ============ 演示满仓推荐 ============
void demo_full_flight_recommend(Manager* manager) {
    printf("\n========== 满仓推荐 ==========\n");

    char flight_no[20];
    printf("请输入满仓的航班号: ");
    scanf("%s", flight_no);

    // 查找航班
    int index = find_flight_index(manager, flight_no);
    if (index == -1) {
        printf("未找到航班 %s\n", flight_no);
        return;
    }

    Flight* flight = manager->flights[index];
    printf("\n场景: 用户想订 %s (%s → %s)，但该航班已满仓\n",
           flight->flight_no, flight->start, flight->end);

    // 获取用户偏好
    char preferred_time[10];
    double max_price;
    printf("请输入偏好的起飞时间 (HH:MM): ");
    scanf("%s", preferred_time);
    printf("请输入最高可接受价格: ");
    scanf("%lf", &max_price);

    Flight* recommendations[10];
    int count = full_flight_recommend(manager, flight->start,
                                      flight->end, preferred_time,
                                      max_price, recommendations, 10);

    display_recommendations(recommendations, count, "推荐其他航班");
}

// ============ 演示AVL树范围查询 ============
void demo_avl_range_query(Manager* manager) {

    printf("\n========== AVL树价格范围查询 ==========\n");

    double min_price, max_price;
    printf("请输入价格范围 (最低 最高): ");
    scanf("%lf %lf", &min_price, &max_price);

    PriceAVLNode* price_root = NULL;
    TimeAVLNode* time_root = NULL;

    // 构建AVL索引
    build_avl_indexes(manager, &price_root, &time_root);

    Flight* results[100];
    int count = price_range_query(price_root, min_price, max_price, manager, results, 100);

    if (count > 0) {
        printf("\n价格范围 %.2f-%.2f 的航班:\n", min_price, max_price);
        printf("========================================\n");
        for (int i = 0; i < count; i++) {
            printf("%d. %s | 航线：%s -> %s | 票价: %.2f | 剩余: %d\n",
                   i + 1,
                   results[i]->flight_no,
                   results[i]->start,
                   results[i]->end,
                   get_current_price(results[i]),
                   results[i]->remaining_seats);
        }
        printf("========================================\n");
        printf("使用AVL树索引查询\n");
    } else {
        printf("没有找到符合条件的航班\n");
    }

    // 释放AVL树
    price_avl_free(price_root);
    time_avl_free(time_root);
}

void client_system(Manager* manager)
{
    srand(time(NULL));
    OrderManager *mgr=create_order_manager();

    int choice;
    char flight_no[MAX_FLIGHT_NO];
    char src[MAX_CITY],dst[MAX_CITY];
    char name[MAX_NAME],id[MAX_ID];
    char orderId[MAX_ORDER_ID];
    char oldOrderId[MAX_ORDER_ID];
    char newFlightNo[MAX_FLIGHT_NO];
    int *indices=NULL;

    // 清空输入缓冲区
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}

    printf("请输入乘客姓名: ");
    fgets(name, sizeof(name), stdin);
    name[strcspn(name, "\n")] = '\0';
    printf("请输入证件号: ");
    fgets(id, sizeof(id), stdin);
    id[strcspn(id, "\n")] = '\0';

    int count;
       while (1) {
        printf("\n========== 民航订票系统（用户端） ==========\n");
        printf("1. 按航班号查询\n");
        printf("2. 按航线查询\n");
        printf("3. 订票\n");
        printf("4. 退票\n");
        printf("5. 查询我的订单\n");
        printf("6. 查看全部订单（含已退票）\n");
        printf("7. 改签\n");
        printf("----------------------------------------\n");
        printf("8. 多条件组合查询 (优化功能)\n");
        printf("9. 智能推荐 (优化功能)\n");
        printf("10. 满仓推荐 (优化功能)\n");
        printf("11. AVL树价格范围查询 (优化功能)\n");
        printf("0. 退出系统\n");
        printf("请选择: ");
        scanf("%d", &choice);
        getchar();
        switch (choice) {
            case 1: {
                printf("请输入航班号: ");
                scanf("%s", flight_no);
                int index = find_flight_index(manager, flight_no);
                if (index != -1) {
                    printf("\n找到航班:\n");
                    print_flight(manager->flights[index]);
                } else {
                    printf("未找到航班 %s\n", flight_no);
                }
                break;
            }
            case 2: {
                printf("请输入起飞城市: ");
                scanf("%s", src);
                printf("请输入抵达城市: ");
                scanf("%s", dst);

                Flight* results[100];
                int count = 0;
                get_flights_by_route(manager, src, dst, results, &count);

                if (count > 0) {
                    display_flights_page_by_page(results, count);
                }
                else {
                    printf("未找到从 %s 到 %s 的航班\n", src, dst);
                }
                break;
            }
            case 3: {
                printf("请输入要预订的航班号: ");
                fgets(flight_no, sizeof(flight_no), stdin);
                flight_no[strcspn(flight_no, "\n")] = '\0';
                Flight *f = get_flight_by_number(manager,flight_no);
                if (f->remaining_seats <= 0) {
                    printf("该航班已满仓。\n");
                    recommend_flights(manager, f->start, f->end);
                    break;
                }
                if (!f) {
                    printf("航班不存在。\n");
                    break;
                }
                if (f->remaining_seats <= 0) {
                    printf("该航班已满仓。\n");
                    break;
                }
                if (reserve_seat(manager, flight_no)) {
                    if (create_order(mgr, id, flight_no, name) == 0) {
                        printf("订票成功！订单已生成。\n");
                    } else {
                        printf("订单创建失败，请重试。\n");
                        /* 回滚座位（简单起见，这里忽略） */
                    }
                } else {
                    printf("订票失败（可能余票不足）。\n");
                }
                break;
            }
            case 4: {
                printf("请输入要退票的订单号: ");
                fgets(orderId,sizeof(orderId),stdin);
                orderId[strcspn(orderId,"\n")]='\0';
                int ret=cancel_order(mgr, orderId, manager);
                if (ret==0) {
                    printf("退票成功！\n");
                }else if (ret==-1) {
                    printf("订单不存在。\n");
                }else if (ret == -2) {
                    printf("该订单已退票或无效。\n");
                }else {
                    printf("退票失败（座位恢复异常）。\n");
                }
                break;
            }
            case 5: {
                Order *orders=query_orders_by_id(mgr, id,&count);
                if (count==0) {
                    printf("您当前没有有效订单。\n");
                }else {
                    printf("您的有效订单如下：\n");
                    for (int i = 0; i < count; i++) {
                        Flight *f_tem2 = get_flight_by_number(manager, orders[i].flightNo);
                        printf("%d. 订单号: %s  航班: %s", i+1, orders[i].orderId, orders[i].flightNo);
                        if (f_tem2) printf("  %s->%s", f_tem2->start, f_tem2->end);
                        printf("  姓名: %s\n", orders[i].passengerName);
                    }
                    free(orders);
                }
                break;
            }

            case 6: {
                printf("请输入您的证件号: ");
                fgets(id, sizeof(id), stdin);
                id[strcspn(id, "\n")] = '\0';
                int cnt;
                Order *orders = query_all_orders_by_id(mgr, id, &cnt);
                if (cnt == 0) {
                    printf("您没有任何订单记录。\n");
                } else {
                    printf("\n========== 您的全部订单 ==========\n");
                    for (int i = 0; i < cnt; i++) {
                        printf("%d. 订单号: %s  航班: %s", i+1, orders[i].orderId, orders[i].flightNo);
                        Flight *f = get_flight_by_number(manager, orders[i].flightNo);
                        if (f) printf("  %s→%s", f->start, f->end);
                        printf("  姓名: %s  状态: %s\n", orders[i].passengerName,
                               orders[i].status == 0 ? "有效" : "已退票");
                    }
                    free(orders);
                }
                break;
            }

            case 7: {
                printf("请输入要改签的订单号: ");
                fgets(oldOrderId, sizeof(oldOrderId), stdin);
                oldOrderId[strcspn(oldOrderId, "\n")] = '\0';
                printf("请输入新的航班号: ");
                fgets(newFlightNo, sizeof(newFlightNo), stdin);
                newFlightNo[strcspn(newFlightNo, "\n")] = '\0';

                int ret = change_flight(mgr, oldOrderId, newFlightNo, manager);
                if (ret == 0) {
                    printf("改签成功！订单已更新。\n");
                } else if (ret == -1) {
                    printf("订单不存在。\n");
                } else if (ret == -2) {
                    printf("该订单已退票或无效。\n");
                } else if (ret == -3) {
                    printf("新航班不存在或余票不足。\n");
                } else if (ret == -4) {
                    printf("新航班与原航班航线不一致，请选择同一航线的航班。\n");
                } else if (ret == -5) {
                    printf("系统参数错误，请重试。\n");
                } else {
                    printf("改签失败，未知错误。\n");
                }
                break;

            }
            case 0: {
                destroy_order_manager(mgr);
                printf("感谢使用，再见！\n");
                return;
            }

            case 8:
                demo_complex_query(manager);
                break;

            case 9:
                demo_smart_recommend(manager);
                break;

            case 10:
                demo_full_flight_recommend(manager);
                break;

            case 11:
                demo_avl_range_query(manager);
                break;
                default:
                printf("无效选择，请重新输入。\n");

            }
            printf("\n按回车键继续...");
                getchar();
        }
    }

// 管理员菜单
void admin_menu(Manager* manager) {
    char password[20];
    int attempts = 0;
    const char *CORRECT_PASSWORD = "admin123";
    while (1) {
        printf("\n========== 管理员登录 ==========\n");
        printf("请输入密码（尝试次数 %d/3）: ", attempts + 1);
        scanf("%s", password);
        getchar();  // 清除回车

        if (strcmp(password, CORRECT_PASSWORD) == 0) {
            printf("登录成功！欢迎使用订票系统。\n");
            break;
        } else {
            attempts++;
            if (attempts >= 3) {
                printf("密码错误次数过多，返回主菜单。\n");
                return;  // 返回主菜单
            }
            printf("密码错误，请重新输入。\n");
        }
    }

    int choice;
    char flight_no[20];
    char start[50], end[50];
    char start_time[20], end_time[20];
    double price, discount;
    int total_seats;

    while (1) {
        printf("\n========== 航班管理系统 - 管理员菜单 ==========\n");
        printf("1. 添加航班\n");
        printf("2. 修改航班价格\n");
        printf("3. 删除航班\n");
        printf("4. 查看所有航班\n");
        printf("5. 按航班号查询\n");
        printf("6. 按航线查询\n");
        printf("7. 查看统计信息\n");
        printf("8. 保存数据\n");
        printf("9. 加载数据\n");
        printf("10. 预定座位\n");
        printf("----------------------------------------\n");
        printf("11. 多条件组合查询 (优化功能)\n");
        printf("12. 智能推荐 (优化功能)\n");
        printf("13. 满仓推荐 (优化功能)\n");
        printf("14. AVL树价格范围查询 (优化功能)\n");
        printf("0. 退出\n");
        printf("请选择: ");
        scanf("%d", &choice);

        switch (choice) {
            case 0:
                printf("正在退出...\n");
                return;

            case 1: {
                printf("请输入航班号: ");
                scanf("%s", flight_no);
                printf("请输入起飞城市: ");
                scanf("%s", start);
                printf("请输入抵达城市: ");
                scanf("%s", end);
                printf("请输入计划起飞时间 (YYYY-MM-DD-HH:MM): ");
                scanf("%s", start_time);
                printf("请输入计划抵达时间 (YYYY-MM-DD-HH:MM): ");
                scanf("%s", end_time);
                printf("请输入基础票价: ");
                scanf("%lf", &price);
                printf("请输入折扣 (0.0-1.0): ");
                scanf("%lf", &discount);
                printf("请输入总座位数: ");
                scanf("%d", &total_seats);

                Flight* flight = create_flight(flight_no, start, end,
                                              start_time, end_time, price, discount, total_seats);
                add_flight(manager, flight);
                break;
            }

            case 2: {
                printf("请输入要修改的航班号: ");
                scanf("%s", flight_no);
                printf("请输入新票价: ");
                scanf("%lf", &price);
                printf("请输入新折扣: ");
                scanf("%lf", &discount);
                update_flight(manager, flight_no, price, discount);
                break;
            }

            case 3: {
                printf("请输入要删除的航班号: ");
                scanf("%s", flight_no);
                delete_flight(manager, flight_no);
                break;
            }

            case 4:
                print_all_flights(manager);
                break;

            case 5: {
                printf("请输入航班号: ");
                scanf("%s", flight_no);
                int index = find_flight_index(manager, flight_no);
                if (index != -1) {
                    printf("\n找到航班:\n");
                    print_flight(manager->flights[index]);
                } else {
                    printf("未找到航班 %s\n", flight_no);
                }
                break;
            }

            case 6: {
                printf("请输入起飞城市: ");
                scanf("%s", start);
                printf("请输入抵达城市: ");
                scanf("%s", end);

                Flight* results[100];
                int count = 0;
                get_flights_by_route(manager, start, end, results, &count);

                if (count > 0) {
                    printf("\n找到 %d 个航班:\n", count);
                    for (int i = 0; i < count; i++) {
                        printf("%d. %s (票价: %.2f, 剩余座位: %d)\n",
                               i + 1, results[i]->flight_no,
                               get_current_price(results[i]),
                               results[i]->remaining_seats);
                    }
                } else {
                    printf("未找到从 %s 到 %s 的航班\n", start, end);
                }
                break;
            }

            case 7:
                print_flight_stats(manager);
                break;

            case 8:
                save_to_file(manager);
                break;

            case 9:
                load_from_file(manager);
                break;

            case 10:
                int quantity;
                printf("请输入您的航班号：\n");
                scanf("%s", flight_no);
                printf("您想要预定几个座位：\n");
                scanf("%d", &quantity);
                book_seat(manager, flight_no, quantity);
                break;

            case 11:
                demo_complex_query(manager);
                break;

            case 12:
                demo_smart_recommend(manager);
                break;

            case 13:
                demo_full_flight_recommend(manager);
                break;

            case 14:
                demo_avl_range_query(manager);
                break;

            default:
                printf("无效选项，请重新选择\n");
        }
    }
}

//主菜单
void main_menu() {
    printf("\n========================================\n");
    printf("  欢迎使用航班管理系统\n");
    printf("========================================\n");
    printf("1. 管理员系统\n");
    printf("2. 乘客系统\n");
    printf("0. 退出系统\n");
    printf("========================================\n");
    printf("请选择: ");
}

// 主函数
int main() {

    printf("========================================\n");
    printf("  民航订票系统\n");
    printf("========================================\n\n");

    Manager* manager = create_flight_manager();

    // 加载数据
    load_from_file(manager);

    // 如果没有数据，添加一些示例数据
    if (manager->flights_count == 0) {
        printf("\n系统为空，添加示例数据...\n");
        
        Flight* f1 = create_flight("CA1832", "北京", "上海", 
                                  "2026-07-14-08:00", "2026-07-14-10:30",
                                  1200.0, 0.9, 150);
        add_flight(manager, f1);

        Flight* f2 = create_flight("MU5892", "北京", "上海",
                                  "2026-07-14-09:30", "2026-07-14-11:30",
                                  980.0, 0.8, 120);
        add_flight(manager, f2);

        Flight* f3 = create_flight("CZ3104", "北京", "广州",
                                  "2026-07-15-09:30", "2026-07-15-12:30",
                                  1500.0, 0.95, 180);
        add_flight(manager, f3);

        Flight* f4 = create_flight("HU7132", "上海", "成都",
                                  "2026-07-16-14:00", "2026-07-16-16:30",
                                  880.0, 0.7, 90);
        add_flight(manager, f4);

        save_to_file(manager);
    }
    
    // 进入主菜单
    //主循环
    int choice;
    while (1) {
        main_menu();
        scanf("%d", &choice);
        switch (choice) {
            case 0:
                printf("\n感谢使用，再见！\n");
                save_to_file(manager);
                free_flight_manager(manager);
                return 0;
            case 1:
                printf("\n进入管理员系统...\n");
                admin_menu(manager);
                break;
            case 2:
                printf("\n进入乘客系统...\n");
                client_system(manager);
                break;
            default:
                printf("无效选项，请重新选择\n");
        }
    }
    
    // 退出前保存
    printf("\n正在保存数据...\n");
    save_to_file(manager);
    
    // 释放内存
    free_flight_manager(manager);
    printf("系统已关闭\n");
    
    return 0;
}