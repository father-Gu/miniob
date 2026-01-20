#include "common/lang/comparator.h"
#include "common/lang/sstream.h"
#include "common/lang/string.h"
#include "common/log/log.h"
#include "common/type/date_type.h"
#include "common/value.h"
#include "storage/common/column.h"

// 闰年判断: 能被4整除但不能被100整除，或者能被400整除
bool DateType::is_leap_year(int year)
{
  return (year % 400 == 0) || (year % 4 == 0 && year % 100 != 0);
}

bool DateType::is_valid_date(int year, int month, int day)
{
  // 检查年份范围：0001-9999
  if (year < 1 || year > 9999) {
    return false;
  }

  // 检查月份范围
  if (month < 1 || month > 12) {
    return false;
  }

  // 检查日期范围
  int days_in_month[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  
  // 闰年2月有29天
  if (is_leap_year(year)) {
    days_in_month[2] = 29;
  }

  if (day < 1 || day > days_in_month[month]) {
    return false;
  }

  return true;
}

// 解析日期字符串，格式为 YYYY-MM-DD 或 YYYY-M-D
RC DateType::str_to_date(const string &date_str, int &date_val)
{
  int year = 0, month = 0, day = 0;
  int parsed_count = sscanf(date_str.c_str(), "%d-%d-%d", &year, &month, &day);
  
  if (parsed_count != 3) {
    LOG_WARN("Invalid date format: %s", date_str.c_str());
    return RC::INVALID_ARGUMENT;
  }

  // 验证日期有效性
  if (!is_valid_date(year, month, day)) {
    LOG_WARN("Invalid date value: year=%d, month=%d, day=%d", year, month, day);
    return RC::INVALID_ARGUMENT;
  }

  // 将日期编码为整数：YYYYMMDD
  date_val = year * 10000 + month * 100 + day;
  return RC::SUCCESS;
}

// 从整数转换为日期字符串，格式为 YYYY-MM-DD
string DateType::date_to_str(int date_val)
{
  int year = date_val / 10000;
  int month = (date_val % 10000) / 100;
  int day = date_val % 100;

  stringstream ss;
  // 设置填充字符为0，宽度为4，确保年份是四位（不足补0）
  ss << std::setfill('0') << std::setw(4) << year << "-";
  // 设置填充字符为0，宽度为2，确保月份是两位（不足补0）
  ss << std::setfill('0') << std::setw(2) << month << "-";
  // 同样设置宽度为2，确保日期是两位（不足补0）
  ss << std::setw(2) << day;
  return ss.str();
}

int DateType::compare(const Value &left, const Value &right) const
{
  ASSERT(left.attr_type() == AttrType::DATES, "left type is not date");
  ASSERT(right.attr_type() == AttrType::DATES, "right type is not date");
  
  int left_date = left.get_int();
  int right_date = right.get_int();
  
  if (left_date < right_date) {
    return -1;
  } else if (left_date > right_date) {
    return 1;
  }
  return 0;
}

int DateType::compare(const Column &left, const Column &right, int left_idx, int right_idx) const
{
  ASSERT(left.attr_type() == AttrType::DATES, "left type is not date");
  ASSERT(right.attr_type() == AttrType::DATES, "right type is not date");
  
  int left_date = ((int*)left.data())[left_idx];
  int right_date = ((int*)right.data())[right_idx];
  
  if (left_date < right_date) {
    return -1;
  } else if (left_date > right_date) {
    return 1;
  }
  return 0;
}

RC DateType::cast_to(const Value &val, AttrType type, Value &result) const
{
  switch (type) {
    case AttrType::INTS: {
      result.set_int(val.get_int());
      return RC::SUCCESS;
    }
    default:
      LOG_WARN("unsupported type %d", type);
      return RC::SCHEMA_FIELD_TYPE_MISMATCH;
  }
}

RC DateType::set_value_from_str(Value &val, const string &data) const
{
  int date_val = 0;
  RC rc = str_to_date(data, date_val);
  if (rc == RC::SUCCESS) {
    val.set_type(AttrType::DATES);
    val.set_data((char *)&date_val, sizeof(date_val));
  }
  return rc;
}

RC DateType::to_string(const Value &val, string &result) const
{
  result = date_to_str(val.get_int());
  return RC::SUCCESS;
}