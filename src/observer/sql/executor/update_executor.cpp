#include "sql/executor/update_executor.h"
#include "common/log/log.h"
#include "event/sql_event.h"
#include "event/session_event.h"
#include "session/session.h"
#include "sql/stmt/update_stmt.h"
#include "storage/table/table.h"
#include "storage/trx/trx.h"
#include "storage/record/record_scanner.h"
#include "storage/common/condition_filter.h"

RC UpdateExecutor::execute(SQLStageEvent *sql_event)
{
  Stmt    *stmt    = sql_event->stmt();
  Session *session = sql_event->session_event()->session();
  ASSERT(stmt->type() == StmtType::UPDATE, 
         "update executor can not run this command: %d", static_cast<int>(stmt->type()));

  UpdateStmt *update_stmt = static_cast<UpdateStmt *>(stmt);
  Table      *table       = update_stmt->table();
  Trx        *trx         = session->current_trx();

  // 创建条件过滤器
  CompositeConditionFilter condition_filter;
  RC                       rc = condition_filter.init(
      *table, update_stmt->conditions().data(), static_cast<int>(update_stmt->conditions().size()));
  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to init condition filter. rc=%s", strrc(rc));
    return rc;
  }

  // 获取字段信息
  const FieldMeta *field_meta = update_stmt->field_meta();
  const Value     &new_value  = update_stmt->value();

  // 获取记录扫描器
  RecordScanner *scanner = nullptr;
  rc                     = table->get_record_scanner(scanner, trx, ReadWriteMode::READ_WRITE);
  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to get record scanner. rc=%s", strrc(rc));
    return rc;
  }

  // 收集需要更新的记录的 RID（只保存位置信息）
  std::vector<RID> rids_to_update;
  Record           record;

  while (RC::SUCCESS == (rc = scanner->next(record))) {
    // 检查是否满足 WHERE 条件
    if (update_stmt->conditions().empty() || condition_filter.filter(record)) {
      rids_to_update.push_back(record.rid());  // ✅ 只保存 RID
    }
  }

  scanner->close_scan();

  if (rc != RC::RECORD_EOF) {
    LOG_WARN("failed to scan records. rc=%s", strrc(rc));
    return rc;
  }

  // 更新所有匹配的记录
  int updated_count = 0;
  for (const RID &rid : rids_to_update) {
    // 重新获取记录（避免使用失效的 Record）
    Record old_record;
    rc = table->get_record(rid, old_record);
    if (rc != RC::SUCCESS) {
      LOG_WARN("failed to get record. rid=%s, rc=%s", 
               rid.to_string().c_str(), strrc(rc));
      return rc;
    }

    // 创建新记录
    Record new_record;
    new_record.copy_data(old_record.data(), old_record.len());
    new_record.set_rid(old_record.rid());

    // 修改字段值
    char *field_data = new_record.data() + field_meta->offset();

    size_t copy_len = field_meta->len();
    if (field_meta->type() == AttrType::CHARS || field_meta->type() == AttrType::TEXT) {
      const size_t data_len = new_value.length();
      if (copy_len > data_len) {
        copy_len = data_len + 1;
      }
    }
    memcpy(field_data, new_value.data(), copy_len);

    // 执行更新
    rc = table->update_record_with_trx(old_record, new_record, trx);
    if (rc != RC::SUCCESS) {
      LOG_WARN("failed to update record. rid=%s, rc=%s", 
               rid.to_string().c_str(), strrc(rc));
      return rc;
    }

    updated_count++;
  }

  // 返回结果
  SqlResult *sql_result = sql_event->session_event()->sql_result();
  sql_result->set_return_code(RC::SUCCESS);

  LOG_INFO("updated %d records", updated_count);

  return RC::SUCCESS;
}