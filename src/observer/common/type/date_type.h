#pragma once

#include "common/type/data_type.h"

/**
 * @brief 日期类型
 * @ingroup DataType
 * @details date 类型的格式是 YYYY-MM-DD，存储date类型时，使用一个整数表示，格式为 YYYYMMDD。
 */
class DateType : public DataType
{
public:
  DateType() : DataType(AttrType::DATES) {}
  virtual ~DateType() {}

  int compare(const Value &left, const Value &right) const override;
  int compare(const Column &left, const Column &right, int left_idx, int right_idx) const override;

  RC cast_to(const Value &val, AttrType type, Value &result) const override;

  int cast_cost(const AttrType type) override
  {
    if (type == AttrType::DATES) {
      return 0;
    } else if (type == AttrType::INTS) {
      return 1;
    }
    return INT32_MAX;
  }

  RC set_value_from_str(Value &val, const string &data) const override;

  RC to_string(const Value &val, string &result) const override;

  /**
   * @brief 验证日期字符串的有效性并将其转换为整数表示
   * @param date_str 日期字符串，格式为 YYYY-MM-DD
   * @param date_val 输出参数，存储转换后的整数表示
   * @return RC::SUCCESS 表示有效日期，RC::INVALID_ARGUMENT 表示无效日期
   */
  static RC str_to_date(const string &date_str, int &date_val);

  /**
   * @brief 将整数表示的日期转换为字符串
   * @param date_val 整数表示的日期 (YYYYMMDD)
   * @return 日期字符串，格式为 YYYY-MM-DD
   */
  static string date_to_str(int date_val);

  /**
   * @brief 检查是否为闰年
   */
  static bool is_leap_year(int year);

  /**
   * @brief 检查日期的月和日是否有效
   */
  static bool is_valid_date(int year, int month, int day);
};