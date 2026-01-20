/* Copyright (c) 2021 OceanBase and/or its affiliates. All rights reserved.
miniob is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
         http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#include "common/lang/comparator.h"
#include "common/log/log.h"
#include "common/type/char_type.h"
#include "common/value.h"
#include "common/type/date_type.h"
//比较
int CharType::compare(const Value &left, const Value &right) const
{
  ASSERT((left.attr_type() == AttrType::CHARS || left.attr_type() == AttrType::TEXT) &&
   (right.attr_type() == AttrType::CHARS || right.attr_type() == AttrType::TEXT), "invalid type");
  return common::compare_string(
      (void *)left.value_.pointer_value_, left.length_, (void *)right.value_.pointer_value_, right.length_);
}
//从字符串设置值
RC CharType::set_value_from_str(Value &val, const string &data) const
{
  val.set_string(data.c_str());
  return RC::SUCCESS;
}

//类型转换
RC CharType::cast_to(const Value &val, AttrType type, Value &result) const
{
  switch (type) {
    //字符串转日期
    case AttrType::DATES: {
      int date_val = 0;
      //判定日期的有效性
      RC rc = DateType::str_to_date(val.get_string(), date_val);
      if (rc != RC::SUCCESS) {
        return rc;
      }
      result.set_type(AttrType::DATES);
      result.set_data((char *)&date_val, sizeof(date_val));
      return RC::SUCCESS;
      //return RC::UNIMPLEMENTED;
    }
    //字符串转TEXT
    case AttrType::TEXT: {
      result.set_string(val.get_string().c_str());
      // set_string把类型设置为CHARS，这里需要改成TEXT
      //result.set_type(AttrType::TEXT);
      break;
    }

    default: return RC::UNIMPLEMENTED;
  }
  return RC::SUCCESS;
}

//转换代价评估
int CharType::cast_cost(AttrType type)
{
  if (type == AttrType::CHARS) {
    // 只允许CHARS->TEXT, 设置TEXT->CHARS的代价为最大
    return INT32_MAX;
  }
  if (type == AttrType::TEXT) {
    return 0;
  }
  if (type == AttrType::DATES) {
    return 0;
  }
  return INT32_MAX;
}

//值转字符串
RC CharType::to_string(const Value &val, string &result) const
{
  stringstream ss;
  if(val.attr_type() == AttrType::CHARS || val.attr_type() == AttrType::TEXT) {
    if(val.value_.pointer_value_ != nullptr) {
      // 防止数据被截斷
      ss.write(val.value_.pointer_value_, val.length());
    }
  } else {
    ss << val.value_.pointer_value_;
  }
  result = ss.str();
  return RC::SUCCESS;
}