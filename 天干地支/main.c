#include <stdint.h>
#include <stdio.h>

const char *天干[] = {"甲", "乙", "丙", "丁", "戊",
                      "己", "庚", "辛", "壬", "癸"};

const char *地支[] = {"子", "丑", "寅", "卯", "辰", "巳",
                      "午", "未", "申", "酉", "戌", "亥"};

const char *生肖[] = {"鼠", "牛", "虎", "兔", "龙", "蛇",
                      "马", "羊", "猴", "鸡", "狗", "猪"};

int main(int argc, char *argv[]) {
  uint32_t year = 0;
  printf("输入年份：");
  int r = scanf("%u", &year);
  year -= 4;
  printf("%u年是 %s%s%s年\n", year + 4, 天干[year % 10], 地支[year % 12],
         生肖[year % 12]);
  return 0;
}